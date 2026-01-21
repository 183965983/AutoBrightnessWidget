#include <QTest>
#include <QApplication>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QDebug>
#include "../UI/mainwindow.h"
#include "../Ctrl/AutoBrightness.h"

class TestNewFeatures : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "Initializing new features test case...";
    }

    void test_brightness_display_widgets_exist() {
        // Test that brightness display widgets exist
        MainWindow window;
        window.show();
        
        QLabel* cameraBrightnessLabel = window.findChild<QLabel*>("cameraBrightnessLabel");
        QLabel* screenBrightnessLabel = window.findChild<QLabel*>("screenBrightnessLabel");
        QLabel* cameraBrightnessBlock = window.findChild<QLabel*>("cameraBrightnessBlock");
        
        QVERIFY(cameraBrightnessLabel != nullptr);
        QVERIFY(screenBrightnessLabel != nullptr);
        QVERIFY(cameraBrightnessBlock != nullptr);
        
        window.hide();
        qDebug() << "Brightness display widgets exist";
    }

    void test_config_buttons_exist() {
        // Test that configuration buttons exist
        MainWindow window;
        window.show();
        
        QPushButton* setMinButton = window.findChild<QPushButton*>("setMinBrightnessButton");
        QPushButton* setMaxButton = window.findChild<QPushButton*>("setMaxBrightnessButton");
        QSpinBox* minSpinBox = window.findChild<QSpinBox*>("minScreenBrightnessSpinBox");
        QSpinBox* maxSpinBox = window.findChild<QSpinBox*>("maxScreenBrightnessSpinBox");
        
        QVERIFY(setMinButton != nullptr);
        QVERIFY(setMaxButton != nullptr);
        QVERIFY(minSpinBox != nullptr);
        QVERIFY(maxSpinBox != nullptr);
        
        window.hide();
        qDebug() << "Configuration buttons and spinboxes exist";
    }

    void test_advanced_settings_checkboxes() {
        // Test that advanced settings checkboxes exist
        MainWindow window;
        window.show();
        
        QCheckBox* useCurveCheckBox = window.findChild<QCheckBox*>("useCurveCheckBox");
        QCheckBox* minimizeToTrayCheckBox = window.findChild<QCheckBox*>("minimizeToTrayCheckBox");
        QCheckBox* autoStartCheckBox = window.findChild<QCheckBox*>("autoStartCheckBox");
        
        QVERIFY(useCurveCheckBox != nullptr);
        QVERIFY(minimizeToTrayCheckBox != nullptr);
        QVERIFY(autoStartCheckBox != nullptr);
        
        window.hide();
        qDebug() << "Advanced settings checkboxes exist";
    }

    void test_brightness_configuration() {
        // Test brightness configuration methods
        AutoBrightness* ab = AutoBrightness::getInstance();
        
        // Set min brightness
        ab->setMinBrightness(20, 10);
        QCOMPARE(ab->getMinCameraBrightness(), 20);
        QCOMPARE(ab->getMinScreenBrightness(), 10);
        
        // Set max brightness
        ab->setMaxBrightness(80, 90);
        QCOMPARE(ab->getMaxCameraBrightness(), 80);
        QCOMPARE(ab->getMaxScreenBrightness(), 90);
        
        qDebug() << "Brightness configuration methods work";
    }

    void test_curve_configuration() {
        // Test curve configuration
        AutoBrightness* ab = AutoBrightness::getInstance();
        
        std::vector<std::pair<int, int>> curvePoints = {
            {0, 0},
            {50, 40},
            {100, 100}
        };
        
        ab->setCurvePoints(curvePoints);
        ab->setUseCurve(true);
        
        QVERIFY(ab->getUseCurve());
        QCOMPARE(ab->getCurvePoints().size(), curvePoints.size());
        
        qDebug() << "Curve configuration works";
    }

    void test_settings_save_load() {
        // Test settings persistence
        AutoBrightness* ab = AutoBrightness::getInstance();
        
        // Set some values
        ab->setMinBrightness(15, 5);
        ab->setMaxBrightness(85, 95);
        ab->setUseCurve(false);
        
        // Save settings
        ab->saveSettings();
        
        // Modify values
        ab->setMinBrightness(0, 0);
        ab->setMaxBrightness(100, 100);
        
        // Load settings
        ab->loadSettings();
        
        // Verify loaded values
        QCOMPARE(ab->getMinCameraBrightness(), 15);
        QCOMPARE(ab->getMinScreenBrightness(), 5);
        QCOMPARE(ab->getMaxCameraBrightness(), 85);
        QCOMPARE(ab->getMaxScreenBrightness(), 95);
        QVERIFY(!ab->getUseCurve());
        
        qDebug() << "Settings save/load works";
    }

    void test_brightness_validation() {
        // Test brightness configuration validation
        AutoBrightness* ab = AutoBrightness::getInstance();
        QString errorMessage;
        
        // Test valid configuration
        ab->setMinBrightness(20, 10);
        ab->setMaxBrightness(80, 90);
        QVERIFY(ab->validateBrightnessConfiguration(&errorMessage));
        
        // Test invalid: min camera >= max camera
        ab->setMinBrightness(80, 10);
        ab->setMaxBrightness(80, 90);
        QVERIFY(!ab->validateBrightnessConfiguration(&errorMessage));
        QVERIFY(errorMessage.contains("摄像头"));
        
        // Test invalid: min screen >= max screen
        ab->setMinBrightness(20, 90);
        ab->setMaxBrightness(80, 90);
        QVERIFY(!ab->validateBrightnessConfiguration(&errorMessage));
        QVERIFY(errorMessage.contains("屏幕"));
        
        // Test invalid: range too small
        ab->setMinBrightness(45, 10);
        ab->setMaxBrightness(50, 90);
        QVERIFY(!ab->validateBrightnessConfiguration(&errorMessage));
        QVERIFY(errorMessage.contains("范围过小"));
        
        // Test edge case: exactly 10 units should fail (< 10)
        ab->setMinBrightness(40, 10);
        ab->setMaxBrightness(49, 90);
        QVERIFY(!ab->validateBrightnessConfiguration(&errorMessage));
        
        // Test edge case: 10 units should pass
        ab->setMinBrightness(40, 10);
        ab->setMaxBrightness(50, 90);
        QVERIFY(ab->validateBrightnessConfiguration(&errorMessage));
        
        // Test clamping: values outside 0-100 should be clamped
        ab->setMinBrightness(-10, -20);
        QCOMPARE(ab->getMinCameraBrightness(), 0);
        QCOMPARE(ab->getMinScreenBrightness(), 0);
        
        ab->setMaxBrightness(120, 110);
        QCOMPARE(ab->getMaxCameraBrightness(), 100);
        QCOMPARE(ab->getMaxScreenBrightness(), 100);
        
        qDebug() << "Brightness validation works correctly";
    }

    void test_curve_editor_widget() {
        // Test curve editor widget functionality (UI-free tests)
        MainWindow window;
        window.show();
        
        // Verify edit curve button exists
        QPushButton* editCurveButton = window.findChild<QPushButton*>("editCurveButton");
        QVERIFY(editCurveButton != nullptr);
        
        window.hide();
        qDebug() << "Curve editor button exists";
    }

    void cleanupTestCase() {
        qDebug() << "New features test cleanup complete";
    }
};

// Test instance creation function for Qt test framework
QObject* createTestNewFeaturesInstance() {
    return new TestNewFeatures();
}

#include "test_new_features.moc"
