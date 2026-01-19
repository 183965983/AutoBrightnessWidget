#include "MonitorConfigDialog.h"
#include "CurveEditorDialog.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>
#include <QFormLayout>

MonitorConfigDialog::MonitorConfigDialog(QWidget *parent)
    : QDialog(parent)
    , m_autoBrightness(AutoBrightness::getInstance())
    , m_currentMonitorIndex(-1)
{
    setupUI();
    refreshMonitorList();
    
    setWindowTitle(tr("多显示器配置"));
    resize(800, 600);
}

MonitorConfigDialog::~MonitorConfigDialog()
{
}

void MonitorConfigDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 显示器列表区域
    QGroupBox* monitorListGroup = new QGroupBox(tr("显示器列表"), this);
    QVBoxLayout* listLayout = new QVBoxLayout(monitorListGroup);
    
    m_monitorList = new QListWidget(this);
    m_monitorList->setMaximumHeight(150);
    connect(m_monitorList, &QListWidget::currentRowChanged, 
            this, &MonitorConfigDialog::onMonitorSelectionChanged);
    listLayout->addWidget(m_monitorList);
    
    m_refreshButton = new QPushButton(tr("刷新显示器列表"), this);
    connect(m_refreshButton, &QPushButton::clicked, 
            this, &MonitorConfigDialog::onRefreshMonitorsClicked);
    listLayout->addWidget(m_refreshButton);
    
    mainLayout->addWidget(monitorListGroup);
    
    // 配置区域
    m_configGroup = new QGroupBox(tr("显示器配置"), this);
    m_configGroup->setEnabled(false);
    QVBoxLayout* configLayout = new QVBoxLayout(m_configGroup);
    
    // 当前亮度显示
    m_currentBrightnessLabel = new QLabel(tr("当前屏幕亮度: --"), this);
    m_currentBrightnessLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    configLayout->addWidget(m_currentBrightnessLabel);
    
    // 最小亮度配置
    QGroupBox* minGroup = new QGroupBox(tr("最小亮度映射"), this);
    QFormLayout* minLayout = new QFormLayout(minGroup);
    
    m_minCameraSpinBox = new QSpinBox(this);
    m_minCameraSpinBox->setRange(0, 100);
    m_minCameraSpinBox->setSuffix(" %");
    minLayout->addRow(tr("摄像头亮度:"), m_minCameraSpinBox);
    
    m_minScreenSpinBox = new QSpinBox(this);
    m_minScreenSpinBox->setRange(0, 100);
    m_minScreenSpinBox->setSuffix(" %");
    minLayout->addRow(tr("屏幕亮度:"), m_minScreenSpinBox);
    
    m_setMinButton = new QPushButton(tr("设置最小亮度"), this);
    connect(m_setMinButton, &QPushButton::clicked,
            this, &MonitorConfigDialog::onSetMinBrightnessClicked);
    minLayout->addRow(m_setMinButton);
    
    configLayout->addWidget(minGroup);
    
    // 最大亮度配置
    QGroupBox* maxGroup = new QGroupBox(tr("最大亮度映射"), this);
    QFormLayout* maxLayout = new QFormLayout(maxGroup);
    
    m_maxCameraSpinBox = new QSpinBox(this);
    m_maxCameraSpinBox->setRange(0, 100);
    m_maxCameraSpinBox->setSuffix(" %");
    maxLayout->addRow(tr("摄像头亮度:"), m_maxCameraSpinBox);
    
    m_maxScreenSpinBox = new QSpinBox(this);
    m_maxScreenSpinBox->setRange(0, 100);
    m_maxScreenSpinBox->setSuffix(" %");
    maxLayout->addRow(tr("屏幕亮度:"), m_maxScreenSpinBox);
    
    m_setMaxButton = new QPushButton(tr("设置最大亮度"), this);
    connect(m_setMaxButton, &QPushButton::clicked,
            this, &MonitorConfigDialog::onSetMaxBrightnessClicked);
    maxLayout->addRow(m_setMaxButton);
    
    configLayout->addWidget(maxGroup);
    
    // 曲线配置
    QGroupBox* curveGroup = new QGroupBox(tr("亮度曲线"), this);
    QVBoxLayout* curveLayout = new QVBoxLayout(curveGroup);
    
    m_useCurveCheckBox = new QCheckBox(tr("使用自定义曲线"), this);
    connect(m_useCurveCheckBox, &QCheckBox::toggled,
            this, &MonitorConfigDialog::onUseCurveToggled);
    curveLayout->addWidget(m_useCurveCheckBox);
    
    m_editCurveButton = new QPushButton(tr("编辑曲线"), this);
    m_editCurveButton->setEnabled(false);
    connect(m_editCurveButton, &QPushButton::clicked,
            this, &MonitorConfigDialog::onEditCurveClicked);
    curveLayout->addWidget(m_editCurveButton);
    
    configLayout->addWidget(curveGroup);
    
    mainLayout->addWidget(m_configGroup);
    
    // 底部按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_applyButton = new QPushButton(tr("应用"), this);
    connect(m_applyButton, &QPushButton::clicked,
            this, &MonitorConfigDialog::onApplyClicked);
    buttonLayout->addWidget(m_applyButton);
    
    m_okButton = new QPushButton(tr("确定"), this);
    connect(m_okButton, &QPushButton::clicked,
            this, &MonitorConfigDialog::onOkClicked);
    buttonLayout->addWidget(m_okButton);
    
    m_cancelButton = new QPushButton(tr("取消"), this);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &MonitorConfigDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void MonitorConfigDialog::refreshMonitorList()
{
    m_monitorList->clear();
    m_autoBrightness->refreshMonitors();
    
    std::vector<MonitorInfo> monitors = m_autoBrightness->getMonitors();
    
    if (monitors.empty()) {
        QMessageBox::warning(this, tr("警告"), 
                           tr("未检测到显示器。请确保显示器已连接并支持亮度调节。"));
        return;
    }
    
    for (const MonitorInfo& monitor : monitors) {
        QString itemText = QString("%1 (索引: %2)")
                          .arg(monitor.friendlyName)
                          .arg(monitor.index);
        m_monitorList->addItem(itemText);
    }
    
    // 加载所有显示器的配置到临时存储
    for (const MonitorInfo& monitor : monitors) {
        m_tempConfigs[monitor.index] = m_autoBrightness->getMonitorConfig(monitor.index);
    }
    
    // 选中第一个显示器
    if (!monitors.empty()) {
        m_monitorList->setCurrentRow(0);
    }
}

