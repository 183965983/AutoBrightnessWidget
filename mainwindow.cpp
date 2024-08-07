#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QThread>
#include "AutoBrightness.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_running(false)
    , m_autoBrightness(AutoBrightness::getInstance())
    , m_autoBrightThread(QThread::create(
            [&](){
                while(m_running){
                    AutoBrightness::getInstance()->openCap();
                    AutoBrightness::getInstance()->update();
                    AutoBrightness::getInstance()->releaseCap();
                }
          }))
{
    m_autoBrightThread->setPriority(QThread::HighestPriority);
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    m_running = true;
    m_autoBrightThread->start();

}


void MainWindow::on_pushButton_2_clicked()
{
    m_running = false;
    m_autoBrightThread->wait();

}

