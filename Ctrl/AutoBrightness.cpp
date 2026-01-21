#include <iostream>
#include "AutoBrightness.h"
#include <QString>
#include <QProcess>
#include <algorithm>


void AutoBrightness::setBrightness(int brightness) {
    // 向后兼容：设置第一个显示器的亮度
    if (!m_monitors.empty()) {
        setMonitorBrightness(0, brightness);
    }
}

void AutoBrightness::setMonitorBrightness(int monitorIndex, int brightness) {
    if (monitorIndex < 0 || monitorIndex >= static_cast<int>(m_monitors.size())) {
        qWarning() << "Invalid monitor index:" << monitorIndex;
        return;
    }
    
    // 验证 brightness 参数范围，防止注入
    if (brightness < 0 || brightness > 100) {
        qWarning() << "Invalid brightness value:" << brightness;
        return;
    }
    
    // 使用 PowerShell 数组索引来设置特定显示器的亮度
    // monitorIndex 已通过上述检查确保在有效范围内，brightness 也已验证
    QString command = QString(
        "$monitors = Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods; "
        "if ($monitors -is [array]) { $monitors[%1].WmiSetBrightness(1, %2) } "
        "else { $monitors.WmiSetBrightness(1, %2) }"
    ).arg(monitorIndex).arg(brightness);
    
    QStringList arguments;
    arguments << "-NoProfile" << "-ExecutionPolicy" << "Bypass" 
              << "-WindowStyle" << "Hidden" << "-Command" << command;
    
    QProcess process;
    qDebug() << "Setting monitor" << monitorIndex << "brightness to" << brightness;
    process.start("powershell.exe", arguments);
    process.waitForFinished();
}

void AutoBrightness::refreshMonitors() {
    m_monitors.clear();
    
    // 使用 PowerShell 获取所有显示器信息
    QString command = 
        "$monitors = Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods; "
        "$index = 0; "
        "foreach ($monitor in $monitors) { "
        "    Write-Host \"$index|$($monitor.InstanceName)\"; "
        "    $index++; "
        "}";
    
    QStringList arguments;
    arguments << "-NoProfile" << "-ExecutionPolicy" << "Bypass" 
              << "-WindowStyle" << "Hidden" << "-Command" << command;
    
    QProcess process;
    process.start("powershell.exe", arguments);
    process.waitForFinished(5000);
    
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QStringList parts = line.trimmed().split('|');
        if (parts.size() >= 2) {
            int index = parts[0].toInt();
            QString instanceName = parts[1];
            
            // 生成友好名称（从实例名称中提取或使用索引）
            QString friendlyName = QString("显示器 %1").arg(index + 1);
            
            m_monitors.push_back(MonitorInfo(instanceName, friendlyName, index));
            
            // 为新显示器创建默认配置
            if (m_monitorConfigs.find(index) == m_monitorConfigs.end()) {
                m_monitorConfigs[index] = MonitorConfig();
            }
        }
    }
    
    qDebug() << "Detected" << m_monitors.size() << "monitor(s)";
    emit monitorsChanged();
}

// 获取图像平均亮度
int AutoBrightness::getBrightness(const cv::Mat& frame) {
    if (frame.empty() || m_samplePoints < 2) {
        return 0;
    }
    
    int rows = frame.rows;
    int cols = frame.cols;
    int rowsInterval = rows / m_samplePoints;
    int colsInterval = cols / m_samplePoints;
    
    if (rowsInterval == 0) rowsInterval = 1;
    if (colsInterval == 0) colsInterval = 1;
    
    int sum = 0;
    int count = 0;
    
    for(int i = 0; i < rows; i += rowsInterval){
        for(int j = 0; j < cols; j += colsInterval){
            cv::Vec3b pixel = frame.at<cv::Vec3b>(i, j);
            sum += pixel[2];  // 使用红色通道
            count++;
        }
    }
    
    if (count == 0) return 0;
    
    return sum / (count * 255 / 100);  // 返回0-100的百分比值
}


