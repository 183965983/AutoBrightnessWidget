#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QThread>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QPixmap>
#include <QPainter>
#include <QIcon>
#include <QTimer>
#include <QLabel>
#include <QTime>
#include <cmath>
#include "AutoBrightness.h"
#include "CurveEditorDialog.h"
#include "UpdateChecker.h"
#include "UpdateDialog.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_running(false)
    , m_autoBrightness(AutoBrightness::getInstance())
    , m_autoBrightThread(nullptr)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_startAction(nullptr)
    , m_stopAction(nullptr)
    , m_minimizeToTray(false)
    , m_statusLabel(nullptr)
    , m_lastUpdateLabel(nullptr)
    , m_curveWidget(nullptr)
    , m_currentMonitorIndex(-1)
{
    ui->setupUi(this);
    
    // 设置窗口标题
    setWindowTitle("Auto Brightness Widget");
    
    // 创建并初始化实时亮度曲线组件
    m_curveWidget = new BrightnessCurveWidget(this);
    // 替换UI中的占位符
    QWidget* placeholder = ui->curveWidgetPlaceholder;
    if (placeholder && placeholder->parentWidget()) {
        QVBoxLayout* curveLayout = qobject_cast<QVBoxLayout*>(placeholder->parentWidget()->layout());
        if (curveLayout) {
            // 移除占位符并添加实际的曲线widget
            curveLayout->removeWidget(placeholder);
            placeholder->hide();
            curveLayout->addWidget(m_curveWidget);
        }
    }
    
    // 设置状态栏
    m_statusLabel = new QLabel("状态: 已停止", this);
    m_lastUpdateLabel = new QLabel("最后更新: --", this);
    ui->statusbar->addWidget(m_statusLabel);
    ui->statusbar->addPermanentWidget(m_lastUpdateLabel);
    
    // 连接信号槽
    connect(m_autoBrightness, &AutoBrightness::cameraBrightnessChanged,
            this, &MainWindow::updateCameraBrightness);
    connect(m_autoBrightness, &AutoBrightness::screenBrightnessChanged,
            this, &MainWindow::updateScreenBrightness);
    
    // 设置系统托盘
    setupSystemTray();
    
    // 加载设置
    QSettings settings("AutoBrightnessWidget", "Settings");
    m_minimizeToTray = settings.value("minimizeToTray", false).toBool();
    ui->minimizeToTrayCheckBox->setChecked(m_minimizeToTray);
    ui->autoStartCheckBox->setChecked(isAutoStartEnabled());
    
    // 初始化增益和采样频率控件的显示值
    ui->gainValueLabel->setText(QString::number(static_cast<int>(m_autoBrightness->getGain())));
    // 初始化采样频率标签（根据当前间隔计算频率）
    double currentFreq = 1000.0 / m_autoBrightness->getCaptureInterval();
    updateSamplingFreqLabel(currentFreq);
    
    // 初始化显示器列表（后台异步）
    // 注意：在主线程启动前不刷新，避免阻塞 UI
    // 初始化显示器列表
    refreshMonitorList();
    
    // 初始化显示
    updateCameraBrightness(0);
    updateScreenBrightness(0);
    
    // 初始化按钮状态
    updateButtonStates();
    
    // 窗口显示后延迟检查更新（避免阻塞启动）
    QTimer::singleShot(1000, this, &MainWindow::checkForUpdates);
}

