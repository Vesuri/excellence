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
    connect(ui->actionImageCopyColor, SIGNAL(triggered()), this, SLOT(imageCopyColor()));
    connect(ui->actionImageSwapColors, SIGNAL(triggered()), this, SLOT(imageSwapColors()));
    connect(ui->actionImageHistogram, SIGNAL(triggered()), this, SLOT(imageHistogram()));
    connect(ui->actionImageProperties, SIGNAL(triggered()), this, SLOT(showProperties()));
    connect(ui->actionPaletteCopyColor, SIGNAL(triggered()), this, SLOT(paletteCopyColor()));
    connect(ui->actionPaletteSwapColors, SIGNAL(triggered()), this, SLOT(paletteSwapColors()));
    connect(ui->actionPaletteSwapAndRemapColors, SIGNAL(triggered()), this, SLOT(paletteSwapAndRemapColors()));
    connect(ui->actionWindowNewWindow, SIGNAL(triggered()), this, SLOT(newWindow()));
    connect(ui->actionWindowCloseWindow, SIGNAL(triggered()), this, SLOT(closeWindow()));
    connect(propertiesDialog, SIGNAL(bufferChanged(Buffer *)), this, SLOT(setBuffer(Buffer *)));

    setAttribute(Qt::WA_Hover);
    installEventFilter(this);

    QTimer::singleShot(1, this, SLOT(initialize()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::runPaletteActionForPaintColor(unsigned paletteIndex)
{
    bool resetPaletteMode = true;

    switch (paletteMode) {
    case ImageCopy:
        buffer->copyImageColor(paintColor, paletteIndex);
        break;
    case ImageSwap:
        buffer->swapImageColors(paintColor, paletteIndex);
        break;
    case PaletteCopy:
        buffer->copyPaletteColor(paintColor, paletteIndex);
        break;
    case PaletteSwap:
        buffer->swapPaletteColors(paintColor, paletteIndex);
        break;
    case PaletteSwapAndRemap:
        buffer->swapImageColors(paintColor, paletteIndex);
        buffer->swapPaletteColors(paintColor, paletteIndex);
        break;
    default:
        setPaintColor(paletteIndex);
        resetPaletteMode = false;
        break;
    }

    if (resetPaletteMode) {
        paletteMode = Pick;
        updateWindowTitle();
    }
}

void MainWindow::runPaletteActionForEraseColor(unsigned paletteIndex)
{
    switch (paletteMode) {
    case Pick:
        setEraseColor(paletteIndex);
        break;
    default:
        paletteMode = Pick;
        updateWindowTitle();
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
        button->setAttribute(Qt::WA_Hover);
        button->installEventFilter(this);
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

void MainWindow::imageHistogram()
{
    QVector<unsigned> histogram(buffer->image().colorCount());

    for (int y = 0; y < buffer->image().height(); y++) {
        for (int x = 0; x < buffer->image().width(); x++) {
            histogram[buffer->image().pixelIndex(x, y)]++;
        }
    }

    for (int i = 0; i < histogram.count(); i++) {
        qWarning("%d #%06x %d", i, buffer->image().color(i) & 0xffffff, histogram[i]);
    }
}

void MainWindow::imageCopyColor()
{
    paletteMode = ImageCopy;
    updateWindowTitle();
}

void MainWindow::imageSwapColors()
{
    paletteMode = ImageSwap;
    updateWindowTitle();
}

void MainWindow::paletteCopyColor()
{
    paletteMode = PaletteCopy;
    updateWindowTitle();
}

void MainWindow::paletteSwapColors()
{
    paletteMode = PaletteSwap;
    updateWindowTitle();
}

void MainWindow::paletteSwapAndRemapColors()
{
    paletteMode = PaletteSwapAndRemap;
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

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowActivate:
        foreach (BufferView *bufferView, bufferViews) {
            if (watched == bufferView) {
                activeBufferView = bufferView;
            }
        }
        break;
    case QEvent::HoverEnter: {
        PaletteButton *paletteButton = qobject_cast<PaletteButton *>(watched);
        if (paletteButton) {
            updateWindowTitle(paletteButton->paletteIndex());
        }
        break;
    }
    case QEvent::HoverLeave:
        updateWindowTitle();
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void MainWindow::updateWindowTitle(int paletteIndex)
{
    QString windowTitle;

    switch (paletteMode) {
    case ImageCopy:
        windowTitle = "Excellence - Copy image color";
        break;
    case ImageSwap:
        windowTitle = "Excellence - Swap image colors";
        break;
    case PaletteCopy:
        windowTitle = "Excellence - Copy palette color";
        break;
    case PaletteSwap:
        windowTitle = "Excellence - Swap palette colors";
        break;
    case PaletteSwapAndRemap:
        windowTitle = "Excellence - Swap palette colors and remap";
        break;
    default:
        windowTitle = "Excellence";
        break;
    }

    if (paletteIndex >= 0) {
        windowTitle.append(paletteMode == Pick ? QString(" %1").arg(paletteIndex) : QString(" %1->%2").arg(paintColor).arg(paletteIndex));
    }

    setWindowTitle(windowTitle);
}