AutoBrightness::AutoBrightness(QObject* parent): QObject(parent)
    , m_cap(0)
    , m_exposure(-6.0)           // 默认曝光值
    , m_captureInterval(10000)   // 默认10秒间隔
    , m_samplePoints(5)          // 默认5x5采样点
    , m_gain(0.0)                // 默认增益值
    , m_minCameraBrightness(0)
    , m_maxCameraBrightness(100)
    , m_minScreenBrightness(0)
    , m_maxScreenBrightness(100)
    , m_useCurve(false)
    , m_currentCameraBrightness(0)
    , m_currentScreenBrightness(0)
{
    // 禁用自动曝光和自动增益
    loadSettings();
}

AutoBrightness::~AutoBrightness(){

}

AutoBrightness* AutoBrightness::getInstance(){
    static AutoBrightness instance;
    return &instance;
}

void AutoBrightness::openCap(){
    m_cap.open(0, cv::CAP_DSHOW);
    
    // 等待摄像头打开
    do {
        QThread::msleep(2000);
    } while (!m_cap.isOpened());
    
    // 设置摄像头参数
    m_cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // 0.25表示手动模式
    m_cap.set(cv::CAP_PROP_EXPOSURE, m_exposure); // 设置曝光值
    m_cap.set(cv::CAP_PROP_GAIN, m_gain);         // 设置增益值
    
    qDebug() << "Camera opened with exposure:" << m_exposure << "gain:" << m_gain;
}

void AutoBrightness::releaseCap(){
    m_cap.release();
}

void AutoBrightness::update(){

    cv::Mat frame;
    m_cap >> frame; // 获取摄像头帧

    // 计算平均亮度
    int cameraBrightness = getBrightness(frame);
    m_currentCameraBrightness = cameraBrightness;
    
    // 为每个显示器应用各自的映射并设置亮度
    for (size_t i = 0; i < m_monitors.size(); ++i) {
        int monitorIndex = static_cast<int>(i);
        int screenBrightness = mapBrightnessForMonitor(monitorIndex, cameraBrightness);
        m_monitorCurrentScreenBrightness[monitorIndex] = screenBrightness;
        setMonitorBrightness(monitorIndex, screenBrightness);
    }
    
    // 向后兼容：使用第一个显示器的映射作为默认值
    int screenBrightness = mapBrightness(cameraBrightness);
    m_currentScreenBrightness = screenBrightness;
    
    // 发送信号更新UI
    emit cameraBrightnessChanged(cameraBrightness);
    emit screenBrightnessChanged(screenBrightness);
}

void AutoBrightness::setCamera(){
    m_cap.set(cv::CAP_PROP_SETTINGS, 1);
}

void AutoBrightness::setExposure(double exposure) {
    m_exposure = exposure;
    if (m_cap.isOpened()) {
        m_cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // 手动模式
        m_cap.set(cv::CAP_PROP_EXPOSURE, m_exposure);
        qDebug() << "Exposure updated to:" << m_exposure;
    }
}

void AutoBrightness::setCaptureInterval(int intervalMs) {
    m_captureInterval = intervalMs;
    qDebug() << "Capture interval updated to:" << m_captureInterval << "ms";
}

void AutoBrightness::setSamplePoints(int points) {
    if (points >= 2 && points <= 20) {
        m_samplePoints = points;
        qDebug() << "Sample points updated to:" << m_samplePoints << "x" << m_samplePoints;
    }
}

void AutoBrightness::setGain(double gain) {
    m_gain = gain;
    if (m_cap.isOpened()) {
        m_cap.set(cv::CAP_PROP_GAIN, m_gain);
        qDebug() << "Gain updated to:" << m_gain;
    }
}

void AutoBrightness::setMinBrightness(int cameraBrightness, int screenBrightness) {
    m_minCameraBrightness = std::max(0, std::min(100, cameraBrightness));
    m_minScreenBrightness = std::max(0, std::min(100, screenBrightness));
    qDebug() << "Min brightness set: camera=" << m_minCameraBrightness << ", screen=" << m_minScreenBrightness;
}

