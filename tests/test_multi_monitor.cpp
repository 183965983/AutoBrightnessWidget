#include <QTest>
#include <QDebug>
#include "AutoBrightness.h"

class TestMultiMonitor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "Initializing multi-monitor test case...";
    }

    void test_monitor_info_structure() {
        // Test MonitorInfo structure
        MonitorInfo info("TestInstance", "Test Monitor", 0);
        
        QCOMPARE(info.instanceName, QString("TestInstance"));
        QCOMPARE(info.friendlyName, QString("Test Monitor"));
        QCOMPARE(info.index, 0);
        
        qDebug() << "MonitorInfo structure test passed";
    }

    void test_monitor_config_structure() {
        // Test MonitorConfig default values
        MonitorConfig config;
        
        QCOMPARE(config.minCameraBrightness, 0);
        QCOMPARE(config.maxCameraBrightness, 100);
        QCOMPARE(config.minScreenBrightness, 0);
        QCOMPARE(config.maxScreenBrightness, 100);
        QCOMPARE(config.useCurve, false);
        QVERIFY(config.curvePoints.empty());
        
        qDebug() << "MonitorConfig structure test passed";
    }

    void test_monitor_config_set_and_get() {
        // Test setting and getting monitor configurations
        AutoBrightness* ab = AutoBrightness::getInstance();
        
        // Since we might not have actual monitors in test environment,
        // we test the API without requiring actual monitor enumeration
        
        // Test monitor config getters return default values for non-existent monitors
        MonitorConfig config = ab->getMonitorConfig(0);
        QCOMPARE(config.minCameraBrightness, 0);
        QCOMPARE(config.maxCameraBrightness, 100);
        
        qDebug() << "Monitor config get/set API test passed";
    }

    void test_monitor_brightness_mapping() {
        // Test brightness mapping for monitors
        AutoBrightness* ab = AutoBrightness::getInstance();
        
        // Set up a test configuration
        ab->setMonitorMinBrightness(0, 20, 10);
        ab->setMonitorMaxBrightness(0, 80, 90);
        
        // Verify configuration was set
        MonitorConfig config = ab->getMonitorConfig(0);
        QCOMPARE(config.minCameraBrightness, 20);
        QCOMPARE(config.minScreenBrightness, 10);
        QCOMPARE(config.maxCameraBrightness, 80);
        QCOMPARE(config.maxScreenBrightness, 90);
        
        qDebug() << "Monitor brightness mapping test passed";
    }

    void test_monitor_curve_configuration() {
        // Test curve point configuration for monitors
        AutoBrightness* ab = AutoBrightness::getInstance();
        
        std::vector<std::pair<int, int>> testCurve;
        testCurve.push_back(std::make_pair(0, 10));
        testCurve.push_back(std::make_pair(50, 60));
        testCurve.push_back(std::make_pair(100, 100));
        
        ab->setMonitorCurvePoints(0, testCurve);
        ab->setMonitorUseCurve(0, true);
        
        MonitorConfig config = ab->getMonitorConfig(0);
        QCOMPARE(config.useCurve, true);
        QCOMPARE(static_cast<int>(config.curvePoints.size()), 3);
        QCOMPARE(config.curvePoints[0].first, 0);
        QCOMPARE(config.curvePoints[0].second, 10);
        QCOMPARE(config.curvePoints[1].first, 50);
        QCOMPARE(config.curvePoints[1].second, 60);
        
        qDebug() << "Monitor curve configuration test passed";
    }

    void test_monitor_validation() {
        // Test monitor configuration validation
        AutoBrightness* ab = AutoBrightness::getInstance();
        QString errorMsg;
        
        // Set invalid configuration (min >= max)
        ab->setMonitorMinBrightness(0, 80, 10);
        ab->setMonitorMaxBrightness(0, 20, 90);
        
        bool valid = ab->validateMonitorConfiguration(0, &errorMsg);
        QCOMPARE(valid, false);
        QVERIFY(!errorMsg.isEmpty());
        qDebug() << "Validation correctly failed:" << errorMsg;
        
        // Set valid configuration
        ab->setMonitorMinBrightness(0, 20, 10);
        ab->setMonitorMaxBrightness(0, 80, 90);
        
        valid = ab->validateMonitorConfiguration(0, &errorMsg);
        QCOMPARE(valid, true);
        
        qDebug() << "Monitor validation test passed";
    }

    void test_monitor_settings_persistence() {
        // Test saving and loading monitor configurations
        AutoBrightness* ab = AutoBrightness::getInstance();
        
        // Configure multiple monitors
        ab->setMonitorMinBrightness(0, 10, 5);
        ab->setMonitorMaxBrightness(0, 90, 95);
        ab->setMonitorUseCurve(0, true);
        
        ab->setMonitorMinBrightness(1, 15, 8);
        ab->setMonitorMaxBrightness(1, 85, 92);
        ab->setMonitorUseCurve(1, false);
        
        // Save settings
        ab->saveSettings();
        
        // Reset configurations
        ab->setMonitorMinBrightness(0, 0, 0);
        ab->setMonitorMaxBrightness(0, 100, 100);
        ab->setMonitorUseCurve(0, false);
        
        // Load settings
        ab->loadSettings();
        
        // Verify loaded configurations
        MonitorConfig config0 = ab->getMonitorConfig(0);
        QCOMPARE(config0.minCameraBrightness, 10);
        QCOMPARE(config0.minScreenBrightness, 5);
        QCOMPARE(config0.maxCameraBrightness, 90);
        QCOMPARE(config0.maxScreenBrightness, 95);
        QCOMPARE(config0.useCurve, true);
        
        MonitorConfig config1 = ab->getMonitorConfig(1);
        QCOMPARE(config1.minCameraBrightness, 15);
        QCOMPARE(config1.minScreenBrightness, 8);
        QCOMPARE(config1.maxCameraBrightness, 85);
        QCOMPARE(config1.maxScreenBrightness, 92);
        QCOMPARE(config1.useCurve, false);
        
        qDebug() << "Monitor settings persistence test passed";
    }

    void test_backward_compatibility() {
        // Test that legacy single-monitor API still works
        AutoBrightness* ab = AutoBrightness::getInstance();
        
        // Use legacy API
        ab->setMinBrightness(20, 15);
        ab->setMaxBrightness(80, 85);
        ab->setUseCurve(true);
        
        // Verify legacy getters work
        QCOMPARE(ab->getMinCameraBrightness(), 20);
        QCOMPARE(ab->getMinScreenBrightness(), 15);
        QCOMPARE(ab->getMaxCameraBrightness(), 80);
        QCOMPARE(ab->getMaxScreenBrightness(), 85);
        QCOMPARE(ab->getUseCurve(), true);
        
        qDebug() << "Backward compatibility test passed";
    }

    void cleanupTestCase() {
        qDebug() << "Multi-monitor test case cleanup complete";
    }
};

QTEST_MAIN(TestMultiMonitor)
#include "test_multi_monitor.moc"
