#include "palettebutton.h"
#include <QSizePolicy>
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
    drawTool(new DrawTool(this)),
    lineTool(new LineTool(this)),
    tool(lineTool),
    penTip(new PenTip(this))
{
    ui->setupUi(this);

    ui->currentColorsButton->setPaintColor(QColor(buffer->palette()->at(1)));
    ui->currentColorsButton->setEraseColor(QColor(buffer->palette()->at(0)));
    penTip->setPaintColor(1);
    penTip->setEraseColor(0);
    drawTool->setDrawMode(DrawTool::ConnectedDraw);
    drawTool->setPen(penTip);
    lineTool->setPen(penTip);
    bufferView->setTool(tool);
    bufferView->setBuffer(buffer);
    bufferView->show();

    connect(ui->clearButton, SIGNAL(clicked(bool)), buffer, SLOT(clear()));

    static const int paletteButtonPerRow = 16;
    for (int i = 0, row = 0, column = 0; i < buffer->palette()->count(); i++) {
        PaletteButton *button = new PaletteButton();
        connect(button, SIGNAL(paintColorSelected(unsigned)), this, SLOT(setPaintColor(unsigned)));
        connect(button, SIGNAL(eraseColorSelected(unsigned)), this, SLOT(setEraseColor(unsigned)));
        button->setPaletteIndex(i);
        button->setColor(QColor(buffer->palette()->at(i)));
        button->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
        ui->paletteLayout->addWidget(button, row, column);
        column++;
        if (column >= paletteButtonPerRow) {
            column = 0;
            row++;
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setPaintColor(unsigned paletteIndex)
{
    penTip->setPaintColor(paletteIndex);
    ui->currentColorsButton->setPaintColor(QColor(buffer->palette()->at(paletteIndex)));
}

void MainWindow::setEraseColor(unsigned paletteIndex)
{
    penTip->setEraseColor(paletteIndex);
    ui->currentColorsButton->setEraseColor(QColor(buffer->palette()->at(paletteIndex)));
}
