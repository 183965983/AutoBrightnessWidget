#ifndef CURVEEDITORWIDGET_H
#define CURVEEDITORWIDGET_H

#include <QWidget>
#include <vector>
#include <utility>

class CurveEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CurveEditorWidget(QWidget *parent = nullptr);
    ~CurveEditorWidget();

    // 设置和获取曲线控制点
    void setCurvePoints(const std::vector<std::pair<int, int>>& points);
    std::vector<std::pair<int, int>> getCurvePoints() const;
    
    // 应用预设曲线
    void applyPreset(int presetIndex);
    
    // 重置为线性曲线
    void resetToLinear();

signals:
    void curveChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // 将屏幕坐标转换为曲线坐标 (0-100)
    std::pair<int, int> screenToValue(const QPoint& pos) const;
    
    // 将曲线坐标转换为屏幕坐标
    QPoint valueToScreen(int x, int y) const;
    
    // 查找距离给定点最近的控制点索引
    int findNearestPoint(const QPoint& pos) const;
    
    // 添加新控制点
    void addPoint(int x, int y);
    
    // 删除控制点（双击）
    void removePoint(int index);
    
    // 对控制点按 x 坐标排序
    void sortPoints();

private:
    std::vector<std::pair<int, int>> m_curvePoints;
    int m_selectedPointIndex;  // -1 表示没有选中的点
    bool m_isDragging;
    
    // 绘制区域的边距
    static constexpr int MARGIN = 40;
    static constexpr int POINT_RADIUS = 6;
    static constexpr int SELECTION_RADIUS = 12;
};

#endif // CURVEEDITORWIDGET_H
