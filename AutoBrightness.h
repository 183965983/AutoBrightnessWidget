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

private:
    AutoBrightness(QObject *parent = nullptr);
    ~AutoBrightness();
    int getBrightness(const cv::Mat& frame);

private:
    void setBrightness(int);
    cv::VideoCapture m_cap;

};




#endif // AUTOBRIGHTNESS_H
