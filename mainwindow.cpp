#include <QGraphicsScene>
#include "paletteitem.h"
#include "imagewindow.h"
#include "drawtool.h"
#include "pentip.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    paletteScene(new QGraphicsScene(this)),
    paletteItem(new PaletteItem),
    imageWindow(new ImageWindow),
    image(new QImage(320, 256, QImage::Format_Indexed8)),
    drawTool(new DrawTool(this)),
    penTip(new PenTip(this)),
    palette(new QVector<QRgb>)
{
    ui->setupUi(this);

    paletteItem->setPalette(palette);
    paletteScene->addItem(paletteItem);
    ui->paletteGraphicsView->setScene(paletteScene);

    palette->append(0xff959595);
    palette->append(0xff000000);
    palette->append(0xffffffff);
    palette->append(0xff3b67a2);
    palette->append(0xff7b7b7b);
    palette->append(0xffafafaf);
    palette->append(0xffaa907c);
    palette->append(0xffffa997);
    image->setColorTable(*palette);
    penTip->setColor(2);
    drawTool->setMode(DrawTool::ConnectedDraw);
    drawTool->setPen(penTip);
    imageWindow->setTool(drawTool);
    imageWindow->setImage(image);
    imageWindow->show();

    paletteItem->setSize(ui->paletteGraphicsView->size());
}

MainWindow::~MainWindow()
{
    delete ui;
    delete image;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    paletteItem->setSize(ui->paletteGraphicsView->size());
}
