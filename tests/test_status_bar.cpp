#include <QTest>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QDebug>
#include "UI/mainwindow.h"

class TestStatusBar : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "测试状态栏功能...";
    }

    void test_statusbar_exists() {
        MainWindow window;
        QStatusBar* statusBar = window.findChild<QStatusBar*>("statusbar");
        QVERIFY(statusBar != nullptr);
        qDebug() << "✓ 状态栏存在";
    }

    void test_statusbar_labels() {
        MainWindow window;
        QStatusBar* statusBar = window.findChild<QStatusBar*>("statusbar");
        QVERIFY(statusBar != nullptr);
        
        QList<QLabel*> labels = statusBar->findChildren<QLabel*>();
        QVERIFY(labels.size() >= 2);
        qDebug() << "✓ 状态栏中有" << labels.size() << "个标签";
        
        for (int i = 0; i < labels.size(); i++) {
            qDebug() << "  标签" << i+1 << ":" << labels[i]->text();
        }
    }

    void test_initial_button_states() {
        MainWindow window;
        
        QPushButton* startButton = window.findChild<QPushButton*>("startButton");
        QPushButton* stopButton = window.findChild<QPushButton*>("stopButton");
        
        QVERIFY(startButton != nullptr);
        QVERIFY(stopButton != nullptr);
        
        // 初始状态：启动按钮启用，停止按钮禁用
        QVERIFY(startButton->isEnabled());
        QVERIFY(!stopButton->isEnabled());
        
        qDebug() << "✓ 启动按钮: " << (startButton->isEnabled() ? "启用" : "禁用");
        qDebug() << "  工具提示: " << startButton->toolTip();
        qDebug() << "✓ 停止按钮: " << (stopButton->isEnabled() ? "启用" : "禁用");
        qDebug() << "  工具提示: " << stopButton->toolTip();
    }

    void test_button_tooltips() {
        MainWindow window;
        
        QPushButton* startButton = window.findChild<QPushButton*>("startButton");
        QPushButton* stopButton = window.findChild<QPushButton*>("stopButton");
        
        QVERIFY(startButton != nullptr);
        QVERIFY(stopButton != nullptr);
        
        // 验证工具提示不为空
        QVERIFY(!startButton->toolTip().isEmpty());
        QVERIFY(!stopButton->toolTip().isEmpty());
        
        qDebug() << "✓ 按钮工具提示已设置";
    }

    void cleanupTestCase() {
        qDebug() << "状态栏测试完成！";
    }
};

// Factory function for creating test instance
QObject* createTestStatusBarInstance() {
    return new TestStatusBar();
}

#include "test_status_bar.moc"
