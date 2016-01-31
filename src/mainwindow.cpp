#include "palettebutton.h"
#include <QSizePolicy>
#include <QTimer>
#include "buffer.h"
#include "bufferview.h"
#include "drawtool.h"
#include "linetool.h"
#include "pentip.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bufferView(new BufferView),
    buffer(new Buffer(320, 256, 8, this)),
    penTip(new PenTip(this))
{
    ui->setupUi(this);

    ui->currentColorsButton->setPaintColor(QColor(buffer->color(1)));
    ui->currentColorsButton->setEraseColor(QColor(buffer->color(0)));
    penTip->setPaintColor(1);
    penTip->setEraseColor(0);
    buffer->setPen(penTip);
    bufferView->setBuffer(buffer);
    bufferView->show();

    static const int paletteButtonPerRow = 16;
    for (unsigned i = 0, row = 0, column = 0; i < buffer->colorCount(); i++) {
        PaletteButton *button = new PaletteButton();
        connect(button, SIGNAL(paintColorSelected(unsigned)), this, SLOT(setPaintColor(unsigned)));
        connect(button, SIGNAL(eraseColorSelected(unsigned)), this, SLOT(setEraseColor(unsigned)));
        button->setPaletteIndex(i);
        button->setColor(QColor(buffer->color(i)));
        button->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
        ui->paletteLayout->addWidget(button, row, column);
        column++;
        if (column >= paletteButtonPerRow) {
            column = 0;
            row++;
        }
    }

    QTimer::singleShot(1, this, SLOT(setupTools()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setPaintColor(unsigned paletteIndex)
{
    penTip->setPaintColor(paletteIndex);
    ui->currentColorsButton->setPaintColor(QColor(buffer->color(paletteIndex)));
}

void MainWindow::setEraseColor(unsigned paletteIndex)
{
    penTip->setEraseColor(paletteIndex);
    ui->currentColorsButton->setEraseColor(QColor(buffer->color(paletteIndex)));
}

void MainWindow::setupTools()
{
    for (int i = 0; i < tools.count(); i++) {
        tools.at(i)->addButtonToGridLayout(ui->toolsLayout);
        tools.at(i)->setBuffer(buffer);
    }

    buffer->setTool(tools.at(0));
}
