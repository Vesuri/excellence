#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include "imagewindow.h"
#include "ui_imagewindow.h"

ImageWindow::ImageWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageWindow),
    scene(new QGraphicsScene),
    image(new QImage(320, 256, QImage::Format_Indexed8))
{
    ui->setupUi(this);

    QVector<QRgb> colors;
    colors.append(0xff959595);
    colors.append(0xff000000);
    colors.append(0xffffffff);
    colors.append(0xff3b67a2);
    colors.append(0xff7b7b7b);
    colors.append(0xffafafaf);
    colors.append(0xffaa907c);
    colors.append(0xffffa997);
    image->setColorTable(colors);
    QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    scene->addItem(pixmapItem);
    ui->graphicsView->setScene(scene);
}

ImageWindow::~ImageWindow()
{
    delete ui;
    delete image;
}
