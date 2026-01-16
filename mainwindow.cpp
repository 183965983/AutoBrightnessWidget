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
    
    m_autoBrightThread = QThread::create(
        [&](){
            AutoBrightness::getInstance()->openCap();
            while(m_running){
                AutoBrightness::getInstance()->update();
                QThread::msleep(10000);
            }
            AutoBrightness::getInstance()->releaseCap();
        });
    m_running = true;
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

