#include "CurveEditorDialog.h"
#include "CurveEditorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

CurveEditorDialog::CurveEditorDialog(QWidget *parent)
    : QDialog(parent)
    , m_curveEditor(nullptr)
    , m_presetComboBox(nullptr)
    , m_resetButton(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("曲线编辑器");
    setMinimumSize(600, 500);
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
