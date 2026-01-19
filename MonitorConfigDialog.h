#ifndef MONITORCONFIGDIALOG_H
#define MONITORCONFIGDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include "AutoBrightness.h"

class MonitorConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MonitorConfigDialog(QWidget *parent = nullptr);
    ~MonitorConfigDialog();

private slots:
    void onMonitorSelectionChanged();
    void onRefreshMonitorsClicked();
    void onSetMinBrightnessClicked();
    void onSetMaxBrightnessClicked();
    void onUseCurveToggled(bool checked);
    void onEditCurveClicked();
    void onApplyClicked();
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();
    void refreshMonitorList();
    void loadMonitorConfig(int monitorIndex);
    void saveCurrentMonitorConfig();
    int getCurrentMonitorIndex() const;

private:
    AutoBrightness* m_autoBrightness;
    
    // UI 组件
    QListWidget* m_monitorList;
    QPushButton* m_refreshButton;
    
    // 配置组
    QGroupBox* m_configGroup;
    QLabel* m_currentBrightnessLabel;
    
    // 最小/最大亮度配置
    QSpinBox* m_minCameraSpinBox;
    QSpinBox* m_minScreenSpinBox;
    QSpinBox* m_maxCameraSpinBox;
    QSpinBox* m_maxScreenSpinBox;
    QPushButton* m_setMinButton;
    QPushButton* m_setMaxButton;
    
    // 曲线配置
    QCheckBox* m_useCurveCheckBox;
    QPushButton* m_editCurveButton;
    
    // 按钮
    QPushButton* m_applyButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    // 当前选中的显示器索引
    int m_currentMonitorIndex;
    
    // 临时配置存储
    std::map<int, MonitorConfig> m_tempConfigs;
};

#endif // MONITORCONFIGDIALOG_H
