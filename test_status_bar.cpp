#include <QApplication>
#include <QLabel>
#include <QStatusBar>
#include <QPushButton>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    
    // 检查状态栏是否存在
    QStatusBar* statusBar = window.findChild<QStatusBar*>("statusbar");
    if (statusBar) {
        qDebug() << "✓ 状态栏存在";
        
        // 查找状态栏中的标签
        QList<QLabel*> labels = statusBar->findChildren<QLabel*>();
        qDebug() << "✓ 状态栏中有" << labels.size() << "个标签";
        
        for (int i = 0; i < labels.size(); i++) {
            qDebug() << "  标签" << i+1 << ":" << labels[i]->text();
        }
    }
    
    // 检查按钮状态
    QPushButton* startButton = window.findChild<QPushButton*>("pushButton");
    QPushButton* stopButton = window.findChild<QPushButton*>("pushButton_2");
    
    if (startButton && stopButton) {
        qDebug() << "\n按钮状态:";
        qDebug() << "✓ 启动按钮:";
        qDebug() << "  - 可用状态:" << (startButton->isEnabled() ? "启用" : "禁用");
        qDebug() << "  - 工具提示:" << startButton->toolTip();
        
        qDebug() << "✓ 停止按钮:";
        qDebug() << "  - 可用状态:" << (stopButton->isEnabled() ? "启用" : "禁用");
        qDebug() << "  - 工具提示:" << stopButton->toolTip();
    }
    
    qDebug() << "\n所有UI功能验证完成！";
    return 0;
}
