#include "CurveEditorDialog.h"
#include "CurveEditorWidget.h"
#include "Ctrl/AutoBrightness.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QSpinBox>

CurveEditorDialog::CurveEditorDialog(QWidget *parent)
    : QDialog(parent)
    , m_curveEditor(nullptr)
    , m_presetComboBox(nullptr)
    , m_resetButton(nullptr)
    , m_setMinButton(nullptr)
    , m_setMaxButton(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("曲线编辑器");
    setMinimumSize(600, 550);
    setupUI();
}

CurveEditorDialog::~CurveEditorDialog()
{
}

void CurveEditorDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 说明文字
    QLabel* instructionLabel = new QLabel(
        "使用说明:\n"
        "• 左键点击空白处添加控制点\n"
        "• 拖拽控制点调整曲线形状\n"
        "• 右键点击控制点删除（端点除外）\n"
        "• 选择预设快速应用常用曲线", this);
    instructionLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }");
    mainLayout->addWidget(instructionLabel);
    
    // 曲线编辑器
    m_curveEditor = new CurveEditorWidget(this);
    mainLayout->addWidget(m_curveEditor);
    
    // 预设选择
    QGroupBox* presetGroup = new QGroupBox("预设曲线", this);
    QHBoxLayout* presetLayout = new QHBoxLayout(presetGroup);
    
    QLabel* presetLabel = new QLabel("选择预设:", presetGroup);
    presetLayout->addWidget(presetLabel);
    
    m_presetComboBox = new QComboBox(presetGroup);
    m_presetComboBox->addItem("线性（默认）");
    m_presetComboBox->addItem("S 曲线（增强对比度）");
    m_presetComboBox->addItem("提亮曲线");
    m_presetComboBox->addItem("压暗曲线");
    m_presetComboBox->addItem("对数曲线（快速响应暗环境）");
    connect(m_presetComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CurveEditorDialog::onPresetChanged);
    presetLayout->addWidget(m_presetComboBox);
    
    m_resetButton = new QPushButton("重置为线性", presetGroup);
    connect(m_resetButton, &QPushButton::clicked, this, &CurveEditorDialog::onResetClicked);
    presetLayout->addWidget(m_resetButton);
    
    presetLayout->addStretch();
    mainLayout->addWidget(presetGroup);
    
    // 自动标定区域
    QGroupBox* calibrationGroup = new QGroupBox("自动标定", this);
    QHBoxLayout* calibrationLayout = new QHBoxLayout(calibrationGroup);
    
    QLabel* calibrationLabel = new QLabel("根据当前环境光线设置曲线端点:", calibrationGroup);
    calibrationLayout->addWidget(calibrationLabel);
    
    m_setMinButton = new QPushButton("设置最小亮度", calibrationGroup);
    m_setMinButton->setToolTip("在最暗环境下点击，将当前摄像头亮度设为曲线起点");
    connect(m_setMinButton, &QPushButton::clicked, this, &CurveEditorDialog::onSetMinBrightnessClicked);
    calibrationLayout->addWidget(m_setMinButton);
    
    m_setMaxButton = new QPushButton("设置最大亮度", calibrationGroup);
    m_setMaxButton->setToolTip("在最亮环境下点击，将当前摄像头亮度设为曲线终点");
    connect(m_setMaxButton, &QPushButton::clicked, this, &CurveEditorDialog::onSetMaxBrightnessClicked);
    calibrationLayout->addWidget(m_setMaxButton);
    
    calibrationLayout->addStretch();
    mainLayout->addWidget(calibrationGroup);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton("确定", this);
    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &CurveEditorDialog::onOkClicked);
    buttonLayout->addWidget(m_okButton);
    
    m_cancelButton = new QPushButton("取消", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &CurveEditorDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void CurveEditorDialog::setCurvePoints(const std::vector<std::pair<int, int>>& points)
{
    m_originalPoints = points;
    if (m_curveEditor) {
        m_curveEditor->setCurvePoints(points);
    }
}

std::vector<std::pair<int, int>> CurveEditorDialog::getCurvePoints() const
{
    if (m_curveEditor) {
        return m_curveEditor->getCurvePoints();
    }
    return {};
}

void CurveEditorDialog::onPresetChanged(int index)
{
    if (m_curveEditor) {
        m_curveEditor->applyPreset(index);
    }
}

void CurveEditorDialog::onResetClicked()
{
    if (m_curveEditor) {
        m_curveEditor->resetToLinear();
    }
    m_presetComboBox->setCurrentIndex(0);
}

void CurveEditorDialog::onSetMinBrightnessClicked()
{
    int cameraBrightness = AutoBrightness::getInstance()->getCurrentCameraBrightness();
    
    // 弹出对话框让用户输入对应的屏幕亮度
    QDialog dialog(this);
    dialog.setWindowTitle("设置最小亮度");
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QLabel* infoLabel = new QLabel(
        QString("当前摄像头亮度: %1\n"
                "请设置对应的屏幕亮度（0-100）：").arg(cameraBrightness), &dialog);
    layout->addWidget(infoLabel);
    
    QSpinBox* spinBox = new QSpinBox(&dialog);
    spinBox->setRange(0, 100);
    spinBox->setValue(0);
    spinBox->setSuffix(" %");
    layout->addWidget(spinBox);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("确定", &dialog);
    QPushButton* cancelButton = new QPushButton("取消", &dialog);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    if (dialog.exec() == QDialog::Accepted) {
        int screenBrightness = spinBox->value();
        
        // 更新曲线的起点
        if (m_curveEditor) {
            auto points = m_curveEditor->getCurvePoints();
            if (points.size() >= 2) {
                points[0] = std::make_pair(cameraBrightness, screenBrightness);
                m_curveEditor->setCurvePoints(points);
            } else {
                // 如果曲线点少于2个，创建默认线性曲线
                points.clear();
                points.push_back(std::make_pair(cameraBrightness, screenBrightness));
                points.push_back(std::make_pair(100, 100));
                m_curveEditor->setCurvePoints(points);
            }
        }
        
        QMessageBox::information(this, "设置成功", 
            QString("最小亮度已设置:\n摄像头亮度: %1\n屏幕亮度: %2")
            .arg(cameraBrightness).arg(screenBrightness));
    }
}

void CurveEditorDialog::onSetMaxBrightnessClicked()
{
    int cameraBrightness = AutoBrightness::getInstance()->getCurrentCameraBrightness();
    
    // 弹出对话框让用户输入对应的屏幕亮度
    QDialog dialog(this);
    dialog.setWindowTitle("设置最大亮度");
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QLabel* infoLabel = new QLabel(
        QString("当前摄像头亮度: %1\n"
                "请设置对应的屏幕亮度（0-100）：").arg(cameraBrightness), &dialog);
    layout->addWidget(infoLabel);
    
    QSpinBox* spinBox = new QSpinBox(&dialog);
    spinBox->setRange(0, 100);
    spinBox->setValue(100);
    spinBox->setSuffix(" %");
    layout->addWidget(spinBox);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("确定", &dialog);
    QPushButton* cancelButton = new QPushButton("取消", &dialog);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    if (dialog.exec() == QDialog::Accepted) {
        int screenBrightness = spinBox->value();
        
        // 更新曲线的终点
        if (m_curveEditor) {
            auto points = m_curveEditor->getCurvePoints();
            if (points.size() >= 2) {
                points.back() = std::make_pair(cameraBrightness, screenBrightness);
                m_curveEditor->setCurvePoints(points);
            } else {
                // 如果曲线点少于2个，创建默认线性曲线
                points.clear();
                points.push_back(std::make_pair(0, 0));
                points.push_back(std::make_pair(cameraBrightness, screenBrightness));
                m_curveEditor->setCurvePoints(points);
            }
        }
        
        QMessageBox::information(this, "设置成功", 
            QString("最大亮度已设置:\n摄像头亮度: %1\n屏幕亮度: %2")
            .arg(cameraBrightness).arg(screenBrightness));
    }
}

void CurveEditorDialog::onOkClicked()
{
    accept();
}

void CurveEditorDialog::onCancelClicked()
{
    // 恢复原始曲线
    if (m_curveEditor) {
        m_curveEditor->setCurvePoints(m_originalPoints);
    }
    reject();
}