void AutoBrightness::setMaxBrightness(int cameraBrightness, int screenBrightness) {
    m_maxCameraBrightness = std::max(0, std::min(100, cameraBrightness));
    m_maxScreenBrightness = std::max(0, std::min(100, screenBrightness));
    qDebug() << "Max brightness set: camera=" << m_maxCameraBrightness << ", screen=" << m_maxScreenBrightness;
}

bool AutoBrightness::validateBrightnessConfiguration(QString* errorMessage) const {
    if (m_minCameraBrightness >= m_maxCameraBrightness) {
        if (errorMessage) {
            *errorMessage = QString("摄像头最小亮度 (%1) 必须小于最大亮度 (%2)")
                .arg(m_minCameraBrightness)
                .arg(m_maxCameraBrightness);
        }
        return false;
    }
    
    if (m_minScreenBrightness >= m_maxScreenBrightness) {
        if (errorMessage) {
            *errorMessage = QString("屏幕最小亮度 (%1) 必须小于最大亮度 (%2)")
                .arg(m_minScreenBrightness)
                .arg(m_maxScreenBrightness);
        }
        return false;
    }
    
    // 检查范围是否过小（可能导致映射不准确）
    if (m_maxCameraBrightness - m_minCameraBrightness < 10) {
        if (errorMessage) {
            *errorMessage = QString("摄像头亮度范围过小 (%1-%2)，建议至少相差10个单位以获得准确映射")
                .arg(m_minCameraBrightness)
                .arg(m_maxCameraBrightness);
        }
        return false;
    }
    
    return true;
}

void AutoBrightness::setCurvePoints(const std::vector<std::pair<int, int>>& points) {
    m_curvePoints = points;
    qDebug() << "Curve points updated, count:" << points.size();
}

int AutoBrightness::mapBrightness(int cameraBrightness) {
    if (m_useCurve && m_curvePoints.size() >= 2) {
        return interpolateCurve(cameraBrightness);
    }
    
    // 线性映射
    if (m_maxCameraBrightness == m_minCameraBrightness) {
        qWarning() << "Warning: Min and max camera brightness are equal, mapping is undefined. Returning min screen brightness.";
        return m_minScreenBrightness;
    }
    
    // 限制在配置范围内
    int clampedCamera = std::max(m_minCameraBrightness, std::min(cameraBrightness, m_maxCameraBrightness));
    
    // 线性映射到屏幕亮度范围
    double ratio = static_cast<double>(clampedCamera - m_minCameraBrightness) / 
                   (m_maxCameraBrightness - m_minCameraBrightness);
    int screenBrightness = m_minScreenBrightness + 
                          static_cast<int>(ratio * (m_maxScreenBrightness - m_minScreenBrightness));
    
    return std::max(0, std::min(100, screenBrightness));
}

int AutoBrightness::interpolateCurve(int cameraBrightness) {
    if (m_curvePoints.empty()) {
        return cameraBrightness;
    }
    
    if (m_curvePoints.size() == 1) {
        return m_curvePoints[0].second;
    }
    
    // 找到两个相邻的点进行插值
    // 使用 int 作为循环计数器以避免 size_t 下溢问题
    for (int i = 0; i < static_cast<int>(m_curvePoints.size()) - 1; ++i) {
        if (cameraBrightness >= m_curvePoints[i].first && 
            cameraBrightness <= m_curvePoints[i + 1].first) {
            // 防止除零错误：检查两个点的 x 坐标是否相同
            int deltaX = m_curvePoints[i + 1].first - m_curvePoints[i].first;
            if (deltaX == 0) {
                qWarning() << "Duplicate curve point x-coordinates at" << m_curvePoints[i].first;
                return m_curvePoints[i + 1].second;
            }
            
            // 线性插值
            double ratio = static_cast<double>(cameraBrightness - m_curvePoints[i].first) / deltaX;
            int screenBrightness = m_curvePoints[i].second + 
                                  static_cast<int>(ratio * (m_curvePoints[i + 1].second - m_curvePoints[i].second));
            return std::max(0, std::min(100, screenBrightness));
        }
    }
    
    // 超出范围，使用边界值
    if (cameraBrightness < m_curvePoints[0].first) {
        return m_curvePoints[0].second;
    }
    return m_curvePoints.back().second;
}