void MonitorConfigDialog::onMonitorSelectionChanged()
{
    int currentRow = m_monitorList->currentRow();
    if (currentRow < 0) {
        m_configGroup->setEnabled(false);
        return;
    }
    
    // 保存当前配置
    if (m_currentMonitorIndex >= 0) {
        saveCurrentMonitorConfig();
    }
    
    // 加载新选中的显示器配置
    std::vector<MonitorInfo> monitors = m_autoBrightness->getMonitors();
    if (currentRow < static_cast<int>(monitors.size())) {
        m_currentMonitorIndex = monitors[currentRow].index;
        loadMonitorConfig(m_currentMonitorIndex);
        m_configGroup->setEnabled(true);
        
        // 更新当前亮度显示
        int currentBrightness = m_autoBrightness->getMonitorCurrentScreenBrightness(m_currentMonitorIndex);
        m_currentBrightnessLabel->setText(tr("当前屏幕亮度: %1%").arg(currentBrightness));
    }
}

void MonitorConfigDialog::loadMonitorConfig(int monitorIndex)
{
    auto it = m_tempConfigs.find(monitorIndex);
    if (it == m_tempConfigs.end()) {
        return;
    }
    
    const MonitorConfig& config = it->second;
    
    m_minCameraSpinBox->setValue(config.minCameraBrightness);
    m_minScreenSpinBox->setValue(config.minScreenBrightness);
    m_maxCameraSpinBox->setValue(config.maxCameraBrightness);
    m_maxScreenSpinBox->setValue(config.maxScreenBrightness);
    m_useCurveCheckBox->setChecked(config.useCurve);
    m_editCurveButton->setEnabled(config.useCurve);
}

