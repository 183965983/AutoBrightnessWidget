#include "BrightnessCurveWidget.h"
#include <QPainter>
#include <QDateTime>
#include <algorithm>  // for std::max, std::min

BrightnessCurveWidget::BrightnessCurveWidget(QWidget *parent)
    : QWidget(parent)
    , m_maxDataPoints(100)  // 默认显示最近100个数据点
{
    setMinimumSize(400, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void BrightnessCurveWidget::addDataPoint(int cameraBrightness, int screenBrightness)
{
    DataPoint point;
    point.timestamp = QDateTime::currentMSecsSinceEpoch();
    point.cameraBrightness = qBound(0, cameraBrightness, 100);
    point.screenBrightness = qBound(0, screenBrightness, 100);
    
    m_cameraData.append(point);
    
    // 删除超过最大数量的旧数据
    while (m_cameraData.size() > m_maxDataPoints) {
        m_cameraData.removeFirst();
    }
    
    update();  // 触发重绘
}

void BrightnessCurveWidget::clearData()
{
    m_cameraData.clear();
    update();
}

void BrightnessCurveWidget::setMaxDataPoints(int maxPoints)
{
    m_maxDataPoints = std::max(10, maxPoints);
    
    // 如果当前数据超过新的最大值，删除旧数据
    while (m_cameraData.size() > m_maxDataPoints) {
        m_cameraData.removeFirst();
    }
    
    update();
}

void BrightnessCurveWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 背景
    painter.fillRect(rect(), Qt::white);
    
    // 计算绘图区域（留出边距用于坐标轴和标签）
    const int margin = 40;
    QRect plotArea = rect().adjusted(margin, margin, -margin, -margin);
    
    // 绘制网格
    drawGrid(painter, plotArea);
    
    // 绘制坐标轴
    drawAxes(painter, plotArea);
    
    // 绘制曲线
    if (!m_cameraData.isEmpty()) {
        drawCurve(painter, plotArea);
    }
    
    // 绘制图例
    drawLegend(painter);
}

void BrightnessCurveWidget::drawGrid(QPainter &painter, const QRect &plotArea)
{
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    
    // 绘制水平网格线（每20%一条线）
    for (int i = 0; i <= 5; ++i) {
        int y = plotArea.bottom() - (plotArea.height() * i / 5);
        painter.drawLine(plotArea.left(), y, plotArea.right(), y);
    }
    
    // 绘制垂直网格线（如果有足够数据点）
    if (m_cameraData.size() > 1) {
        int gridCount = std::min(10, static_cast<int>(m_cameraData.size() - 1));
        for (int i = 0; i <= gridCount; ++i) {
            int x = plotArea.left() + (plotArea.width() * i / gridCount);
            painter.drawLine(x, plotArea.top(), x, plotArea.bottom());
        }
    }
}

void BrightnessCurveWidget::drawAxes(QPainter &painter, const QRect &plotArea)
{
    painter.setPen(QPen(Qt::black, 2));
    
    // Y轴
    painter.drawLine(plotArea.bottomLeft(), plotArea.topLeft());
    
    // X轴
    painter.drawLine(plotArea.bottomLeft(), plotArea.bottomRight());
    
    // Y轴标签（亮度百分比）
    painter.setFont(QFont("Arial", 8));
    for (int i = 0; i <= 5; ++i) {
        int y = plotArea.bottom() - (plotArea.height() * i / 5);
        QString label = QString::number(i * 20) + "%";
        painter.drawText(QRect(0, y - 10, 35, 20), Qt::AlignRight | Qt::AlignVCenter, label);
    }
    
    // X轴标签（时间或样本数）
    if (!m_cameraData.isEmpty()) {
        painter.drawText(QRect(plotArea.left(), plotArea.bottom() + 5, 
                              plotArea.width() / 2, 30), 
                        Qt::AlignLeft | Qt::AlignTop, 
                        QString("最旧"));
        painter.drawText(QRect(plotArea.left() + plotArea.width() / 2, plotArea.bottom() + 5,
                              plotArea.width() / 2, 30),
                        Qt::AlignRight | Qt::AlignTop,
                        QString("最新"));
    }
}

void BrightnessCurveWidget::drawCurve(QPainter &painter, const QRect &plotArea)
{
    if (m_cameraData.size() < 2) {
        return;  // 至少需要2个点才能画线
    }
    
    // 准备绘制摄像头亮度曲线（蓝色）
    painter.setPen(QPen(QColor(0, 120, 215), 2));  // 蓝色线条
    
    const int dataSize = m_cameraData.size();
    Q_ASSERT(dataSize > 1);  // 已在函数开头对 size < 2 做过检查，这里不会发生除零
    const double widthScale = plotArea.width() / static_cast<double>(dataSize - 1);
    
    QVector<QPointF> cameraPoints;
    for (int i = 0; i < dataSize; ++i) {
        double x = plotArea.left() + i * widthScale;
        double y = plotArea.bottom() - (m_cameraData[i].cameraBrightness / 100.0) * plotArea.height();
        cameraPoints.append(QPointF(x, y));
    }
    
    // 绘制摄像头亮度曲线
    for (int i = 0; i < cameraPoints.size() - 1; ++i) {
        painter.drawLine(cameraPoints[i], cameraPoints[i + 1]);
    }
    
    // 准备绘制屏幕亮度曲线（橙色）
    painter.setPen(QPen(QColor(255, 140, 0), 2));  // 橙色线条
    
    QVector<QPointF> screenPoints;
    for (int i = 0; i < dataSize; ++i) {
        double x = plotArea.left() + i * widthScale;
        double y = plotArea.bottom() - (m_cameraData[i].screenBrightness / 100.0) * plotArea.height();
        screenPoints.append(QPointF(x, y));
    }
    
    // 绘制屏幕亮度曲线
    for (int i = 0; i < screenPoints.size() - 1; ++i) {
        painter.drawLine(screenPoints[i], screenPoints[i + 1]);
    }
}

void BrightnessCurveWidget::drawLegend(QPainter &painter)
{
    const int legendX = width() - 150;
    const int legendY = 10;
    const int lineLength = 30;
    const int lineSpacing = 20;
    
    painter.setFont(QFont("Arial", 9));
    
    // 摄像头亮度图例（蓝色）
    painter.setPen(QPen(QColor(0, 120, 215), 2));
    painter.drawLine(legendX, legendY + 5, legendX + lineLength, legendY + 5);
    painter.setPen(Qt::black);
    painter.drawText(legendX + lineLength + 5, legendY, 100, 15, 
                    Qt::AlignLeft | Qt::AlignVCenter, "摄像头亮度");
    
    // 屏幕亮度图例（橙色）
    painter.setPen(QPen(QColor(255, 140, 0), 2));
    painter.drawLine(legendX, legendY + lineSpacing + 5, 
                    legendX + lineLength, legendY + lineSpacing + 5);
    painter.setPen(Qt::black);
    painter.drawText(legendX + lineLength + 5, legendY + lineSpacing, 100, 15,
                    Qt::AlignLeft | Qt::AlignVCenter, "屏幕亮度");
}
