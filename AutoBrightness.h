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
#include <map>
#include <QString>

#ifndef AUTOBRIGHTNESS_H
#define AUTOBRIGHTNESS_H

// 显示器信息结构
struct MonitorInfo {
    QString instanceName;    // WMI 实例名称
    QString friendlyName;    // 显示器友好名称
    int index;               // 显示器索引
    
    MonitorInfo() : index(0) {}
    MonitorInfo(const QString& name, const QString& friendly, int idx)
        : instanceName(name), friendlyName(friendly), index(idx) {}
};

// 显示器配置结构
struct MonitorConfig {
    int minCameraBrightness;
    int maxCameraBrightness;
    int minScreenBrightness;
    int maxScreenBrightness;
    bool useCurve;
    std::vector<std::pair<int, int>> curvePoints;
    
    MonitorConfig()
        : minCameraBrightness(0)
        , maxCameraBrightness(100)
        , minScreenBrightness(0)
        , maxScreenBrightness(100)
        , useCurve(false) {}
};

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
    void setGain(double gain);  // 新增：设置曝光增益
    
    // 获取参数的方法
    double getExposure() const { return m_exposure; }
    int getCaptureInterval() const { return m_captureInterval; }
    int getSamplePoints() const { return m_samplePoints; }
    double getGain() const { return m_gain; }  // 新增：获取曝光增益
    
    // 多显示器支持
    void refreshMonitors();
    std::vector<MonitorInfo> getMonitors() const { return m_monitors; }
    int getMonitorCount() const { return static_cast<int>(m_monitors.size()); }
    MonitorInfo getMonitorInfo(int index) const;
    
    // 显示器配置（传统单显示器接口，用于向后兼容）
    void setMinBrightness(int cameraBrightness, int screenBrightness);
    void setMaxBrightness(int cameraBrightness, int screenBrightness);
    int getMinCameraBrightness() const { return m_minCameraBrightness; }
    int getMaxCameraBrightness() const { return m_maxCameraBrightness; }
    int getMinScreenBrightness() const { return m_minScreenBrightness; }
    int getMaxScreenBrightness() const { return m_maxScreenBrightness; }
    
    // 多显示器配置接口
    void setMonitorMinBrightness(int monitorIndex, int cameraBrightness, int screenBrightness);
    void setMonitorMaxBrightness(int monitorIndex, int cameraBrightness, int screenBrightness);
    void setMonitorCurvePoints(int monitorIndex, const std::vector<std::pair<int, int>>& points);
    void setMonitorUseCurve(int monitorIndex, bool use);
    
    MonitorConfig getMonitorConfig(int monitorIndex) const;
    
    // 配置验证
    bool validateBrightnessConfiguration(QString* errorMessage = nullptr) const;
    bool validateMonitorConfiguration(int monitorIndex, QString* errorMessage = nullptr) const;
    
    // 曲线调整功能（传统单显示器接口）
    void setCurvePoints(const std::vector<std::pair<int, int>>& points);
    std::vector<std::pair<int, int>> getCurvePoints() const { return m_curvePoints; }
    void setUseCurve(bool use) { m_useCurve = use; }
    bool getUseCurve() const { return m_useCurve; }
    
    // 获取当前亮度值（用于显示）
    int getCurrentCameraBrightness() const { return m_currentCameraBrightness; }
    int getCurrentScreenBrightness() const { return m_currentScreenBrightness; }
    int getMonitorCurrentScreenBrightness(int monitorIndex) const;
    
    // 配置保存和加载
    void saveSettings();
    void loadSettings();

signals:
    void cameraBrightnessChanged(int brightness);
    void screenBrightnessChanged(int brightness);
    void monitorsChanged();  // 新增：显示器列表变化信号

private:
    AutoBrightness(QObject *parent = nullptr);
    ~AutoBrightness();
    int getBrightness(const cv::Mat& frame);
    int mapBrightness(int cameraBrightness);
    int interpolateCurve(int cameraBrightness);
    int mapBrightnessForMonitor(int monitorIndex, int cameraBrightness);
    int interpolateCurveForMonitor(int monitorIndex, int cameraBrightness);

private:
    void setBrightness(int);
    void setMonitorBrightness(int monitorIndex, int brightness);
    cv::VideoCapture m_cap;
    
    // 显示器列表
    std::vector<MonitorInfo> m_monitors;
    
    // 每个显示器的配置
    std::map<int, MonitorConfig> m_monitorConfigs;
    
    // 每个显示器的当前屏幕亮度
    std::map<int, int> m_monitorCurrentScreenBrightness;
    
    // 可配置参数
    double m_exposure;           // 曝光时间
    int m_captureInterval;       // 拍照间隔（毫秒）
    int m_samplePoints;          // 采样点数（每行/列）
    double m_gain;               // 曝光增益
    
    // 传统单显示器配置（向后兼容，应用到第一个显示器）
    int m_minCameraBrightness;
    int m_maxCameraBrightness;
    int m_minScreenBrightness;
    int m_maxScreenBrightness;
    
    // 曲线调整（传统单显示器）
    bool m_useCurve;
    std::vector<std::pair<int, int>> m_curvePoints;
    
    // 当前亮度值
    int m_currentCameraBrightness;
    int m_currentScreenBrightness;

};




#endif // AUTOBRIGHTNESS_H
