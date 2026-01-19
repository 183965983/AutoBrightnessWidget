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
#include <cmath>
#include "AutoBrightness.h"
#include "CurveEditorDialog.h"

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
{
    ui->setupUi(this);
    
    // 设置窗口标题
    setWindowTitle("Auto Brightness Widget");
    
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
    ui->useCurveCheckBox->setChecked(m_autoBrightness->getUseCurve());
    
    // 初始化显示
    updateCameraBrightness(0);
    updateScreenBrightness(0);
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
    
    m_running = true;
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
    if(m_autoBrightThread){
        m_autoBrightThread->wait();
        delete m_autoBrightThread;
        m_autoBrightThread = nullptr;
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

void MainWindow::on_setMinBrightnessButton_clicked()
{
    int cameraBrightness = m_autoBrightness->getCurrentCameraBrightness();
    int screenBrightness = ui->minScreenBrightnessSpinBox->value();
    m_autoBrightness->setMinBrightness(cameraBrightness, screenBrightness);
    
    // 验证配置
    QString errorMessage;
    if (!m_autoBrightness->validateBrightnessConfiguration(&errorMessage)) {
        QMessageBox::warning(this, "配置警告", 
            QString("最小亮度已配置，但当前配置可能无效:\n\n%1\n\n"
                    "请配置有效的最大亮度以确保正确映射。")
            .arg(errorMessage));
    } else {
        m_autoBrightness->saveSettings();
        QMessageBox::information(this, "配置成功", 
            QString("最小亮度已配置:\n摄像头亮度: %1\n屏幕亮度: %2")
            .arg(cameraBrightness).arg(screenBrightness));
    }
}

void MainWindow::on_setMaxBrightnessButton_clicked()
{
    int cameraBrightness = m_autoBrightness->getCurrentCameraBrightness();
    int screenBrightness = ui->maxScreenBrightnessSpinBox->value();
    m_autoBrightness->setMaxBrightness(cameraBrightness, screenBrightness);
    
    // 验证配置
    QString errorMessage;
    if (!m_autoBrightness->validateBrightnessConfiguration(&errorMessage)) {
        QMessageBox::warning(this, "配置警告", 
            QString("最大亮度已配置，但当前配置可能无效:\n\n%1\n\n"
                    "请检查并调整配置以确保正确映射。")
            .arg(errorMessage));
    } else {
        m_autoBrightness->saveSettings();
        QMessageBox::information(this, "配置成功", 
            QString("最大亮度已配置:\n摄像头亮度: %1\n屏幕亮度: %2")
            .arg(cameraBrightness).arg(screenBrightness));
    }
}

void MainWindow::on_useCurveCheckBox_toggled(bool checked)
{
    m_autoBrightness->setUseCurve(checked);
    m_autoBrightness->saveSettings();
}

void MainWindow::on_editCurveButton_clicked()
{
    CurveEditorDialog dialog(this);
    dialog.setCurvePoints(m_autoBrightness->getCurvePoints());
    
    if (dialog.exec() == QDialog::Accepted) {
        auto points = dialog.getCurvePoints();
        m_autoBrightness->setCurvePoints(points);
        m_autoBrightness->saveSettings();
        
        // 如果用户编辑了曲线，自动启用曲线模式
        if (points.size() > 2) {
            ui->useCurveCheckBox->setChecked(true);
            m_autoBrightness->setUseCurve(true);
        }
        
        QMessageBox::information(this, "曲线已保存", 
            QString("曲线控制点数量: %1\n曲线映射已更新并保存").arg(points.size()));
    }
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

