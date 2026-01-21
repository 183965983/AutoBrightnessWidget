#ifndef BRIGHTNESSCURVEWIDGET_H
#define BRIGHTNESSCURVEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPair>
#include <QDateTime>

/**
 * @brief 实时亮度曲线显示组件
 * 
 * 显示摄像头亮度和屏幕亮度随时间变化的曲线图
 */
class BrightnessCurveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BrightnessCurveWidget(QWidget *parent = nullptr);
    ~BrightnessCurveWidget() override = default;

    /**
     * @brief 添加一个亮度数据点
     * @param cameraBrightness 摄像头亮度 (0-100)
     * @param screenBrightness 屏幕亮度 (0-100)
     */
    void addDataPoint(int cameraBrightness, int screenBrightness);

    /**
     * @brief 清除所有数据点
     */
    void clearData();

    /**
     * @brief 设置最大显示数据点数量
     * @param maxPoints 最大数据点数（超过后删除旧数据）
     */
    void setMaxDataPoints(int maxPoints);

    /**
     * @brief 获取当前数据点数量
     */
    int getDataPointCount() const { return m_cameraData.size(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct DataPoint {
        qint64 timestamp;      // Unix 时间戳（毫秒）
        int cameraBrightness;  // 摄像头亮度
        int screenBrightness;  // 屏幕亮度
    };

    QVector<DataPoint> m_cameraData;  // 摄像头亮度历史数据
    int m_maxDataPoints;              // 最大数据点数量
    
    // 绘图辅助方法
    void drawGrid(QPainter &painter, const QRect &plotArea);
    void drawAxes(QPainter &painter, const QRect &plotArea);
    void drawCurve(QPainter &painter, const QRect &plotArea);
    void drawLegend(QPainter &painter);
};

#endif // BRIGHTNESSCURVEWIDGET_H