MainWindow::~MainWindow()
{
    m_running = false;
    if(m_autoBrightThread){
        m_autoBrightThread->wait();
        delete m_autoBrightThread;
        m_autoBrightThread = nullptr;
    }
    
    // 清理托盘图标
    if (m_trayIcon) {
        m_trayIcon->hide();
        delete m_trayIcon;
    }
    if (m_trayMenu) {
        delete m_trayMenu;
    }
    
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if(m_running){
        return; // Already running
    }
    
    // 刷新显示器列表
    m_autoBrightness->refreshMonitors();
    if (m_autoBrightness->getMonitorCount() == 0) {
        QMessageBox::warning(this, "警告", 
                           "未检测到支持亮度调节的显示器。\n"
                           "请确保：\n"
                           "1. 显示器已连接并开启\n"
                           "2. 显示器支持 DDC/CI 协议\n"
                           "3. 在显示器 OSD 菜单中启用了 DDC/CI");
        return;
    }
    
    m_running = true;
    updateStatusBar();
    updateButtonStates();
    
    m_autoBrightThread = QThread::create(
        [this](){
            AutoBrightness::getInstance()->openCap();
            while(m_running){
                AutoBrightness::getInstance()->update();
                QThread::msleep(AutoBrightness::getInstance()->getCaptureInterval());
            }
            AutoBrightness::getInstance()->releaseCap();
        });
    m_autoBrightThread->start();
    
    // 更新托盘菜单状态
    updateTrayMenu();

}


void MainWindow::on_pushButton_2_clicked()
{
    m_running = false;
    updateStatusBar();
    updateButtonStates();
    
    if(m_autoBrightThread){
        m_autoBrightThread->wait();
        delete m_autoBrightThread;
        m_autoBrightThread = nullptr;
    }
    
    // 清除实时亮度曲线数据
    if (m_curveWidget) {
        m_curveWidget->clearData();
    }
    
    // 更新托盘菜单状态
    updateTrayMenu();

}

void MainWindow::on_exposureSlider_valueChanged(int value)
{
    // 曝光值范围通常是 -13 到 -1，我们将滑块值(0-100)映射到这个范围
    double exposure = -13.0 + (value / 100.0) * 12.0;
    m_autoBrightness->setExposure(exposure);
    ui->exposureValueLabel->setText(QString::number(exposure, 'f', 1));
}

void MainWindow::on_intervalSlider_valueChanged(int value)
{
    // 间隔范围：1秒到60秒，滑块值直接对应秒数
    int intervalMs = value * 1000;
    m_autoBrightness->setCaptureInterval(intervalMs);
    ui->intervalValueLabel->setText(QString::number(value) + "s");
}

void MainWindow::on_samplePointsSlider_valueChanged(int value)
{
    // 采样点数范围：2到20
    m_autoBrightness->setSamplePoints(value);
    ui->samplePointsValueLabel->setText(QString::number(value) + "x" + QString::number(value));
}

void MainWindow::on_gainSlider_valueChanged(int value)
{
    // 增益值范围：0到255，滑块值直接对应增益值
    double gain = static_cast<double>(value);
    m_autoBrightness->setGain(gain);
    ui->gainValueLabel->setText(QString::number(value));
}

void MainWindow::on_samplingFreqSlider_valueChanged(int value)
{
    // 采样频率范围：0.01Hz到60Hz（对数刻度）
    // 使用对数映射：slider value 0-100 -> frequency 0.01-60 Hz
    // log10(0.01) = -2, log10(60) ≈ 1.778
    // 线性映射到对数空间
    const double logMin = -2.0;  // log10(0.01)
    const double logMax = std::log10(60.0);  // log10(60) = 1.778...
    double logFreq = logMin + (value / 100.0) * (logMax - logMin);
    double freqHz = std::pow(10.0, logFreq);
    
    // 防止除零错误
    if (freqHz <= 0.0) {
        freqHz = 0.01;  // 最小频率
    }
    
    // 转换为毫秒间隔
    int intervalMs = static_cast<int>(1000.0 / freqHz);
    m_autoBrightness->setCaptureInterval(intervalMs);
    
    // 更新UI标签
    updateSamplingFreqLabel(freqHz);
    
    // 更新曝光时间限制
    updateExposureLimits(freqHz);
}

void MainWindow::on_setMinBrightnessButton_clicked()
void MainWindow::on_editCurveButton_clicked()
{
    if (m_currentMonitorIndex < 0) {
        QMessageBox::warning(this, "警告", "请先选择一个显示器");
        return;
    }
    
    CurveEditorDialog dialog(this);
    dialog.setCurvePoints(m_autoBrightness->getMonitorConfig(m_currentMonitorIndex).curvePoints);
    
    if (dialog.exec() == QDialog::Accepted) {
        auto points = dialog.getCurvePoints();
        m_autoBrightness->setMonitorCurvePoints(m_currentMonitorIndex, points);
        m_autoBrightness->saveSettings();
        
        QMessageBox::information(this, "曲线已保存", 
            QString("曲线控制点数量: %1\n曲线映射已更新并保存").arg(points.size()));
    }
}

