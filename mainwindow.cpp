#include "imagewindow.h"
#include "drawtool.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    imageWindow(new ImageWindow),
    drawTool(new DrawTool(this))
{
    ui->setupUi(this);

    drawTool->setMode(DrawTool::ConnectedDraw);
    imageWindow->setTool(drawTool);
    imageWindow->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