// 多显示器配置方法
MonitorInfo AutoBrightness::getMonitorInfo(int index) const {
    if (index >= 0 && index < static_cast<int>(m_monitors.size())) {
        return m_monitors[index];
    }
    return MonitorInfo();
}

void AutoBrightness::setMonitorMinBrightness(int monitorIndex, int cameraBrightness, int screenBrightness) {
    if (monitorIndex < 0 || monitorIndex >= static_cast<int>(m_monitors.size())) {
        qWarning() << "Invalid monitor index:" << monitorIndex;
        return;
    }
    
    m_monitorConfigs[monitorIndex].minCameraBrightness = std::max(0, std::min(100, cameraBrightness));
    m_monitorConfigs[monitorIndex].minScreenBrightness = std::max(0, std::min(100, screenBrightness));
    qDebug() << "Monitor" << monitorIndex << "min brightness set: camera=" 
             << cameraBrightness << ", screen=" << screenBrightness;
}

void AutoBrightness::setMonitorMaxBrightness(int monitorIndex, int cameraBrightness, int screenBrightness) {
    if (monitorIndex < 0 || monitorIndex >= static_cast<int>(m_monitors.size())) {
        qWarning() << "Invalid monitor index:" << monitorIndex;
        return;
    }
    
    m_monitorConfigs[monitorIndex].maxCameraBrightness = std::max(0, std::min(100, cameraBrightness));
    m_monitorConfigs[monitorIndex].maxScreenBrightness = std::max(0, std::min(100, screenBrightness));
    qDebug() << "Monitor" << monitorIndex << "max brightness set: camera=" 
             << cameraBrightness << ", screen=" << screenBrightness;
}

void AutoBrightness::setMonitorCurvePoints(int monitorIndex, const std::vector<std::pair<int, int>>& points) {
    if (monitorIndex < 0 || monitorIndex >= static_cast<int>(m_monitors.size())) {
        qWarning() << "Invalid monitor index:" << monitorIndex;
        return;
    }
    
    m_monitorConfigs[monitorIndex].curvePoints = points;
    qDebug() << "Monitor" << monitorIndex << "curve points updated, count:" << points.size();
}

void AutoBrightness::setMonitorUseCurve(int monitorIndex, bool use) {
    if (monitorIndex < 0 || monitorIndex >= static_cast<int>(m_monitors.size())) {
        qWarning() << "Invalid monitor index:" << monitorIndex;
        return;
    }
    
    m_monitorConfigs[monitorIndex].useCurve = use;
    qDebug() << "Monitor" << monitorIndex << "use curve:" << use;
}

MonitorConfig AutoBrightness::getMonitorConfig(int monitorIndex) const {
    auto it = m_monitorConfigs.find(monitorIndex);
    if (it != m_monitorConfigs.end()) {
        return it->second;
    }
    return MonitorConfig();
}

int AutoBrightness::getMonitorCurrentScreenBrightness(int monitorIndex) const {
    auto it = m_monitorCurrentScreenBrightness.find(monitorIndex);
    if (it != m_monitorCurrentScreenBrightness.end()) {
        return it->second;
    }
    return 0;
}