void MainWindow::on_monitorSelectComboBox_currentIndexChanged(int index)
{
    if (index < 0) {
        return;
    }
    
    std::vector<MonitorInfo> monitors = m_autoBrightness->getMonitors();
    if (index < static_cast<int>(monitors.size())) {
        m_currentMonitorIndex = monitors[index].index;
        loadMonitorConfig(m_currentMonitorIndex);
    }
}

void MainWindow::on_refreshMonitorsButton_clicked()
{
    refreshMonitorList();
}

void MainWindow::on_autoStartCheckBox_toggled(bool checked)
{
    setAutoStart(checked);
}

void MainWindow::on_minimizeToTrayCheckBox_toggled(bool checked)
{
    m_minimizeToTray = checked;
    QSettings settings("AutoBrightnessWidget", "Settings");
    settings.setValue("minimizeToTray", checked);
}

void MainWindow::updateCameraBrightness(int brightness)
{
    ui->cameraBrightnessLabel->setText(QString::number(brightness));
    
    // 更新灰阶色块 - 使用浮点运算避免精度损失
    int grayValue = static_cast<int>(brightness * 255.0 / 100.0);
    QString styleSheet = QString("background-color: rgb(%1, %1, %1);").arg(grayValue);
    ui->cameraBrightnessBlock->setStyleSheet(styleSheet);
    
    // 更新实时亮度曲线（仅在运行时）
    if (m_running && m_curveWidget) {
        int screenBrightness = m_autoBrightness->getCurrentScreenBrightness();
        m_curveWidget->addDataPoint(brightness, screenBrightness);
    }
    
    // 更新状态栏的最后更新时间
    if (m_running) {
        updateLastUpdateTime();
    }
}

void MainWindow::updateScreenBrightness(int brightness)
{
    ui->screenBrightnessLabel->setText(QString::number(brightness));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_minimizeToTray && m_running && m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        m_trayIcon->showMessage("Auto Brightness Widget", 
                               "程序已最小化到托盘，后台继续运行",
                               QSystemTrayIcon::Information, 2000);
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::setupSystemTray()
{
    m_trayIcon = new QSystemTrayIcon(this);
    
    // 设置自定义图标
    m_trayIcon->setIcon(createTrayIcon());
    
    // 设置工具提示
    m_trayIcon->setToolTip("Auto Brightness Widget - 自动亮度调节工具");
    
    // 创建托盘菜单
    m_trayMenu = new QMenu(this);
    
    // 添加启动和停止菜单项
    m_startAction = m_trayMenu->addAction("启动");
    m_stopAction = m_trayMenu->addAction("停止");
    m_trayMenu->addSeparator();
    
    QAction *showAction = m_trayMenu->addAction("显示窗口");
    QAction *quitAction = m_trayMenu->addAction("退出");
    
    // 连接信号槽
    connect(m_startAction, &QAction::triggered, this, &MainWindow::startFromTray);
    connect(m_stopAction, &QAction::triggered, this, &MainWindow::stopFromTray);
    connect(showAction, &QAction::triggered, this, &MainWindow::showWindowFromTray);
    connect(quitAction, &QAction::triggered, this, &MainWindow::quitApplication);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();
    
    // 初始化菜单状态
    updateTrayMenu();
}

QIcon MainWindow::createTrayIcon()
{
    // 图标尺寸和几何常量
    constexpr int ICON_SIZE = 16;
    constexpr int CENTER = ICON_SIZE / 2;
    constexpr int NUM_RAYS = 8;
    constexpr double ANGLE_STEP = 360.0 / NUM_RAYS;  // 45度间隔
    constexpr double INNER_RADIUS = 5.0;
    constexpr double OUTER_RADIUS = 7.0;
    constexpr int CORE_RADIUS = 4;
    
    // 创建图标
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制太阳图标（代表亮度）
    // 外圈光芒
    painter.setPen(QPen(QColor(255, 200, 0), 1));
    for (int i = 0; i < NUM_RAYS; ++i) {
        double angle = i * ANGLE_STEP * M_PI / 180.0;
        int x1 = CENTER + static_cast<int>(INNER_RADIUS * std::cos(angle));
        int y1 = CENTER + static_cast<int>(INNER_RADIUS * std::sin(angle));
        int x2 = CENTER + static_cast<int>(OUTER_RADIUS * std::cos(angle));
        int y2 = CENTER + static_cast<int>(OUTER_RADIUS * std::sin(angle));
        painter.drawLine(x1, y1, x2, y2);
    }
    
    // 中心圆（太阳核心）
    painter.setPen(QPen(QColor(255, 180, 0), 1));
    painter.setBrush(QBrush(QColor(255, 220, 0)));
    painter.drawEllipse(QPoint(CENTER, CENTER), CORE_RADIUS, CORE_RADIUS);
    
    return QIcon(pixmap);
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        showWindowFromTray();
    }
}

