#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QImage>
#include <QImageReader>
#include <QBuffer>
#include <QDebug>
#include <QThread>
#include <QSettings>
#include <vector>
#include <utility>

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
    
    // 最大/最小亮度配置
    void setMinBrightness(int cameraBrightness, int screenBrightness);
    void setMaxBrightness(int cameraBrightness, int screenBrightness);
    int getMinCameraBrightness() const { return m_minCameraBrightness; }
    int getMaxCameraBrightness() const { return m_maxCameraBrightness; }
    int getMinScreenBrightness() const { return m_minScreenBrightness; }
    int getMaxScreenBrightness() const { return m_maxScreenBrightness; }
    
    // 配置验证
    bool validateBrightnessConfiguration(QString* errorMessage = nullptr) const;
    
    // 曲线调整功能
    void setCurvePoints(const std::vector<std::pair<int, int>>& points);
    std::vector<std::pair<int, int>> getCurvePoints() const { return m_curvePoints; }
    void setUseCurve(bool use) { m_useCurve = use; }
    bool getUseCurve() const { return m_useCurve; }
    
    // 获取当前亮度值（用于显示）
    int getCurrentCameraBrightness() const { return m_currentCameraBrightness; }
    int getCurrentScreenBrightness() const { return m_currentScreenBrightness; }
    
    // 配置保存和加载
    void saveSettings();
    void loadSettings();

signals:
    void cameraBrightnessChanged(int brightness);
    void screenBrightnessChanged(int brightness);

private:
    AutoBrightness(QObject *parent = nullptr);
    ~AutoBrightness();
    int getBrightness(const cv::Mat& frame);
    int mapBrightness(int cameraBrightness);
    int interpolateCurve(int cameraBrightness);

private:
    void setBrightness(int);
    cv::VideoCapture m_cap;
    
    // 可配置参数
    double m_exposure;           // 曝光时间
    int m_captureInterval;       // 拍照间隔（毫秒）
    int m_samplePoints;          // 采样点数（每行/列）
    
    // 最大/最小亮度配置
    int m_minCameraBrightness;
    int m_maxCameraBrightness;
    int m_minScreenBrightness;
    int m_maxScreenBrightness;
    
    // 曲线调整
    bool m_useCurve;
    std::vector<std::pair<int, int>> m_curvePoints;
    
    // 当前亮度值
    int m_currentCameraBrightness;
    int m_currentScreenBrightness;

};




#endif // AUTOBRIGHTNESS_H