bool AutoBrightness::validateMonitorConfiguration(int monitorIndex, QString* errorMessage) const {
    auto it = m_monitorConfigs.find(monitorIndex);
    if (it == m_monitorConfigs.end()) {
        if (errorMessage) {
            *errorMessage = QString("显示器 %1 配置不存在").arg(monitorIndex);
        }
        return false;
    }
    
    const MonitorConfig& config = it->second;
    
    if (config.minCameraBrightness >= config.maxCameraBrightness) {
        if (errorMessage) {
            *errorMessage = QString("显示器 %1: 摄像头最小亮度 (%2) 必须小于最大亮度 (%3)")
                .arg(monitorIndex)
                .arg(config.minCameraBrightness)
                .arg(config.maxCameraBrightness);
        }
        return false;
    }
    
    if (config.minScreenBrightness >= config.maxScreenBrightness) {
        if (errorMessage) {
            *errorMessage = QString("显示器 %1: 屏幕最小亮度 (%2) 必须小于最大亮度 (%3)")
                .arg(monitorIndex)
                .arg(config.minScreenBrightness)
                .arg(config.maxScreenBrightness);
        }
        return false;
    }
    
    if (config.maxCameraBrightness - config.minCameraBrightness < 10) {
        if (errorMessage) {
            *errorMessage = QString("显示器 %1: 摄像头亮度范围过小 (%2-%3)，建议至少相差10个单位")
                .arg(monitorIndex)
                .arg(config.minCameraBrightness)
                .arg(config.maxCameraBrightness);
        }
        return false;
    }
    
    return true;
}

int AutoBrightness::mapBrightnessForMonitor(int monitorIndex, int cameraBrightness) {
    auto it = m_monitorConfigs.find(monitorIndex);
    if (it == m_monitorConfigs.end()) {
        // 使用默认线性曲线 (0,0) -> (100,100)
        qDebug() << "Monitor" << monitorIndex << "has no config, using default linear curve";
        return std::max(0, std::min(100, cameraBrightness));
    }
    
    const MonitorConfig& config = it->second;
    
    // 统一使用曲线插值
    if (config.curvePoints.size() >= 2) {
        return interpolateCurveForMonitor(monitorIndex, cameraBrightness);
    }
    
    // 如果没有曲线点，从线性配置生成临时曲线
    if (config.maxCameraBrightness == config.minCameraBrightness) {
        qWarning() << "Monitor" << monitorIndex << "min and max camera brightness are equal";
        return config.minScreenBrightness;
    }
    
    int clampedCamera = std::max(config.minCameraBrightness, 
                                 std::min(cameraBrightness, config.maxCameraBrightness));
    
    double ratio = static_cast<double>(clampedCamera - config.minCameraBrightness) / 
                   (config.maxCameraBrightness - config.minCameraBrightness);
    int screenBrightness = config.minScreenBrightness + 
                          static_cast<int>(ratio * (config.maxScreenBrightness - config.minScreenBrightness));
    
    return std::max(0, std::min(100, screenBrightness));
}

int AutoBrightness::interpolateCurveForMonitor(int monitorIndex, int cameraBrightness) {
    auto it = m_monitorConfigs.find(monitorIndex);
    if (it == m_monitorConfigs.end() || it->second.curvePoints.empty()) {
        return cameraBrightness;
    }
    
    const std::vector<std::pair<int, int>>& curvePoints = it->second.curvePoints;
    
    if (curvePoints.size() == 1) {
        return curvePoints[0].second;
    }
    
    for (int i = 0; i < static_cast<int>(curvePoints.size()) - 1; ++i) {
        if (cameraBrightness >= curvePoints[i].first && 
            cameraBrightness <= curvePoints[i + 1].first) {
            // 防止除零错误：检查两个点的 x 坐标是否相同
            int deltaX = curvePoints[i + 1].first - curvePoints[i].first;
            if (deltaX == 0) {
                // 如果两个点 x 坐标相同，返回第二个点的 y 值
                qWarning() << "Monitor" << monitorIndex << "has duplicate curve point x-coordinates at" << curvePoints[i].first;
                return curvePoints[i + 1].second;
            }
            
            double ratio = static_cast<double>(cameraBrightness - curvePoints[i].first) / deltaX;
            int screenBrightness = curvePoints[i].second + 
                                  static_cast<int>(ratio * (curvePoints[i + 1].second - curvePoints[i].second));
            return std::max(0, std::min(100, screenBrightness));
        }
    }
    
    if (cameraBrightness < curvePoints[0].first) {
        return curvePoints[0].second;
    }
    return curvePoints.back().second;
}

