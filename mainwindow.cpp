#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QThread>
#include "AutoBrightness.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_running(false)
    , m_autoBrightness(AutoBrightness::getInstance())
    , m_autoBrightThread(nullptr)
{


    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    m_running = false;
    if(m_autoBrightThread){
        m_autoBrightThread->wait();
        delete m_autoBrightThread;
        m_autoBrightThread = nullptr;
    }
    delete ui;

}

void MainWindow::on_pushButton_clicked()
{
    if(m_running){
        return; // Already running
    }
    
    m_running = true;
    m_autoBrightThread = QThread::create(
        [this](){
            AutoBrightness::getInstance()->openCap();
            while(m_running){
                AutoBrightness::getInstance()->update();
                QThread::msleep(AutoBrightness::getInstance()->getCaptureInterval());
            }
            AutoBrightness::getInstance()->releaseCap();
        });
    m_autoBrightThread->start();

}


void MainWindow::on_pushButton_2_clicked()
{
    m_running = false;
    if(m_autoBrightThread){
        m_autoBrightThread->wait();
        delete m_autoBrightThread;
        m_autoBrightThread = nullptr;
    }

}

void MainWindow::on_exposureSlider_valueChanged(int value)
{
    // 曝光值范围通常是 -13 到 -1，我们将滑块值(0-100)映射到这个范围
    double exposure = -13.0 + (value / 100.0) * 12.0;
    m_autoBrightness->setExposure(exposure);
    ui->exposureValueLabel->setText(QString::number(exposure, 'f', 1));
}

void MainWindow::on_intervalSlider_valueChanged(int value)
{
    // 间隔范围：1秒到60秒，滑块值直接对应秒数
    int intervalMs = value * 1000;
    m_autoBrightness->setCaptureInterval(intervalMs);
    ui->intervalValueLabel->setText(QString::number(value) + "s");
}

void MainWindow::on_samplePointsSlider_valueChanged(int value)
{
    // 采样点数范围：2到20
    m_autoBrightness->setSamplePoints(value);
    ui->samplePointsValueLabel->setText(QString::number(value) + "x" + QString::number(value));
}

