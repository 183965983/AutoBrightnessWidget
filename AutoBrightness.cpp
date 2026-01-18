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
int AutoBrightness::getBrightness(const cv::Mat& frame) {
    if (frame.empty() || m_samplePoints < 2) {
        return 0;
    }
    
    int rows = frame.rows;
    int cols = frame.cols;
    int rowsInterval = rows / m_samplePoints;
    int colsInterval = cols / m_samplePoints;
    
    if (rowsInterval == 0) rowsInterval = 1;
    if (colsInterval == 0) colsInterval = 1;
    
    int sum = 0;
    int count = 0;
    
    for(int i = 0; i < rows; i += rowsInterval){
        for(int j = 0; j < cols; j += colsInterval){
            cv::Vec3b pixel = frame.at<cv::Vec3b>(i, j);
            sum += pixel[2];  // 使用红色通道
            count++;
        }
    }
    
    if (count == 0) return 0;
    
    return sum / (count * 255 / 100);  // 返回0-100的百分比值
}


AutoBrightness::AutoBrightness(QObject* parent): QObject(parent)
    , m_cap(0)
    , m_exposure(-6.0)           // 默认曝光值
    , m_captureInterval(10000)   // 默认10秒间隔
    , m_samplePoints(5)          // 默认5x5采样点
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
    m_cap.open(0, cv::CAP_DSHOW);
    
    // 等待摄像头打开
    do {
        QThread::msleep(2000);
    } while (!m_cap.isOpened());
    
    // 设置摄像头参数
    m_cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // 0.25表示手动模式
    m_cap.set(cv::CAP_PROP_EXPOSURE, m_exposure); // 设置曝光值
    
    qDebug() << "Camera opened with exposure:" << m_exposure;
}

void AutoBrightness::releaseCap(){
    m_cap.release();
}

void AutoBrightness::update(){

    cv::Mat frame;
    m_cap >> frame; // 获取摄像头帧

    // 计算平均亮度并设置屏幕亮度

    int brightness = getBrightness(frame);

    setBrightness(brightness);
}

void AutoBrightness::setCamera(){
    m_cap.set(cv::CAP_PROP_SETTINGS, 1);
}

void AutoBrightness::setExposure(double exposure) {
    m_exposure = exposure;
    if (m_cap.isOpened()) {
        m_cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // 手动模式
        m_cap.set(cv::CAP_PROP_EXPOSURE, m_exposure);
        qDebug() << "Exposure updated to:" << m_exposure;
    }
}

void AutoBrightness::setCaptureInterval(int intervalMs) {
    m_captureInterval = intervalMs;
    qDebug() << "Capture interval updated to:" << m_captureInterval << "ms";
}

void AutoBrightness::setSamplePoints(int points) {
    if (points >= 2 && points <= 20) {
        m_samplePoints = points;
        qDebug() << "Sample points updated to:" << m_samplePoints << "x" << m_samplePoints;
    }
}

#include "moc_AutoBrightness.cpp"
