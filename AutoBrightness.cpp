#include <iostream>
#include "AutoBrightness.h"
#include <QString>
#include <QProcess>


void AutoBrightness::setBrightness(int brightness) {
    std::string command = "(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1, ";
    command += std::to_string(brightness);
    command += ")";

    QStringList arguments;
    arguments << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-WindowStyle" << "Hidden" << "-Command" << command.c_str();

    QProcess process;
    qDebug()<<brightness;
    process.start("powershell.exe", arguments);
    process.waitForFinished();
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
    
    qDebug() << "Camera opened with exposure:" << m_exposure;
}

void AutoBrightness::releaseCap(){
    m_cap.release();
}

void AutoBrightness::update(){

    cv::Mat frame;
    m_cap >> frame; // 获取摄像头帧

    // 计算平均亮度并设置屏幕亮度
    int cameraBrightness = getBrightness(frame);
    m_currentCameraBrightness = cameraBrightness;
    
    // 应用映射（线性或曲线）
    int screenBrightness = mapBrightness(cameraBrightness);
    m_currentScreenBrightness = screenBrightness;
    
    setBrightness(screenBrightness);
    
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

void AutoBrightness::setMinBrightness(int cameraBrightness, int screenBrightness) {
    m_minCameraBrightness = cameraBrightness;
    m_minScreenBrightness = screenBrightness;
    qDebug() << "Min brightness set: camera=" << cameraBrightness << ", screen=" << screenBrightness;
}

void AutoBrightness::setMaxBrightness(int cameraBrightness, int screenBrightness) {
    m_maxCameraBrightness = cameraBrightness;
    m_maxScreenBrightness = screenBrightness;
    qDebug() << "Max brightness set: camera=" << cameraBrightness << ", screen=" << screenBrightness;
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
    for (size_t i = 0; i < m_curvePoints.size() - 1; ++i) {
        if (cameraBrightness >= m_curvePoints[i].first && 
            cameraBrightness <= m_curvePoints[i + 1].first) {
            // 线性插值
            double ratio = static_cast<double>(cameraBrightness - m_curvePoints[i].first) / 
                          (m_curvePoints[i + 1].first - m_curvePoints[i].first);
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
    
    // 保存曲线点
    settings.beginWriteArray("curvePoints");
    for (size_t i = 0; i < m_curvePoints.size(); ++i) {
        settings.setArrayIndex(static_cast<int>(i));
        settings.setValue("camera", m_curvePoints[i].first);
        settings.setValue("screen", m_curvePoints[i].second);
    }
    settings.endArray();
    
    qDebug() << "Settings saved";
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
    
    qDebug() << "Settings loaded";
}

#include "moc_AutoBrightness.cpp"
