#include "palettebutton.h"
#include <QSizePolicy>
#include "imagewindow.h"
#include "drawtool.h"
#include "pentip.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    imageWindow(new ImageWindow),
    image(new QImage(320, 256, QImage::Format_Indexed8)),
    drawTool(new DrawTool(this)),
    penTip(new PenTip(this)),
    palette(new QVector<QRgb>)
{
    ui->setupUi(this);

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

    int row = ui->gridLayout->rowCount() + 2;
    for (int i = 0; i < palette->count(); i++) {
        PaletteButton *button = new PaletteButton();
        button->setColor(QColor(palette->at(i)));
        button->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        ui->gridLayout->addWidget(button, row + i / 2, i % 2);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete image;
}
