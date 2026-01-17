#include <QTest>
#include <QProcess>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "MockCamera.h"

// Helper class to test brightness calculation without using AutoBrightness directly
class BrightnessCalculator {
public:
    // Replicate the brightness calculation logic from AutoBrightness
    static int getBrightness(const cv::Mat& frame) {
        int rows = frame.rows;
        int cols = frame.cols;
        int rowsInterval = rows / 5;
        int colsInterval = cols / 5;
        int sum = 0;
        
        for(int i = 0; i < rows; i += rowsInterval) {
            for(int j = 0; j < cols; j += colsInterval) {
                cv::Vec3b pixel = frame.at<cv::Vec3b>(i, j);
                sum += pixel[2];  // Red channel
            }
        }
        
        return sum / (255 * 4);
    }
    
    // Get Windows brightness using PowerShell
    static int getWindowsBrightness() {
        QProcess process;
        QStringList arguments;
        arguments << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-WindowStyle" << "Hidden" 
                  << "-Command" << "(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightness).CurrentBrightness";
        
        process.start("powershell.exe", arguments);
        process.waitForFinished(5000);
        
        QString output = process.readAllStandardOutput().trimmed();
        bool ok;
        int brightness = output.toInt(&ok);
        
        if (!ok) {
            qWarning() << "Failed to parse brightness:" << output;
            return -1;
        }
        
        return brightness;
    }
    
    // Set Windows brightness using PowerShell
    static bool setWindowsBrightness(int brightness) {
        QString command = QString("(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1, %1)").arg(brightness);
        
        QStringList arguments;
        arguments << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-WindowStyle" << "Hidden" 
                  << "-Command" << command;
        
        QProcess process;
        process.start("powershell.exe", arguments);
        process.waitForFinished(5000);
        
        return process.exitCode() == 0;
    }
};

class TestBrightness : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        // Create test images if they don't exist
        qDebug() << "Initializing test case...";
        
        // Create dark image
        cv::Mat dark_img(480, 640, CV_8UC3, cv::Scalar(25, 25, 25));
        cv::imwrite("test_images/dark.jpg", dark_img);
        
        // Create medium image
        cv::Mat medium_img(480, 640, CV_8UC3, cv::Scalar(128, 128, 128));
        cv::imwrite("test_images/medium.jpg", medium_img);
        
        // Create bright image
        cv::Mat bright_img(480, 640, CV_8UC3, cv::Scalar(230, 230, 230));
        cv::imwrite("test_images/bright.jpg", bright_img);
        
        qDebug() << "Test images created";
    }

    void test_brightness_calculation_dark() {
        // Test brightness calculation with dark image (stubbed camera)
        MockCamera mockCamera;
        QVERIFY(mockCamera.loadTestImage("test_images/dark.jpg"));
        
        cv::Mat frame = mockCamera.getFrame();
        QVERIFY(!frame.empty());
        
        int brightness = BrightnessCalculator::getBrightness(frame);
        
        // Dark image should result in low brightness (around 0-10)
        QVERIFY(brightness >= 0);
        QVERIFY(brightness <= 20);
        
        qDebug() << "Dark image brightness:" << brightness;
    }

    void test_brightness_calculation_medium() {
        // Test brightness calculation with medium brightness image
        MockCamera mockCamera;
        QVERIFY(mockCamera.loadTestImage("test_images/medium.jpg"));
        
        cv::Mat frame = mockCamera.getFrame();
        QVERIFY(!frame.empty());
        
        int brightness = BrightnessCalculator::getBrightness(frame);
        
        // Medium image should result in medium brightness (around 40-60)
        QVERIFY(brightness >= 35);
        QVERIFY(brightness <= 65);
        
        qDebug() << "Medium image brightness:" << brightness;
    }

    void test_brightness_calculation_bright() {
        // Test brightness calculation with bright image
        MockCamera mockCamera;
        QVERIFY(mockCamera.loadTestImage("test_images/bright.jpg"));
        
        cv::Mat frame = mockCamera.getFrame();
        QVERIFY(!frame.empty());
        
        int brightness = BrightnessCalculator::getBrightness(frame);
        
        // Bright image should result in high brightness (around 80-100)
        QVERIFY(brightness >= 75);
        QVERIFY(brightness <= 100);
        
        qDebug() << "Bright image brightness:" << brightness;
    }

    void test_windows_brightness_api() {
        // Test Windows brightness adjustment API (no stubbing)
        // This test actually calls the Windows API and verifies it works
        
        qDebug() << "Testing Windows brightness API...";
        
        // Get current brightness first
        int originalBrightness = BrightnessCalculator::getWindowsBrightness();
        
        // Skip test if we can't read brightness (e.g., virtual machine without monitor support)
        if (originalBrightness < 0) {
            QSKIP("Unable to read Windows brightness - monitor may not support WMI brightness control");
        }
        
        qDebug() << "Original brightness:" << originalBrightness;
        
        // Test setting brightness to 50
        QVERIFY(BrightnessCalculator::setWindowsBrightness(50));
        QTest::qWait(500);  // Wait for brightness to stabilize
        
        int newBrightness = BrightnessCalculator::getWindowsBrightness();
        QVERIFY(newBrightness >= 0);
        QCOMPARE(newBrightness, 50);
        
        qDebug() << "Set brightness to 50, read back:" << newBrightness;
        
        // Test setting brightness to 80
        QVERIFY(BrightnessCalculator::setWindowsBrightness(80));
        QTest::qWait(500);
        
        newBrightness = BrightnessCalculator::getWindowsBrightness();
        QVERIFY(newBrightness >= 0);
        QCOMPARE(newBrightness, 80);
        
        qDebug() << "Set brightness to 80, read back:" << newBrightness;
        
        // Restore original brightness
        QVERIFY(BrightnessCalculator::setWindowsBrightness(originalBrightness));
        QTest::qWait(500);
        
        newBrightness = BrightnessCalculator::getWindowsBrightness();
        QCOMPARE(newBrightness, originalBrightness);
        
        qDebug() << "Restored original brightness:" << newBrightness;
    }

    void cleanupTestCase() {
        qDebug() << "Cleanup complete";
    }
};

// Factory function for creating test instance
QObject* createTestBrightnessInstance() {
    return new TestBrightness();
}

#include "test_brightness.moc"
