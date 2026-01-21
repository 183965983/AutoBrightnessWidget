#include <QTest>
#include <QApplication>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>
#include <QDebug>
#include "../UI/mainwindow.h"
#include "../Ctrl/AutoBrightness.h"
#include "../UI/BrightnessCurveWidget.h"

class TestRealTimeCurve : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "Initializing real-time curve test case...";
    }

    void test_gain_slider_exists() {
        // Test that gain slider and label exist
        MainWindow window;
        window.show();
        
        QSlider* gainSlider = window.findChild<QSlider*>("gainSlider");
        QLabel* gainLabel = window.findChild<QLabel*>("gainLabel");
        QLabel* gainValueLabel = window.findChild<QLabel*>("gainValueLabel");
        
        QVERIFY(gainSlider != nullptr);
        QVERIFY(gainLabel != nullptr);
        QVERIFY(gainValueLabel != nullptr);
        
        QCOMPARE(gainSlider->minimum(), 0);
        QCOMPARE(gainSlider->maximum(), 255);
        
        window.hide();
        qDebug() << "Gain slider exists and has correct range";
    }

    void test_sampling_freq_slider_exists() {
        // Test that sampling frequency slider exists
        MainWindow window;
        window.show();
        
        QSlider* samplingFreqSlider = window.findChild<QSlider*>("samplingFreqSlider");
        QLabel* samplingFreqLabel = window.findChild<QLabel*>("samplingFreqLabel");
        QLabel* samplingFreqValueLabel = window.findChild<QLabel*>("samplingFreqValueLabel");
        
        QVERIFY(samplingFreqSlider != nullptr);
        QVERIFY(samplingFreqLabel != nullptr);
        QVERIFY(samplingFreqValueLabel != nullptr);
        
        QCOMPARE(samplingFreqSlider->minimum(), 0);
        QCOMPARE(samplingFreqSlider->maximum(), 100);
        
        window.hide();
        qDebug() << "Sampling frequency slider exists and has correct range";
    }

    void test_curve_group_exists() {
        // Test that curve display group exists
        MainWindow window;
        window.show();
        
        QGroupBox* curveGroupBox = window.findChild<QGroupBox*>("curveGroupBox");
        QVERIFY(curveGroupBox != nullptr);
        
        window.hide();
        qDebug() << "Curve display group exists";
    }

    void test_brightness_curve_widget() {
        // Test BrightnessCurveWidget functionality
        BrightnessCurveWidget widget;
        
        // Test initial state
        QCOMPARE(widget.getDataPointCount(), 0);
        
        // Add some data points
        widget.addDataPoint(50, 60);
        QCOMPARE(widget.getDataPointCount(), 1);
        
        widget.addDataPoint(55, 65);
        widget.addDataPoint(60, 70);
        QCOMPARE(widget.getDataPointCount(), 3);
        
        // Test clearing data
        widget.clearData();
        QCOMPARE(widget.getDataPointCount(), 0);
        
        qDebug() << "BrightnessCurveWidget basic functionality works";
    }

    void test_brightness_curve_max_points() {
        // Test max data points limit
        BrightnessCurveWidget widget;
        widget.setMaxDataPoints(10);
        
        // Add more than max points
        for (int i = 0; i < 15; ++i) {
            widget.addDataPoint(i * 5, i * 6);
        }
        
        // Should only keep the last 10 points
        QCOMPARE(widget.getDataPointCount(), 10);
        
        qDebug() << "BrightnessCurveWidget max points limit works";
    }

    void test_autobrightness_gain() {
        // Test AutoBrightness gain control
        AutoBrightness* ab = AutoBrightness::getInstance();
        
        // Test setting and getting gain
        ab->setGain(128.0);
        QCOMPARE(ab->getGain(), 128.0);
        
        ab->setGain(255.0);
        QCOMPARE(ab->getGain(), 255.0);
        
        ab->setGain(0.0);
        QCOMPARE(ab->getGain(), 0.0);
        
        qDebug() << "AutoBrightness gain control works";
    }

    void cleanupTestCase() {
        qDebug() << "Cleaning up real-time curve test case...";
    }
};

QObject* createTestRealTimeCurveInstance() {
    return new TestRealTimeCurve();
}

#include "test_realtime_curve.moc"
