#include <iostream>
#include "AutoBrightness.h"
#include <QString>
#include <QProcess>


void AutoBrightness::setBrightness(int brightness) {
    std::string command = "(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1, ";
    command += std::to_string(brightness);
    command += ")";

    QStringList arguments;
    arguments << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-WindowStyle" << "Hidden" << "-Command" << command.c_str();

    QProcess process;
    qDebug()<<brightness;
    process.start("powershell.exe", arguments);
    process.waitForFinished();
}

// 获取图像平均亮度
double AutoBrightness::getAverageBrightness(const cv::Mat& frame) {
    cv::Mat grayFrame;
    cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
    cv::Scalar meanScalar = mean(grayFrame);
    return meanScalar[0];
}


AutoBrightness::AutoBrightness(QObject* parent): QObject(parent)
    ,m_cap(0)
{
    // 禁用自动曝光和自动增益


}

AutoBrightness::~AutoBrightness(){

}

AutoBrightness* AutoBrightness::getInstance(){
    static AutoBrightness instance;
    return &instance;
}

void AutoBrightness::openCap(){
    m_cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // 0.25表示手动模式
    m_cap.set(cv::CAP_PROP_EXPOSURE, -7); // 设置曝光值，具体值需要根据摄像头型号调整
    m_cap.set(cv::CAP_PROP_GAIN, 0); // 设置增益值
    m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 160);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 120);
    m_cap.set(cv::CAP_PROP_FPS, 1);
    m_cap.open(0,cv::CAP_DSHOW);
    qDebug()<<"CAP_PROP_FPS:"<<m_cap.get(cv::CAP_PROP_FPS);
    qDebug()<<"CAP_PROP_FRAME_WIDTH:"<<m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    qDebug()<<"CAP_PROP_FRAME_HEIGHT:"<<m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);


    do{QThread::msleep(2000);}
    while (!m_cap.isOpened());
}

void AutoBrightness::releaseCap(){
    m_cap.release();
}

void AutoBrightness::update(){

    cv::Mat frame;
    m_cap >> frame; // 获取摄像头帧

    // 计算平均亮度并设置屏幕亮度
    double brightness = getAverageBrightness(frame);
    int screenBrightness = static_cast<int>(brightness * 200 / 255); // 将亮度转换为0-100范围
    setBrightness(screenBrightness);
}

#include "moc_AutoBrightness.cpp"
