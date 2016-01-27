#include "imagewindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    imageWindow(new ImageWindow)
{
    ui->setupUi(this);

    imageWindow->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
