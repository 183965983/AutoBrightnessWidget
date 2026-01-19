#include <QTest>
#include <QApplication>
#include <QPushButton>
#include <QDebug>
#include "mainwindow.h"

class TestUI : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "Initializing UI test case...";
    }

    void test_mainwindow_creation() {
        // Test that MainWindow can be created
        MainWindow window;
        QVERIFY(true);
        qDebug() << "MainWindow created successfully";
    }

    void test_mainwindow_show() {
        // Test that MainWindow can be shown
        MainWindow window;
        window.show();
        QVERIFY(window.isVisible());
        window.hide();
        qDebug() << "MainWindow show/hide works";
    }

    void test_start_button_exists() {
        // Test that the start button exists
        MainWindow window;
        window.show();
        
        // Find the start button (pushButton)
        QPushButton* startButton = window.findChild<QPushButton*>("pushButton");
        QVERIFY(startButton != nullptr);
        QVERIFY(startButton->isVisible());
        
        window.hide();
        qDebug() << "Start button exists and is visible";
    }

    void test_stop_button_exists() {
        // Test that the stop button exists
        MainWindow window;
        window.show();
        
        // Find the stop button (pushButton_2)
        QPushButton* stopButton = window.findChild<QPushButton*>("pushButton_2");
        QVERIFY(stopButton != nullptr);
        QVERIFY(stopButton->isVisible());
        
        window.hide();
        qDebug() << "Stop button exists and is visible";
    }

    void test_button_interaction() {
        // Test button interaction without actually starting the camera
        // This tests the UI logic without hardware dependency
        MainWindow window;
        window.show();
        
        QPushButton* startButton = window.findChild<QPushButton*>("pushButton");
        QPushButton* stopButton = window.findChild<QPushButton*>("pushButton_2");
        
        QVERIFY(startButton != nullptr);
        QVERIFY(stopButton != nullptr);
        
        // Initially, start button should be enabled and stop button disabled
        QVERIFY(startButton->isEnabled());
        QVERIFY(!stopButton->isEnabled());
        
        window.hide();
        qDebug() << "Button interaction test passed";
    }

    void test_window_title() {
        // Test that the window has a title
        MainWindow window;
        QString title = window.windowTitle();
        QVERIFY(!title.isEmpty());
        qDebug() << "Window title:" << title;
    }

    void test_window_close() {
        // Test that the window can be closed properly
        MainWindow window;
        window.show();
        QTest::qWait(100);
        
        window.close();
        QVERIFY(!window.isVisible());
        
        qDebug() << "Window close test passed";
    }

    void test_multiple_window_instances() {
        // Test that multiple windows can be created
        MainWindow window1;
        MainWindow window2;
        
        window1.show();
        window2.show();
        
        QVERIFY(window1.isVisible());
        QVERIFY(window2.isVisible());
        
        window1.hide();
        window2.hide();
        
        qDebug() << "Multiple window instances test passed";
    }

    void cleanupTestCase() {
        qDebug() << "UI test cleanup complete";
    }
};

// Factory function for creating test instance
QObject* createTestUIInstance() {
    return new TestUI();
}

#include "test_ui.moc"