void AutoBrightness::saveSettings() {
    QSettings settings("AutoBrightnessWidget", "Settings");
    settings.setValue("minCameraBrightness", m_minCameraBrightness);
    settings.setValue("maxCameraBrightness", m_maxCameraBrightness);
    settings.setValue("minScreenBrightness", m_minScreenBrightness);
    settings.setValue("maxScreenBrightness", m_maxScreenBrightness);
    settings.setValue("useCurve", m_useCurve);
    settings.setValue("exposure", m_exposure);
    settings.setValue("captureInterval", m_captureInterval);
    settings.setValue("samplePoints", m_samplePoints);
    settings.setValue("gain", m_gain);  // 保存增益值
    
    // 保存曲线点
    settings.beginWriteArray("curvePoints");
    for (size_t i = 0; i < m_curvePoints.size(); ++i) {
        settings.setArrayIndex(static_cast<int>(i));
        settings.setValue("camera", m_curvePoints[i].first);
        settings.setValue("screen", m_curvePoints[i].second);
    }
    settings.endArray();
    
    // 保存多显示器配置
    settings.setValue("monitorCount", static_cast<int>(m_monitors.size()));
    settings.beginWriteArray("monitors");
    for (size_t i = 0; i < m_monitors.size(); ++i) {
        settings.setArrayIndex(static_cast<int>(i));
        settings.setValue("instanceName", m_monitors[i].instanceName);
        settings.setValue("friendlyName", m_monitors[i].friendlyName);
        settings.setValue("index", m_monitors[i].index);
    }
    settings.endArray();
    
    // 保存每个显示器的配置
    settings.beginWriteArray("monitorConfigs");
    int configIndex = 0;
    for (const auto& pair : m_monitorConfigs) {
        settings.setArrayIndex(configIndex++);
        settings.setValue("monitorIndex", pair.first);
        settings.setValue("minCameraBrightness", pair.second.minCameraBrightness);
        settings.setValue("maxCameraBrightness", pair.second.maxCameraBrightness);
        settings.setValue("minScreenBrightness", pair.second.minScreenBrightness);
        settings.setValue("maxScreenBrightness", pair.second.maxScreenBrightness);
        settings.setValue("useCurve", pair.second.useCurve);
        
        // 保存每个显示器的曲线点
        settings.beginWriteArray("curvePoints");
        for (size_t j = 0; j < pair.second.curvePoints.size(); ++j) {
            settings.setArrayIndex(static_cast<int>(j));
            settings.setValue("camera", pair.second.curvePoints[j].first);
            settings.setValue("screen", pair.second.curvePoints[j].second);
        }
        settings.endArray();
    }
    settings.endArray();
    
    qDebug() << "Settings saved (including" << m_monitorConfigs.size() << "monitor configs)";
}

