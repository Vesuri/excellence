#include "imagewindow.h"
#include "drawtool.h"
#include "pentip.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    imageWindow(new ImageWindow),
    drawTool(new DrawTool(this)),
    penTip(new PenTip(this))
{
    ui->setupUi(this);

    penTip->setColor(2);
    drawTool->setMode(DrawTool::ConnectedDraw);
    drawTool->setPen(penTip);
    imageWindow->setTool(drawTool);
    imageWindow->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