void MainWindow::showWindowFromTray()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::startFromTray()
{
    if (!m_running) {
        on_pushButton_clicked();  // 调用现有的启动逻辑
        // updateTrayMenu() 已在 on_pushButton_clicked() 中调用
    }
}

void MainWindow::stopFromTray()
{
    if (m_running) {
        on_pushButton_2_clicked();  // 调用现有的停止逻辑
        // updateTrayMenu() 已在 on_pushButton_2_clicked() 中调用
    }
}

void MainWindow::updateTrayMenu()
{
    // 根据运行状态更新菜单项的启用状态
    if (m_startAction && m_stopAction) {
        m_startAction->setEnabled(!m_running);
        m_stopAction->setEnabled(m_running);
    }
}

void MainWindow::quitApplication()
{
    m_running = false;
    if (m_autoBrightThread) {
        m_autoBrightThread->wait();
        delete m_autoBrightThread;
        m_autoBrightThread = nullptr;
    }
    QApplication::quit();
}

void MainWindow::setAutoStart(bool enable)
{
#ifdef Q_OS_WIN
    static const QString AUTO_START_REGISTRY_PATH = 
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    
    QSettings settings(AUTO_START_REGISTRY_PATH, QSettings::NativeFormat);
    if (enable) {
        QString appPath = QCoreApplication::applicationFilePath();
        appPath = QDir::toNativeSeparators(appPath);
        settings.setValue("AutoBrightnessWidget", appPath);
    } else {
        settings.remove("AutoBrightnessWidget");
    }
#endif
}

bool MainWindow::isAutoStartEnabled()
{
#ifdef Q_OS_WIN
    static const QString AUTO_START_REGISTRY_PATH = 
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    
    QSettings settings(AUTO_START_REGISTRY_PATH, QSettings::NativeFormat);
    return settings.contains("AutoBrightnessWidget");
#else
    return false;
#endif
}

void MainWindow::checkForUpdates()
{
    // 创建更新检查器
    // 使用 CMakeLists.txt 中定义的版本号和仓库信息
    UpdateChecker* updateChecker = new UpdateChecker(
        APP_VERSION,
        GITHUB_OWNER,
        GITHUB_REPO,
        this
    );
    
    // 连接信号
    connect(updateChecker, &UpdateChecker::updateAvailable,
            this, &MainWindow::onUpdateAvailable);
    connect(updateChecker, &UpdateChecker::noUpdateAvailable,
            this, &MainWindow::onNoUpdateAvailable);
    connect(updateChecker, &UpdateChecker::checkFailed,
            this, &MainWindow::onUpdateCheckFailed);
    
    // 开始检查
    updateChecker->checkForUpdates();
}

void MainWindow::onUpdateAvailable(const QString& latestVersion,
                                   const QString& releaseUrl,
                                   const QString& releaseNotes)
{
    // 显示更新对话框
    UpdateDialog dialog(APP_VERSION, latestVersion, releaseUrl, releaseNotes, this);
    dialog.exec();
}

