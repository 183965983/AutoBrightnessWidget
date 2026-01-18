#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QImage>
#include <QImageReader>
#include <QBuffer>
#include <QDebug>
#include <QThread>

#ifndef AUTOBRIGHTNESS_H
#define AUTOBRIGHTNESS_H


class AutoBrightness:public QObject{
    Q_OBJECT

public:
    static AutoBrightness* getInstance();
    void update();
    void openCap();
    void releaseCap();
    void setCamera();
    
    // 设置参数的方法
    void setExposure(double exposure);
    void setCaptureInterval(int intervalMs);
    void setSamplePoints(int points);
    
    // 获取参数的方法
    double getExposure() const { return m_exposure; }
    int getCaptureInterval() const { return m_captureInterval; }
    int getSamplePoints() const { return m_samplePoints; }

private:
    AutoBrightness(QObject *parent = nullptr);
    ~AutoBrightness();
    int getBrightness(const cv::Mat& frame);

private:
    void setBrightness(int);
    cv::VideoCapture m_cap;
    
    // 可配置参数
    double m_exposure;           // 曝光时间
    int m_captureInterval;       // 拍照间隔（毫秒）
    int m_samplePoints;          // 采样点数（每行/列）

};




#endif // AUTOBRIGHTNESS_H
