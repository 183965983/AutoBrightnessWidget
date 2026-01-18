#ifndef CURVEEDITORDIALOG_H
#define CURVEEDITORDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <vector>
#include <utility>

class CurveEditorWidget;

class CurveEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CurveEditorDialog(QWidget *parent = nullptr);
    ~CurveEditorDialog();

    // 设置和获取曲线控制点
    void setCurvePoints(const std::vector<std::pair<int, int>>& points);
    std::vector<std::pair<int, int>> getCurvePoints() const;

private slots:
    void onPresetChanged(int index);
    void onResetClicked();
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();

private:
    CurveEditorWidget* m_curveEditor;
    QComboBox* m_presetComboBox;
    QPushButton* m_resetButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    std::vector<std::pair<int, int>> m_originalPoints;
};

#endif // CURVEEDITORDIALOG_H
