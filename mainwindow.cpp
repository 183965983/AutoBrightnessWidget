#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QThread>
#include "AutoBrightness.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_autoBrightness(AutoBrightness::getInstance())
    , m_autoBrightThread(QThread::create([](){AutoBrightness::getInstance()->start();}))
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
    m_autoBrightThread->start();

}


void MainWindow::on_pushButton_2_clicked()
{
    //5m_autoBrightThread.
}

