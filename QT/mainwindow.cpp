#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>  // for qApp
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Desktop Assistant -- a course homework project for SDUCS");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnPassword_clicked()
{
    // 启动密码管理器控制台程序
    QProcess::startDetached("cmd.exe", {"/c","start","password.exe"});
}

void MainWindow::on_btnReminder_clicked()
{
    QProcess::startDetached("cmd.exe", {"/c","start","reminder.exe"});
}

void MainWindow::on_btnLedger_clicked()
{
    QProcess::startDetached("cmd.exe", {"/c","start","ledger.exe"});
}



void MainWindow::on_btnExit_clicked()
{
    qApp->quit();       // 发送退出信号
}