void AutoBrightness::loadSettings() {
    QSettings settings("AutoBrightnessWidget", "Settings");
    m_minCameraBrightness = settings.value("minCameraBrightness", 0).toInt();
    m_maxCameraBrightness = settings.value("maxCameraBrightness", 100).toInt();
    m_minScreenBrightness = settings.value("minScreenBrightness", 0).toInt();
    m_maxScreenBrightness = settings.value("maxScreenBrightness", 100).toInt();
    m_useCurve = settings.value("useCurve", false).toBool();
    m_exposure = settings.value("exposure", -6.0).toDouble();
    m_captureInterval = settings.value("captureInterval", 10000).toInt();
    m_samplePoints = settings.value("samplePoints", 5).toInt();
    m_gain = settings.value("gain", 0.0).toDouble();  // 加载增益值
    
    // 加载曲线点
    m_curvePoints.clear();
    int size = settings.beginReadArray("curvePoints");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        int camera = settings.value("camera").toInt();
        int screen = settings.value("screen").toInt();
        m_curvePoints.push_back(std::make_pair(camera, screen));
    }
    settings.endArray();
    
    // 加载多显示器配置
    m_monitors.clear();
    int monitorCount = settings.value("monitorCount", 0).toInt();
    int loadedMonitors = settings.beginReadArray("monitors");
    for (int i = 0; i < loadedMonitors; ++i) {
        settings.setArrayIndex(i);
        QString instanceName = settings.value("instanceName").toString();
        QString friendlyName = settings.value("friendlyName").toString();
        int index = settings.value("index").toInt();
        m_monitors.push_back(MonitorInfo(instanceName, friendlyName, index));
    }
    settings.endArray();
    
    // 加载每个显示器的配置
    m_monitorConfigs.clear();
    int configCount = settings.beginReadArray("monitorConfigs");
    for (int i = 0; i < configCount; ++i) {
        settings.setArrayIndex(i);
        int monitorIndex = settings.value("monitorIndex").toInt();
        
        MonitorConfig config;
        config.minCameraBrightness = settings.value("minCameraBrightness", 0).toInt();
        config.maxCameraBrightness = settings.value("maxCameraBrightness", 100).toInt();
        config.minScreenBrightness = settings.value("minScreenBrightness", 0).toInt();
        config.maxScreenBrightness = settings.value("maxScreenBrightness", 100).toInt();
        config.useCurve = settings.value("useCurve", false).toBool();
        
        // 加载曲线点
        config.curvePoints.clear();
        int curveSize = settings.beginReadArray("curvePoints");
        for (int j = 0; j < curveSize; ++j) {
            settings.setArrayIndex(j);
            int camera = settings.value("camera").toInt();
            int screen = settings.value("screen").toInt();
            config.curvePoints.push_back(std::make_pair(camera, screen));
        }
        settings.endArray();
        
        m_monitorConfigs[monitorIndex] = config;
    }
    settings.endArray();
    
    // 配置迁移：将旧的线性配置转换为曲线
    migrateLinearToCurve();
    
    qDebug() << "Settings loaded (including" << m_monitorConfigs.size() << "monitor configs)";
}

void AutoBrightness::migrateLinearToCurve() {
    bool needsSave = false;
    
    // 迁移全局配置（向后兼容）
    if (!m_useCurve && m_curvePoints.size() < 2) {
        // 从线性配置创建两点曲线
        m_curvePoints.clear();
        m_curvePoints.push_back(std::make_pair(m_minCameraBrightness, m_minScreenBrightness));
        m_curvePoints.push_back(std::make_pair(m_maxCameraBrightness, m_maxScreenBrightness));
        m_useCurve = true;
        needsSave = true;
        qDebug() << "Migrated global linear config to curve:" 
                 << "(" << m_minCameraBrightness << "," << m_minScreenBrightness << ")" 
                 << "to" 
                 << "(" << m_maxCameraBrightness << "," << m_maxScreenBrightness << ")";
    }
    
    // 迁移每个显示器的配置
    for (auto& pair : m_monitorConfigs) {
        MonitorConfig& config = pair.second;
        if (!config.useCurve && config.curvePoints.size() < 2) {
            // 从线性配置创建两点曲线
            config.curvePoints.clear();
            config.curvePoints.push_back(std::make_pair(config.minCameraBrightness, config.minScreenBrightness));
            config.curvePoints.push_back(std::make_pair(config.maxCameraBrightness, config.maxScreenBrightness));
            config.useCurve = true;
            needsSave = true;
            qDebug() << "Migrated monitor" << pair.first << "linear config to curve:" 
                     << "(" << config.minCameraBrightness << "," << config.minScreenBrightness << ")" 
                     << "to" 
                     << "(" << config.maxCameraBrightness << "," << config.maxScreenBrightness << ")";
        }
    }
    
    if (needsSave) {
        saveSettings();
        qDebug() << "Migration complete, settings saved";
    }
}

#include "moc_AutoBrightness.cpp"
