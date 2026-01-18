#include "CurveEditorWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <algorithm>
#include <cmath>

CurveEditorWidget::CurveEditorWidget(QWidget *parent)
    : QWidget(parent)
    , m_selectedPointIndex(-1)
    , m_isDragging(false)
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
    
    // 初始化为线性曲线（两个端点）
    m_curvePoints = {{0, 0}, {100, 100}};
}

CurveEditorWidget::~CurveEditorWidget()
{
}

void CurveEditorWidget::setCurvePoints(const std::vector<std::pair<int, int>>& points)
{
    m_curvePoints = points;
    sortPoints();
    update();
    emit curveChanged();
}

std::vector<std::pair<int, int>> CurveEditorWidget::getCurvePoints() const
{
    return m_curvePoints;
}

void CurveEditorWidget::applyPreset(int presetIndex)
{
    switch (presetIndex) {
        case 0: // 线性
            m_curvePoints = {{0, 0}, {100, 100}};
            break;
        case 1: // S 曲线（增强对比度）
            m_curvePoints = {{0, 0}, {25, 10}, {50, 50}, {75, 90}, {100, 100}};
            break;
        case 2: // 提亮曲线
            m_curvePoints = {{0, 20}, {50, 60}, {100, 100}};
            break;
        case 3: // 压暗曲线
            m_curvePoints = {{0, 0}, {50, 40}, {100, 80}};
            break;
        case 4: // 对数曲线（快速响应暗环境）
            m_curvePoints = {{0, 0}, {20, 40}, {40, 60}, {70, 80}, {100, 100}};
            break;
        default:
            resetToLinear();
            return;
    }
    update();
    emit curveChanged();
}

void CurveEditorWidget::resetToLinear()
{
    m_curvePoints = {{0, 0}, {100, 100}};
    update();
    emit curveChanged();
}

void CurveEditorWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int w = width() - 2 * MARGIN;
    int h = height() - 2 * MARGIN;
    
    // 绘制背景
    painter.fillRect(rect(), Qt::white);
    
    // 绘制网格
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    for (int i = 0; i <= 4; ++i) {
        int x = MARGIN + i * w / 4;
        int y = MARGIN + i * h / 4;
        painter.drawLine(x, MARGIN, x, MARGIN + h);
        painter.drawLine(MARGIN, y, MARGIN + w, y);
    }
    
    // 绘制坐标轴
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(MARGIN, MARGIN, w, h);
    
    // 绘制轴标签
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    painter.drawText(MARGIN + w / 2 - 40, height() - 5, "摄像头亮度 →");
    painter.save();
    painter.translate(10, MARGIN + h / 2 + 40);
    painter.rotate(-90);
    painter.drawText(0, 0, "屏幕亮度 →");
    painter.restore();
    
    // 绘制刻度标签
    font.setPointSize(8);
    painter.setFont(font);
    for (int i = 0; i <= 4; ++i) {
        int value = i * 25;
        int x = MARGIN + i * w / 4;
        int y = MARGIN + (4 - i) * h / 4;
        painter.drawText(x - 10, height() - MARGIN + 15, QString::number(value));
        painter.drawText(5, y + 5, QString::number(value));
    }
    
    // 绘制曲线
    if (m_curvePoints.size() >= 2) {
        painter.setPen(QPen(QColor(0, 120, 215), 2));
        for (size_t i = 0; i < m_curvePoints.size() - 1; ++i) {
            QPoint p1 = valueToScreen(m_curvePoints[i].first, m_curvePoints[i].second);
            QPoint p2 = valueToScreen(m_curvePoints[i + 1].first, m_curvePoints[i + 1].second);
            painter.drawLine(p1, p2);
        }
    }
    
    // 绘制控制点
    for (size_t i = 0; i < m_curvePoints.size(); ++i) {
        QPoint p = valueToScreen(m_curvePoints[i].first, m_curvePoints[i].second);
        
        // 选中的点高亮显示
        if (static_cast<int>(i) == m_selectedPointIndex) {
            painter.setPen(QPen(QColor(255, 100, 0), 2));
            painter.setBrush(QBrush(QColor(255, 150, 50)));
        } else {
            painter.setPen(QPen(QColor(0, 120, 215), 2));
            painter.setBrush(QBrush(Qt::white));
        }
        
        painter.drawEllipse(p, POINT_RADIUS, POINT_RADIUS);
        
        // 显示点的坐标
        if (static_cast<int>(i) == m_selectedPointIndex) {
            painter.setPen(Qt::black);
            QString coord = QString("(%1, %2)")
                .arg(m_curvePoints[i].first)
                .arg(m_curvePoints[i].second);
            painter.drawText(p.x() + 10, p.y() - 10, coord);
        }
    }
}

void CurveEditorWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int index = findNearestPoint(event->pos());
        
        if (index >= 0) {
            // 选中现有点
            m_selectedPointIndex = index;
            m_isDragging = true;
        } else {
            // 添加新点
            auto value = screenToValue(event->pos());
            if (value.first >= 0 && value.first <= 100 && 
                value.second >= 0 && value.second <= 100) {
                addPoint(value.first, value.second);
            }
        }
        update();
    } else if (event->button() == Qt::RightButton) {
        // 右键删除点（除了端点）
        int index = findNearestPoint(event->pos());
        if (index > 0 && index < static_cast<int>(m_curvePoints.size()) - 1) {
            removePoint(index);
            update();
        }
    }
}

void CurveEditorWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && m_selectedPointIndex >= 0) {
        auto value = screenToValue(event->pos());
        
        // 限制在有效范围内
        value.first = std::max(0, std::min(100, value.first));
        value.second = std::max(0, std::min(100, value.second));
        
        // 端点的 x 坐标不能改变
        if (m_selectedPointIndex == 0) {
            value.first = 0;
        } else if (m_selectedPointIndex == static_cast<int>(m_curvePoints.size()) - 1) {
            value.first = 100;
        }
        
        m_curvePoints[m_selectedPointIndex] = value;
        sortPoints();
        
        // 重新查找选中点的索引（排序后可能改变）
        for (size_t i = 0; i < m_curvePoints.size(); ++i) {
            if (m_curvePoints[i] == value) {
                m_selectedPointIndex = static_cast<int>(i);
                break;
            }
        }
        
        update();
        emit curveChanged();
    }
}

void CurveEditorWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_isDragging = false;
}

std::pair<int, int> CurveEditorWidget::screenToValue(const QPoint& pos) const
{
    int w = width() - 2 * MARGIN;
    int h = height() - 2 * MARGIN;
    
    int x = std::round((pos.x() - MARGIN) * 100.0 / w);
    int y = std::round((MARGIN + h - pos.y()) * 100.0 / h);
    
    return {x, y};
}

QPoint CurveEditorWidget::valueToScreen(int x, int y) const
{
    int w = width() - 2 * MARGIN;
    int h = height() - 2 * MARGIN;
    
    int screenX = MARGIN + x * w / 100;
    int screenY = MARGIN + h - y * h / 100;
    
    return QPoint(screenX, screenY);
}

int CurveEditorWidget::findNearestPoint(const QPoint& pos) const
{
    int nearestIndex = -1;
    double minDistance = SELECTION_RADIUS;
    
    for (size_t i = 0; i < m_curvePoints.size(); ++i) {
        QPoint p = valueToScreen(m_curvePoints[i].first, m_curvePoints[i].second);
        double distance = std::sqrt(std::pow(p.x() - pos.x(), 2) + std::pow(p.y() - pos.y(), 2));
        
        if (distance < minDistance) {
            minDistance = distance;
            nearestIndex = static_cast<int>(i);
        }
    }
    
    return nearestIndex;
}

void CurveEditorWidget::addPoint(int x, int y)
{
    m_curvePoints.push_back({x, y});
    sortPoints();
    
    // 找到新添加点的索引并选中它
    for (size_t i = 0; i < m_curvePoints.size(); ++i) {
        if (m_curvePoints[i].first == x && m_curvePoints[i].second == y) {
            m_selectedPointIndex = static_cast<int>(i);
            break;
        }
    }
    
    emit curveChanged();
}

void CurveEditorWidget::removePoint(int index)
{
    if (index >= 0 && index < static_cast<int>(m_curvePoints.size())) {
        m_curvePoints.erase(m_curvePoints.begin() + index);
        m_selectedPointIndex = -1;
        emit curveChanged();
    }
}

void CurveEditorWidget::sortPoints()
{
    std::sort(m_curvePoints.begin(), m_curvePoints.end(),
              [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                  return a.first < b.first;
              });
}