void MonitorConfigDialog::saveCurrentMonitorConfig()
{
    if (m_currentMonitorIndex < 0) {
        return;
    }
    
    MonitorConfig& config = m_tempConfigs[m_currentMonitorIndex];
    config.minCameraBrightness = m_minCameraSpinBox->value();
    config.minScreenBrightness = m_minScreenSpinBox->value();
    config.maxCameraBrightness = m_maxCameraSpinBox->value();
    config.maxScreenBrightness = m_maxScreenSpinBox->value();
    config.useCurve = m_useCurveCheckBox->isChecked();
}

int MonitorConfigDialog::getCurrentMonitorIndex() const
{
    return m_currentMonitorIndex;
}

void MonitorConfigDialog::onRefreshMonitorsClicked()
{
    refreshMonitorList();
}

void MonitorConfigDialog::onSetMinBrightnessClicked()
{
    if (m_currentMonitorIndex < 0) {
        return;
    }
    
    int cameraBrightness = m_autoBrightness->getCurrentCameraBrightness();
    
    m_minCameraSpinBox->setValue(cameraBrightness);
    
    QMessageBox::information(this, tr("提示"), 
                           tr("已将当前摄像头亮度 %1% 设置为最小亮度参考点。\n"
                              "请调整对应的屏幕亮度值。")
                           .arg(cameraBrightness));
}

void MonitorConfigDialog::onSetMaxBrightnessClicked()
{
    if (m_currentMonitorIndex < 0) {
        return;
    }
    
    int cameraBrightness = m_autoBrightness->getCurrentCameraBrightness();
    
    m_maxCameraSpinBox->setValue(cameraBrightness);
    
    QMessageBox::information(this, tr("提示"), 
                           tr("已将当前摄像头亮度 %1% 设置为最大亮度参考点。\n"
                              "请调整对应的屏幕亮度值。")
                           .arg(cameraBrightness));
}

void MonitorConfigDialog::onUseCurveToggled(bool checked)
{
    m_editCurveButton->setEnabled(checked);
}

void MonitorConfigDialog::onEditCurveClicked()
{
    if (m_currentMonitorIndex < 0) {
        return;
    }
    
    // 保存当前配置
    saveCurrentMonitorConfig();
    
    CurveEditorDialog dialog(this);
    dialog.setCurvePoints(m_tempConfigs[m_currentMonitorIndex].curvePoints);
    
    if (dialog.exec() == QDialog::Accepted) {
        m_tempConfigs[m_currentMonitorIndex].curvePoints = dialog.getCurvePoints();
        QMessageBox::information(this, tr("提示"), tr("曲线已更新"));
    }
}

void MonitorConfigDialog::onApplyClicked()
{
    // 保存当前显示器配置
    if (m_currentMonitorIndex >= 0) {
        saveCurrentMonitorConfig();
    }
    
    // 应用所有临时配置到 AutoBrightness
    for (const auto& pair : m_tempConfigs) {
        int monitorIndex = pair.first;
        const MonitorConfig& config = pair.second;
        
        // 验证配置
        QString errorMsg;
        MonitorConfig tempConfig = config;
        m_autoBrightness->setMonitorMinBrightness(monitorIndex, 
                                                   tempConfig.minCameraBrightness, 
                                                   tempConfig.minScreenBrightness);
        m_autoBrightness->setMonitorMaxBrightness(monitorIndex, 
                                                   tempConfig.maxCameraBrightness, 
                                                   tempConfig.maxScreenBrightness);
        
        if (!m_autoBrightness->validateMonitorConfiguration(monitorIndex, &errorMsg)) {
            QMessageBox::warning(this, tr("配置错误"), errorMsg);
            // 回滚这个显示器的配置
            continue;
        }
        
        // 应用配置
        m_autoBrightness->setMonitorCurvePoints(monitorIndex, config.curvePoints);
        m_autoBrightness->setMonitorUseCurve(monitorIndex, config.useCurve);
    }
    
    // 保存设置
    m_autoBrightness->saveSettings();
    
    QMessageBox::information(this, tr("提示"), tr("配置已应用并保存"));
}

void MonitorConfigDialog::onOkClicked()
{
    onApplyClicked();
    accept();
}

void MonitorConfigDialog::onCancelClicked()
{
    reject();
}

#include "moc_MonitorConfigDialog.cpp"