void MainWindow::onNoUpdateAvailable()
{
    // 静默处理，不打扰用户
    qDebug() << "已是最新版本";
}

void MainWindow::onUpdateCheckFailed(const QString& errorMessage)
{
    // 更新检查失败，静默记录日志即可
    qDebug() << "更新检查失败:" << errorMessage;
}

void MainWindow::updateStatusBar()
{
    if (!m_statusLabel || !m_lastUpdateLabel) {
        return;
    }
    
    if (m_running) {
        m_statusLabel->setText("状态: 正在运行");
        updateLastUpdateTime();
    } else {
        m_statusLabel->setText("状态: 已停止");
        m_lastUpdateLabel->setText("最后更新: --");
    }
}

void MainWindow::updateLastUpdateTime()
{
    if (!m_lastUpdateLabel) {
        return;
    }
    
    QTime currentTime = QTime::currentTime();
    m_lastUpdateLabel->setText(QString("最后更新: %1").arg(currentTime.toString("HH:mm:ss")));
}

void MainWindow::updateButtonStates()
{
    // 更新启动按钮
    if (m_running) {
        ui->pushButton->setEnabled(false);
        ui->pushButton->setToolTip("自动亮度调节正在运行中");
    } else {
        ui->pushButton->setEnabled(true);
        ui->pushButton->setToolTip("点击启动自动亮度调节");
    }
    
    // 更新停止按钮
    if (m_running) {
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_2->setToolTip("点击停止自动亮度调节");
    } else {
        ui->pushButton_2->setEnabled(false);
        ui->pushButton_2->setToolTip("自动亮度调节未运行");
    }
}

void MainWindow::updateSamplingFreqLabel(double freqHz)
{
    QString freqText;
    if (freqHz < 1.0) {
        // 显示为毫赫兹（mHz）
        freqText = QString::number(freqHz * 1000.0, 'f', 1) + " mHz";
    } else {
        freqText = QString::number(freqHz, 'f', 2) + " Hz";
    }
    ui->samplingFreqValueLabel->setText(freqText);
}

void MainWindow::updateExposureLimits(double freqHz)
{
    // 曝光时间不应超过采样间隔的一半，以避免冲突
    // 采样间隔 = 1 / freqHz 秒
    double maxExposureTime = 0.5 / freqHz;  // 秒
    
    // 相机曝光值通常是对数刻度，这里做简单提示
    // 实际限制需要根据具体相机特性调整
    qDebug() << "Sampling frequency:" << freqHz << "Hz, max recommended exposure time:" << maxExposureTime << "s";
    
    // 可以在这里添加UI提示或自动调整曝光滑块的最大值
    // 例如：如果当前曝光时间过长，给出警告
}
void MainWindow::refreshMonitorList()
{
    ui->monitorSelectComboBox->clear();
    m_autoBrightness->refreshMonitors();
    
    std::vector<MonitorInfo> monitors = m_autoBrightness->getMonitors();
    
    if (monitors.empty()) {
        ui->monitorSelectComboBox->addItem("未检测到显示器");
        ui->monitorSelectComboBox->setEnabled(false);
        ui->editCurveButton->setEnabled(false);
        m_currentMonitorIndex = -1;
        return;
    }
    
    ui->monitorSelectComboBox->setEnabled(true);
    ui->editCurveButton->setEnabled(true);
    
    for (const MonitorInfo& monitor : monitors) {
        QString itemText = QString("%1").arg(monitor.friendlyName);
        ui->monitorSelectComboBox->addItem(itemText);
    }
    
    // 选中第一个显示器
    if (!monitors.empty()) {
        ui->monitorSelectComboBox->setCurrentIndex(0);
        m_currentMonitorIndex = monitors[0].index;
        loadMonitorConfig(m_currentMonitorIndex);
    }
}

void MainWindow::loadMonitorConfig(int monitorIndex)
{
    if (monitorIndex < 0) {
        return;
    }
    
    MonitorConfig config = m_autoBrightness->getMonitorConfig(monitorIndex);
    
    // 目前只需要加载配置用于曲线编辑器
    // 其他UI元素已经被移除
}


