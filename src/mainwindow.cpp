#include "palettebutton.h"
#include <QSizePolicy>
#include <QTimer>
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>
#include "propertiesdialog.h"
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
    propertiesDialog(new PropertiesDialog),
    buffer(nullptr),
    penTip(new PenTip(this)),
    paletteMode(Pick),
    paintColor(1),
    eraseColor(0)
{
    ui->setupUi(this);

    connect(openDialog, SIGNAL(fileSelected(QString)), this, SLOT(openFile(QString)));
    connect(ui->actionFileQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionFileNew, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->actionFileOpen, SIGNAL(triggered()), openDialog, SLOT(show()));
    connect(ui->actionFileSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(ui->actionFileSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui->actionFileProperties, SIGNAL(triggered()), this, SLOT(showProperties()));
    connect(ui->actionPaletteCopyColor, SIGNAL(triggered()), this, SLOT(startCopyColor()));
    connect(ui->actionPaletteSwapColors, SIGNAL(triggered()), this, SLOT(startSwapColors()));
    connect(ui->actionWindowNewWindow, SIGNAL(triggered()), this, SLOT(newWindow()));
    connect(ui->actionWindowCloseWindow, SIGNAL(triggered()), this, SLOT(closeWindow()));
    connect(propertiesDialog, SIGNAL(bufferChanged(Buffer *)), this, SLOT(setBuffer(Buffer *)));

    QTimer::singleShot(1, this, SLOT(initialize()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::runPaletteActionForPaintColor(unsigned paletteIndex)
{
    switch (paletteMode) {
    case Pick:
        setPaintColor(paletteIndex);
        break;
    case Copy:
        copyColor(paletteIndex);
        break;
    case Swap:
        swapColors(paletteIndex);
        break;
    default:
        break;
    }
}

void MainWindow::runPaletteActionForEraseColor(unsigned paletteIndex)
{
    switch (paletteMode) {
    case Pick:
        setEraseColor(paletteIndex);
        break;
    case Copy:
        copyColor(paletteIndex);
        break;
    case Swap:
        swapColors(paletteIndex);
        break;
    default:
        break;
    }
}

void MainWindow::initialize()
{
    QStringList arguments = qApp->arguments();
    openFile(arguments.length() > 1 ? arguments.last() : QString());

    penTip->setPaintColor(paintColor);
    penTip->setEraseColor(eraseColor);

    newWindow();

    for (int i = 0; i < tools.count(); i++) {
        tools.at(i)->addButtonToGridLayout(ui->toolsLayout);
        tools.at(i)->setBuffer(buffer);
    }

    buffer->setTool(tools.at(0));
}

void MainWindow::setBuffer(Buffer *newBuffer)
{
    Buffer *oldBuffer = buffer;
    buffer = newBuffer;
    for (int i = 0; i < tools.count(); i++) {
        tools.at(i)->setBuffer(buffer);
    }

    buffer->setPen(penTip);
    buffer->setTool(tools.at(0));

    foreach (BufferView *bufferView, bufferViews) {
        bufferView->setBuffer(buffer);
    }

    while (ui->paletteLayout->count() > 0) {
        QLayoutItem *item = ui->paletteLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    static const int paletteButtonPerRow = 16;
    for (int i = 0, row = 0, column = 0; i < buffer->image().colorCount(); i++) {
        PaletteButton *button = new PaletteButton();
        connect(button, SIGNAL(paintColorSelected(unsigned)), this, SLOT(runPaletteActionForPaintColor(unsigned)));
        connect(button, SIGNAL(eraseColorSelected(unsigned)), this, SLOT(runPaletteActionForEraseColor(unsigned)));
        button->setPaletteIndex(static_cast<unsigned>(i));
        button->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
        ui->paletteLayout->addWidget(button, row, column);
        column++;
        if (column >= paletteButtonPerRow) {
            column = 0;
            row++;
        }
    }

    updatePalette();
    connect(buffer, SIGNAL(paletteModified()), this, SLOT(updatePalette()));

    delete oldBuffer;
}

void MainWindow::updatePalette()
{
    for (int i = 0; i < buffer->image().colorCount(); i++) {
        PaletteButton *button = static_cast<PaletteButton *>(ui->paletteLayout->itemAt(i)->widget());
        button->setColor(QColor(buffer->image().color(i)));
    }
    ui->currentColorsButton->setPaintColor(QColor(buffer->image().color(static_cast<int>(paintColor))));
    ui->currentColorsButton->setEraseColor(QColor(buffer->image().color(static_cast<int>(eraseColor))));
}

void MainWindow::openFile(const QString &path)
{
    setBuffer(new Buffer(path, this));
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
        QImageWriter imageWriter(path);
        if (!imageWriter.write(buffer->image())) {
            QMessageBox msgBox;
            msgBox.setText(imageWriter.errorString());
            msgBox.exec();
        }
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

void MainWindow::showProperties()
{
    propertiesDialog->setBuffer(buffer);
    propertiesDialog->show();
}

void MainWindow::startCopyColor()
{
    paletteMode = Copy;
    updateWindowTitle();
}

void MainWindow::startSwapColors()
{
    paletteMode = Swap;
    updateWindowTitle();
}

void MainWindow::setPaintColor(unsigned paletteIndex)
{
    paintColor = paletteIndex;

    penTip->setPaintColor(paintColor);
    ui->currentColorsButton->setPaintColor(QColor(buffer->image().color(static_cast<int>(paintColor))));
}

void MainWindow::setEraseColor(unsigned paletteIndex)
{
    eraseColor = paletteIndex;

    penTip->setEraseColor(eraseColor);
    ui->currentColorsButton->setEraseColor(QColor(buffer->image().color(static_cast<int>(eraseColor))));
}

void MainWindow::copyColor(unsigned paletteIndex)
{
    buffer->copyColor(paintColor, paletteIndex, ui->actionPaletteRemap->isChecked());

    paletteMode = Pick;
    updateWindowTitle();
}

void MainWindow::swapColors(unsigned paletteIndex)
{
    buffer->swapColors(paintColor, paletteIndex, ui->actionPaletteRemap->isChecked());

    paletteMode = Pick;
    updateWindowTitle();
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

void MainWindow::updateWindowTitle()
{
    QString windowTitle;

    switch (paletteMode) {
    case Copy:
        windowTitle = "Excellence - Copy color";
        break;
    case Swap:
        windowTitle = "Excellence - Swap colors";
        break;
    default:
        windowTitle = "Excellence";
        break;
    }

    setWindowTitle(windowTitle);
}
