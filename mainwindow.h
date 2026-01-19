#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QLabel>
#include "AutoBrightness.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_exposureSlider_valueChanged(int value);
    void on_intervalSlider_valueChanged(int value);
    void on_samplePointsSlider_valueChanged(int value);
    
    // 新增槽函数
    void on_setMinBrightnessButton_clicked();
    void on_setMaxBrightnessButton_clicked();
    void on_useCurveCheckBox_toggled(bool checked);
    void on_autoStartCheckBox_toggled(bool checked);
    void on_minimizeToTrayCheckBox_toggled(bool checked);
    void on_editCurveButton_clicked();  // 新增：打开曲线编辑器
    
    // 亮度更新槽
    void updateCameraBrightness(int brightness);
    void updateScreenBrightness(int brightness);
    
    // 系统托盘槽
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWindowFromTray();
    void quitApplication();
    void startFromTray();
    void stopFromTray();
    
    // 更新检查槽
    void onUpdateAvailable(const QString& latestVersion,
                          const QString& releaseUrl,
                          const QString& releaseNotes);
    void onNoUpdateAvailable();
    void onUpdateCheckFailed(const QString& errorMessage);

private:
    void setupSystemTray();
    void setAutoStart(bool enable);
    bool isAutoStartEnabled();
    QIcon createTrayIcon();
    void updateStatusBar();
    void updateButtonStates();
    void updateLastUpdateTime();
    void updateTrayMenu();  // 更新托盘菜单状态
    void checkForUpdates();
    
    Ui::MainWindow *ui;
    QThread* m_autoBrightThread;
    AutoBrightness* m_autoBrightness;
    bool m_running;
    
    // 系统托盘
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    QAction* m_startAction;  // 启动菜单项
    QAction* m_stopAction;   // 停止菜单项
    bool m_minimizeToTray;
    
    // 状态栏标签
    QLabel* m_statusLabel;
    QLabel* m_lastUpdateLabel;
};
#endif // MAINWINDOW_H
