#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QThread>
#include "AutoBrightness.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_running(false)
    , m_autoBrightness(AutoBrightness::getInstance())

{


    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::on_pushButton_clicked()
{
    m_autoBrightThread = QThread::create(
        [&](){
            while(m_running){
                AutoBrightness::getInstance()->openCap();
                AutoBrightness::getInstance()->update();
                AutoBrightness::getInstance()->releaseCap();
                QThread::msleep(10000);
            }
        });
    m_running = true;
    m_autoBrightThread->start();

}


void MainWindow::on_pushButton_2_clicked()
{
    m_running = false;
    //m_autoBrightThread->wait();

}

