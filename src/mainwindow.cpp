#include "palettebutton.h"
#include <QSizePolicy>
#include <QTimer>
#include <QFileDialog>
#include <QImageWriter>
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
    openDialog(new QFileDialog(nullptr, tr("Open file"))),
    buffer(nullptr),
    penTip(new PenTip(this))
{
    ui->setupUi(this);

    connect(openDialog, SIGNAL(fileSelected(QString)), this, SLOT(openFile(QString)));
    connect(ui->actionFileQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionFileNew, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->actionFileOpen, SIGNAL(triggered()), openDialog, SLOT(show()));
    connect(ui->actionFileSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(ui->actionFileSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui->actionWindowNewWindow, SIGNAL(triggered()), this, SLOT(newWindow()));
    connect(ui->actionWindowCloseWindow, SIGNAL(triggered()), this, SLOT(closeWindow()));

    QTimer::singleShot(1, this, SLOT(initialize()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setPaintColor(unsigned paletteIndex)
{
    penTip->setPaintColor(paletteIndex);
    ui->currentColorsButton->setPaintColor(QColor(buffer->image().color(static_cast<int>(paletteIndex))));
}

void MainWindow::setEraseColor(unsigned paletteIndex)
{
    penTip->setEraseColor(paletteIndex);
    ui->currentColorsButton->setEraseColor(QColor(buffer->image().color(static_cast<int>(paletteIndex))));
}

void MainWindow::initialize()
{
    openFile("");

    penTip->setPaintColor(1);
    penTip->setEraseColor(0);

    newWindow();

    for (int i = 0; i < tools.count(); i++) {
        tools.at(i)->addButtonToGridLayout(ui->toolsLayout);
        tools.at(i)->setBuffer(buffer);
    }

    buffer->setTool(tools.at(0));
}

void MainWindow::openFile(const QString &path)
{
    Buffer *oldBuffer = buffer;
    buffer = new Buffer(path, this);
    for (int i = 0; i < tools.count(); i++) {
        tools.at(i)->setBuffer(buffer);
    }

    buffer->setPen(penTip);
    buffer->setTool(tools.at(0));

    foreach (BufferView *bufferView, bufferViews) {
        bufferView->setBuffer(buffer);
    }

    ui->currentColorsButton->setPaintColor(QColor(buffer->image().color(1)));
    ui->currentColorsButton->setEraseColor(QColor(buffer->image().color(0)));

    while (ui->paletteLayout->count() > 0) {
        QLayoutItem *item = ui->paletteLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    static const int paletteButtonPerRow = 16;
    for (int i = 0, row = 0, column = 0; i < buffer->image().colorCount(); i++) {
        PaletteButton *button = new PaletteButton();
        connect(button, SIGNAL(paintColorSelected(unsigned)), this, SLOT(setPaintColor(unsigned)));
        connect(button, SIGNAL(eraseColorSelected(unsigned)), this, SLOT(setEraseColor(unsigned)));
        button->setPaletteIndex(static_cast<unsigned>(i));
        button->setColor(QColor(buffer->image().color(i)));
        button->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
        ui->paletteLayout->addWidget(button, row, column);
        column++;
        if (column >= paletteButtonPerRow) {
            column = 0;
            row++;
        }
    }

    delete oldBuffer;
}

void MainWindow::saveFile(const QString &savePath)
{
    if (!savePath.isEmpty()) {
        buffer->setPath(savePath);
    }

    QString path = buffer->path();
    if (path.isEmpty()) {
        saveAs();
    } else {
        QImageWriter imageWriter(path, "ilbm");
        imageWriter.write(buffer->image());
    }
}

void MainWindow::saveAs()
{
    QString path = QFileDialog::getSaveFileName(nullptr, tr("Open file"));

    if (!path.isEmpty()) {
        saveFile(path);
    }
}

void MainWindow::newWindow()
{
    BufferView *bufferView = new BufferView();
    bufferView->installEventFilter(this);
    bufferView->setBuffer(buffer);
    bufferView->show();
    bufferViews.append(bufferView);
}

void MainWindow::closeWindow()
{
    if (bufferViews.length() > 1) {
        if (activeBufferView) {
            bufferViews.removeAll(activeBufferView);
            delete activeBufferView;
            activeBufferView = nullptr;
        }
    } else {
        openFile();
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::WindowActivate) {
        foreach (BufferView *bufferView, bufferViews) {
            if (watched == bufferView) {
                activeBufferView = bufferView;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}
