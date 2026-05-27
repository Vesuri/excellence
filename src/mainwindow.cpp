#include "colorutils.h"
#include "palettebutton.h"
#include <QAbstractSpinBox>
#include <QDockWidget>
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#include <QStatusBar>
#include "currentcolorsbutton.h"
#include "drawmodetool.h"
#include "gradientrange.h"
#include <QInputDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSizePolicy>
#include <QTimer>
#include <QFileDialog>
#include <QImageWriter>
#include <QImageReader>
#include <QMessageBox>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QClipboard>
#include "brush.h"
#include "propertiesdialog.h"
#include "buffer.h"
#include "bufferview.h"
#include "drawtool.h"
#include "linetool.h"
#include "pentip.h"
#include "tool.h"
#include "pickcolortool.h"
#include "mainwindow.h"
#include "plugins/raw/rawplugin.h"
#include "rawsaveoptionsdialog.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    openDialog(new QFileDialog(nullptr, tr("Open file"))),
    loadPaletteDialog(new QFileDialog(nullptr, tr("Load palette"))),
    savePaletteDialog(new QFileDialog(nullptr, tr("Save palette"))),
    propertiesDialog(new PropertiesDialog),
    buffer(nullptr),
    penTip(new PenTip(this)),
    toolPenTip(new PenTip(this)),
    paletteMode(Pick)
{
    ui->setupUi(this);

    // Status bar
    statusColorsButton_ = new CurrentColorsButton;
    statusToolLabel_ = new QLabel;
    statusToolLabel_->setToolTip(tr("Active tool"));
    statusModeLabel_ = new QLabel;
    statusModeLabel_->setToolTip(tr("Draw mode"));
    statusBrushLabel_ = new QLabel;
    statusCoordsLabel_ = new QLabel;

    auto makeSep = [](bool visible = true) {
        auto *f = new QFrame;
        f->setFrameShape(QFrame::VLine);
        f->setFrameShadow(QFrame::Sunken);
        f->setVisible(visible);
        return f;
    };
    auto *sep1 = makeSep();
    auto *sep2 = makeSep();
    auto *sep3 = makeSep();
    statusBrushSep_  = makeSep(false);
    statusCoordsSep_ = makeSep(false);

    statusBar()->setSizeGripEnabled(false);
    auto sbMargins = statusBar()->contentsMargins();
    sbMargins.setRight(8);
    statusBar()->setContentsMargins(sbMargins);
    statusBar()->addWidget(statusColorsButton_);
    statusBar()->addWidget(sep1);
    statusBar()->addWidget(statusToolLabel_);
    statusBar()->addWidget(sep2);
    statusBar()->addWidget(statusModeLabel_);
    statusBar()->addWidget(sep3);
    statusBar()->addWidget(statusBrushLabel_);
    statusBar()->addWidget(statusBrushSep_);
    statusBar()->addPermanentWidget(statusCoordsSep_);
    statusBar()->addPermanentWidget(statusCoordsLabel_);

    connect(statusColorsButton_, &CurrentColorsButton::foregroundClicked,
            this, &MainWindow::pickForegroundColor);
    connect(statusColorsButton_, &CurrentColorsButton::backgroundClicked,
            this, &MainWindow::pickBackgroundColor);

#ifdef Q_OS_MAC
    // Detach the menu bar so it becomes the application-level menu bar on
    // macOS, visible regardless of which Excellence window is currently active.
    menuBar()->setParent(nullptr);
#endif

    connect(openDialog, SIGNAL(fileSelected(QString)), this, SLOT(openFile(QString)));
    connect(loadPaletteDialog, SIGNAL(fileSelected(QString)), this, SLOT(loadPalette(QString)));
    savePaletteDialog->setAcceptMode(QFileDialog::AcceptSave);
    connect(savePaletteDialog, SIGNAL(fileSelected(QString)), this, SLOT(paletteSave(QString)));
    connect(ui->actionFileQuit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionFileNew, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->actionFileOpen, SIGNAL(triggered()), openDialog, SLOT(show()));
    connect(ui->actionFileSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(ui->actionFileSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui->actionFileSaveWithTransparency, &QAction::toggled, [this](bool checked) {
        saveWithTransparency_ = checked;
    });
    connect(ui->actionImageCopy, SIGNAL(triggered()), this, SLOT(imageCopy()));
    connect(ui->actionImagePaste, SIGNAL(triggered()), this, SLOT(imagePaste()));
    connect(ui->actionImageCopyColor, SIGNAL(triggered()), this, SLOT(imageCopyColor()));
    connect(ui->actionImageSwapColors, SIGNAL(triggered()), this, SLOT(imageSwapColors()));
    connect(ui->actionImageHistogram, SIGNAL(triggered()), this, SLOT(imageHistogram()));
    connect(ui->actionImageProperties, SIGNAL(triggered()), this, SLOT(showProperties()));
    connect(ui->actionPaletteLoad, SIGNAL(triggered()), loadPaletteDialog, SLOT(show()));
    connect(ui->actionPaletteSave, SIGNAL(triggered()), savePaletteDialog, SLOT(show()));
    connect(ui->actionPaletteSpread, SIGNAL(triggered()), this, SLOT(paletteSpread()));
    connect(ui->actionPaletteCopyColor, SIGNAL(triggered()), this, SLOT(paletteCopyColor()));
    connect(ui->actionPaletteSwapColors, SIGNAL(triggered()), this, SLOT(paletteSwapColors()));
    connect(ui->actionPaletteSwapAndRemapColors, SIGNAL(triggered()), this, SLOT(paletteSwapAndRemapColors()));
    connect(ui->actionPaletteDefault, SIGNAL(triggered()), this, SLOT(paletteDefault()));
    connect(ui->actionPaletteRestore, SIGNAL(triggered()), this, SLOT(paletteRestore()));
    connect(ui->actionPaletteUndo, SIGNAL(triggered()), this, SLOT(paletteUndo()));
    connect(ui->actionPaletteRemapPage, SIGNAL(triggered()), this, SLOT(paletteRemapPage()));
    connect(ui->actionBrushLoad, SIGNAL(triggered()), this, SLOT(brushLoad()));
    connect(ui->actionBrushSave, SIGNAL(triggered()), this, SLOT(brushSave()));
    connect(ui->actionBrushCopy, SIGNAL(triggered()), this, SLOT(brushCopy()));
    connect(ui->actionBrushPaste, SIGNAL(triggered()), this, SLOT(brushPaste()));
    connect(ui->actionBrushDelete, SIGNAL(triggered()), this, SLOT(brushDelete()));
    connect(ui->actionBrushRemap, SIGNAL(triggered()), this, SLOT(brushRemap()));
    connect(ui->actionBrushBgFgSwap, SIGNAL(triggered()), this, SLOT(brushBgFgSwap()));
    connect(ui->actionBrushAutoBackground, SIGNAL(triggered()), this, SLOT(brushAutoBackground()));
    connect(ui->actionBrushFlipHorizontal, SIGNAL(triggered()), this, SLOT(brushFlipHorizontal()));
    connect(ui->actionBrushFlipVertical, SIGNAL(triggered()), this, SLOT(brushFlipVertical()));
    connect(ui->actionBrushRotate90CW, SIGNAL(triggered()), this, SLOT(brushRotate90CW()));
    connect(ui->actionBrushRotate90CCW, SIGNAL(triggered()), this, SLOT(brushRotate90CCW()));
    connect(ui->actionBrushRotateNumeric, SIGNAL(triggered()), this, SLOT(brushRotateNumeric()));
    connect(ui->actionBrushDouble, SIGNAL(triggered()), this, SLOT(brushDouble()));
    connect(ui->actionBrushDoubleX, SIGNAL(triggered()), this, SLOT(brushDoubleX()));
    connect(ui->actionBrushDoubleY, SIGNAL(triggered()), this, SLOT(brushDoubleY()));
    connect(ui->actionBrushHalve, SIGNAL(triggered()), this, SLOT(brushHalve()));
    connect(ui->actionBrushHalveX, SIGNAL(triggered()), this, SLOT(brushHalveX()));
    connect(ui->actionBrushHalveY, SIGNAL(triggered()), this, SLOT(brushHalveY()));
    connect(ui->actionBrushScaleToSize, SIGNAL(triggered()), this, SLOT(brushScaleToSize()));
    connect(ui->actionBrushShearX, SIGNAL(triggered()), this, SLOT(brushShearX()));
    connect(ui->actionBrushShearY, SIGNAL(triggered()), this, SLOT(brushShearY()));
    connect(ui->actionBrushBendX, SIGNAL(triggered()), this, SLOT(brushBendX()));
    connect(ui->actionBrushBendY, SIGNAL(triggered()), this, SLOT(brushBendY()));
    connect(ui->actionBrushOutline, SIGNAL(triggered()), this, SLOT(brushOutline()));
    connect(ui->actionBrushTrim, SIGNAL(triggered()), this, SLOT(brushTrim()));
    connect(ui->actionBrushTileCut, SIGNAL(triggered()), this, SLOT(brushTileCut()));
    connect(ui->actionBrushRestore, SIGNAL(triggered()), this, SLOT(brushRestore()));
    connect(ui->actionWindowNewWindow, SIGNAL(triggered()), this, SLOT(newWindow()));
    connect(ui->actionWindowCloseWindow, SIGNAL(triggered()), this, SLOT(closeWindow()));
    connect(ui->actionWindowFloatPanels, &QAction::toggled, this, &MainWindow::toggleFloatPanels);
    connect(ui->actionWindowSingleWindow, &QAction::toggled, this, &MainWindow::toggleSingleWindowMode);
    connect(ui->actionHelpAbout, &QAction::triggered, this, &MainWindow::about);
    connect(propertiesDialog, SIGNAL(bufferChanged(Buffer *)), this, SLOT(setBuffer(Buffer *)));
    connect(penTip, &PenTip::sizeChanged, this, [this](int, int) { updateStatusBarStatic(); });

    setAttribute(Qt::WA_Hover);
    installEventFilter(this);
    qApp->installEventFilter(this);

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
        buffer->copyImageColor(buffer->paintColor(), paletteIndex);
        break;
    case ImageSwap:
        buffer->swapImageColors(buffer->paintColor(), paletteIndex);
        break;
    case PaletteCopy:
        buffer->copyPaletteColor(buffer->paintColor(), paletteIndex);
        break;
    case PaletteSwap:
        buffer->swapPaletteColors(buffer->paintColor(), paletteIndex);
        break;
    case PaletteSwapAndRemap:
        buffer->swapImageColors(buffer->paintColor(), paletteIndex);
        buffer->swapPaletteColors(buffer->paintColor(), paletteIndex);
        break;
    case PaletteSpread: {
        int from = static_cast<int>(buffer->paintColor());
        int to   = static_cast<int>(paletteIndex);
        if (from != to) {
            if (from > to)
                qSwap(from, to);
            paletteUndoSnapshot_ = buffer->image().colorTable();
            QRgb cFrom = buffer->image().color(from);
            QRgb cTo   = buffer->image().color(to);
            int steps  = to - from;
            for (int i = 1; i < steps; i++) {
                int r = qRed(cFrom)   + (qRed(cTo)   - qRed(cFrom))   * i / steps;
                int g = qGreen(cFrom) + (qGreen(cTo) - qGreen(cFrom)) * i / steps;
                int b = qBlue(cFrom)  + (qBlue(cTo)  - qBlue(cFrom))  * i / steps;
                buffer->setColor(static_cast<unsigned>(from + i), QColor(r, g, b));
            }
        }
        break;
    }
    default:
        buffer->setPaintColor(paletteIndex);
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
        buffer->setEraseColor(paletteIndex);
        break;
    default:
        paletteMode = Pick;
        updateWindowTitle();
        break;
    }
}

void MainWindow::initialize()
{
    Tool::setMainWindow(this);

    QStringList arguments = qApp->arguments();
    openFile(arguments.length() > 1 ? arguments.last() : QString());

    penTip->setPaintColor(buffer->paintColor());
    penTip->setEraseColor(buffer->eraseColor());
    toolPenTip->setPaintColor(buffer->paintColor());
    toolPenTip->setEraseColor(buffer->eraseColor());

    newWindow();
    activateWindow();
    raise();

    for (int i = 0; i < tools.count(); i++) {
        tools.at(i)->addButtonToGridLayout(ui->toolsLayout);
        tools.at(i)->setBuffer(buffer);
    }


    for (Tool *tool : tools) {
        if (qobject_cast<DrawTool *>(tool)) { buffer->setTool(tool); break; }
    }

    QTimer::singleShot(0, this, [this]() {
        centralWidget()->setMinimumHeight(centralWidget()->height());
    });
}

void MainWindow::setBuffer(Buffer *newBuffer)
{
    Buffer *oldBuffer = buffer;
    buffer = newBuffer;
    for (int i = 0; i < tools.count(); i++) {
        tools.at(i)->setBuffer(buffer);
    }

    buffer->setPen(penTip);
    buffer->setToolPen(toolPenTip);
    for (Tool *tool : tools) {
        if (qobject_cast<DrawTool *>(tool)) { buffer->setTool(tool); break; }
    }

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
    connect(buffer, SIGNAL(paintColorChanged(unsigned,QColor)), this, SLOT(updatePalette()));
    connect(buffer, SIGNAL(eraseColorChanged(unsigned,QColor)), this, SLOT(updatePalette()));
    paletteRestorePoint_ = buffer->image().colorTable();
    paletteUndoSnapshot_ = paletteRestorePoint_;
    connect(buffer, SIGNAL(paintColorChanged(unsigned, QColor)), penTip, SLOT(setPaintColor(unsigned)));
    connect(buffer, SIGNAL(eraseColorChanged(unsigned, QColor)), penTip, SLOT(setEraseColor(unsigned)));
    connect(buffer, SIGNAL(paintColorChanged(unsigned, QColor)), toolPenTip, SLOT(setPaintColor(unsigned)));
    connect(buffer, SIGNAL(eraseColorChanged(unsigned, QColor)), toolPenTip, SLOT(setEraseColor(unsigned)));
    connect(buffer, &Buffer::dirtyChanged, this, &MainWindow::onDirtyChanged);
    connect(buffer, SIGNAL(toolChanged(Tool*)), this, SLOT(updateStatusBarStatic()));
    connect(buffer, &Buffer::penChanged, this, [this](Pen *) { updateStatusBarStatic(); });
    connect(buffer, &Buffer::paintModeChanged, this, [this](Buffer::PaintMode) { updateStatusBarStatic(); });
    connect(buffer, SIGNAL(paintColorChanged(unsigned,QColor)), this, SLOT(updateStatusBarStatic()));
    connect(buffer, SIGNAL(eraseColorChanged(unsigned,QColor)), this, SLOT(updateStatusBarStatic()));
    connect(buffer, SIGNAL(paletteModified()), this, SLOT(updateStatusBarStatic()));
    connect(buffer, SIGNAL(paintColorChanged(unsigned,QColor)), statusColorsButton_, SLOT(setPaintColor(unsigned,QColor)));
    connect(buffer, SIGNAL(eraseColorChanged(unsigned,QColor)), statusColorsButton_, SLOT(setEraseColor(unsigned,QColor)));
    updateStatusBarStatic();

    delete oldBuffer;
}

void MainWindow::updatePalette()
{
    unsigned paintIdx = buffer->paintColor();
    unsigned eraseIdx = buffer->eraseColor();
    for (int i = 0; i < buffer->image().colorCount(); i++) {
        PaletteButton *button = static_cast<PaletteButton *>(ui->paletteLayout->itemAt(i)->widget());
        button->setColor(QColor(buffer->image().color(i)));
        button->setIsPaintColor(static_cast<unsigned>(i) == paintIdx);
        button->setIsEraseColor(static_cast<unsigned>(i) == eraseIdx);
    }
}

void MainWindow::openFile(const QString &path)
{
    setBuffer(new Buffer(path, this));
    openDialog->setDirectory(path);
}

void MainWindow::saveFile(const QString &savePath, const RawSaveOptions &rawOptions)
{
    if (!savePath.isEmpty()) {
        buffer->setPath(savePath);
    }

    QString path = buffer->path();
    if (path.isEmpty()) {
        saveAs();
    } else {
        QImage saveImage = buffer->image();
        if (saveWithTransparency_ && path.toLower().endsWith(".png")) {
            int eraseIdx = static_cast<int>(buffer->eraseColor());
            QRgb c = saveImage.color(eraseIdx);
            saveImage.setColor(eraseIdx, qRgba(qRed(c), qGreen(c), qBlue(c), 0));
        }
        bool writeOk = false;
        QString errorString;
        if (path.toLower().endsWith(".raw")) {
            QFile file(path);
            if (!file.open(QIODevice::WriteOnly)) {
                errorString = file.errorString();
            } else {
                RawHandler handler;
                handler.setDevice(&file);
                handler.setOption(static_cast<QImageIOHandler::ImageOption>(RawOption::Interleave),       rawOptions.interleave);
                handler.setOption(static_cast<QImageIOHandler::ImageOption>(RawOption::WordAlign),        rawOptions.wordAlign);
                handler.setOption(static_cast<QImageIOHandler::ImageOption>(RawOption::PaletteDepth),     rawOptions.paletteDepth);
                handler.setOption(static_cast<QImageIOHandler::ImageOption>(RawOption::PalettePlacement), rawOptions.palettePlacement);
                writeOk = handler.write(saveImage);
                if (!writeOk)
                    errorString = tr("Failed to write raw file.");
            }
        } else {
            QImageWriter imageWriter(path);
            writeOk = imageWriter.write(saveImage);
            if (!writeOk)
                errorString = imageWriter.errorString();
        }
        if (!writeOk) {
            QMessageBox msgBox;
            msgBox.setText(errorString);
            msgBox.exec();
        } else {
            buffer->clearDirty();
        }
    }
}

void MainWindow::saveAs()
{
    QString path = QFileDialog::getSaveFileName(nullptr, tr("Save file"), buffer->path());
    if (path.isEmpty())
        return;

    RawSaveOptions options;
    if (path.toLower().endsWith(".raw")) {
        RawSaveOptionsDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted)
            return;
        options = dlg.options();
    }

    saveFile(path, options);
}

void MainWindow::newWindow()
{
    BufferView *bufferView = new BufferView();
    bufferView->installEventFilter(this);
    bufferView->setBuffer(buffer);
    connect(bufferView, &BufferView::magnifyAtPointRequested, this, &MainWindow::openMagnifiedViewAt);
    connect(bufferView, &BufferView::cursorMoved, this, &MainWindow::updateCursorStatus);
    connect(bufferView, &BufferView::squashDialogsRequested, this, &MainWindow::squashDialogs);
    connect(bufferView, &BufferView::toggleAllDialogsRequested, this, &MainWindow::toggleAllDialogs);
    bufferView->show();
    bufferViews.append(bufferView);
    if (!activeBufferView)
        activeBufferView = bufferView;
    if (bufferViews.count() > 1)
        ui->actionWindowSingleWindow->setEnabled(false);
}

void MainWindow::openMagnifiedViewAt(int zoomLevel, QPoint point)
{
    BufferView *bufferView = new BufferView();
    bufferView->installEventFilter(this);
    bufferView->setBuffer(buffer);
    bufferView->setZoomLevel(zoomLevel);

    QSize ideal = bufferView->idealSize(zoomLevel);
    if (ideal.isValid()) {
        QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
        bufferView->resize(ideal.boundedTo(screen.size()));
    }

    if (activeBufferView) {
        QRect srcFrame = activeBufferView->frameGeometry();
        bufferView->move(srcFrame.right() + 1, srcFrame.top());
    }

    if (point.x() >= 0 && point.y() >= 0)
        bufferView->centerOn(point);

    connect(bufferView, &BufferView::magnifyAtPointRequested, this, &MainWindow::openMagnifiedViewAt);
    connect(bufferView, &BufferView::cursorMoved, this, &MainWindow::updateCursorStatus);
    connect(bufferView, &BufferView::squashDialogsRequested, this, &MainWindow::squashDialogs);
    connect(bufferView, &BufferView::toggleAllDialogsRequested, this, &MainWindow::toggleAllDialogs);
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
        if (bufferViews.count() <= 1)
            ui->actionWindowSingleWindow->setEnabled(true);
    } else {
        openFile();
    }
}

void MainWindow::toggleFloatPanels(bool checked)
{
    Tool::setFloatPanelsByDefault(checked);
}

void MainWindow::toggleSingleWindowMode(bool checked)
{
    if (checked) {
        // Dock all open panels and strip their floatable feature
        for (Tool *tool : tools) {
            QDockWidget *dw = tool->dockWidget();
            if (dw) {
                dw->setFloating(false);
                dw->setFeatures(dw->features() & ~QDockWidget::DockWidgetFloatable);
            }
        }
        Tool::setFloatPanelsByDefault(false);
        Tool::setSingleWindowMode(true);
        ui->actionWindowFloatPanels->setEnabled(false);
        ui->actionWindowNewWindow->setEnabled(false);

        // Rearrange widgetMain's gridLayout to insert the BufferView above the tools/palette.
        // Remove the two sub-layouts by searching for them to avoid index-order assumptions.
        QLayoutItem *toolsItem = nullptr;
        QLayoutItem *paletteItem = nullptr;
        for (int i = ui->gridLayout->count() - 1; i >= 0; --i) {
            QLayoutItem *item = ui->gridLayout->itemAt(i);
            if (item->layout() == ui->toolsLayout)
                toolsItem = ui->gridLayout->takeAt(i);
            else if (item->layout() == ui->paletteLayout)
                paletteItem = ui->gridLayout->takeAt(i);
        }
        ui->gridLayout->addWidget(activeBufferView, 0, 0);
        ui->gridLayout->addItem(toolsItem, 1, 0);
        ui->gridLayout->addItem(paletteItem, 2, 0);

        // Reparenting hides the view; show it so the layout counts its size.
        activeBufferView->show();

        // Grow the window to fit the buffer. Use explicit arithmetic rather than
        // gridLayout->sizeHint() because the layout cache may not have updated yet.
        int left, top, right, bottom;
        ui->gridLayout->getContentsMargins(&left, &top, &right, &bottom);
        QSize bvSize = activeBufferView->sizeHint();
        resize(qMax(width(),  bvSize.width()  + left + right),
               height() + bvSize.height() + ui->gridLayout->verticalSpacing());

    } else {
        // Restore widgetMain's gridLayout: remove the BufferView and shift sub-layouts back up.
        QLayoutItem *toolsItem = nullptr;
        QLayoutItem *paletteItem = nullptr;
        QLayoutItem *bvItem = nullptr;
        for (int i = ui->gridLayout->count() - 1; i >= 0; --i) {
            QLayoutItem *item = ui->gridLayout->itemAt(i);
            if (item->layout() == ui->toolsLayout)
                toolsItem = ui->gridLayout->takeAt(i);
            else if (item->layout() == ui->paletteLayout)
                paletteItem = ui->gridLayout->takeAt(i);
            else if (item->widget() == activeBufferView)
                bvItem = ui->gridLayout->takeAt(i);
        }
        ui->gridLayout->addItem(toolsItem, 0, 0);
        ui->gridLayout->addItem(paletteItem, 1, 0);

        // bvItem is a QWidgetItem wrapper; delete it after reparenting the widget
        activeBufferView->setParent(nullptr);
        delete bvItem;
        activeBufferView->show();

        resize(minimumSizeHint());

        // Restore floatable feature on existing panels
        for (Tool *tool : tools) {
            QDockWidget *dw = tool->dockWidget();
            if (dw)
                dw->setFeatures(dw->features() | QDockWidget::DockWidgetFloatable);
        }
        Tool::setSingleWindowMode(false);
        Tool::setFloatPanelsByDefault(ui->actionWindowFloatPanels->isChecked());
        ui->actionWindowFloatPanels->setEnabled(true);
        ui->actionWindowNewWindow->setEnabled(true);
    }
}

void MainWindow::loadPalette(const QString &path)
{
    QImage image(path);
    for (int i = 0; i < qMin(image.colorCount(), buffer->image().colorCount()); i++) {
        buffer->image().setColor(i, image.color(i));
    }
    updatePalette();
    emit buffer->modified(buffer->image().rect());
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

void MainWindow::imageCopy()
{
    QApplication::clipboard()->setImage(buffer->image().convertToFormat(QImage::Format_RGB32));
}

void MainWindow::imagePaste()
{
    QImage img = QApplication::clipboard()->image();
    if (img.isNull())
        return;

    // Scale to canvas size if needed, then quantize to current palette
    QImage src = img.scaled(buffer->image().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage indexed = convertToIndexed(src);

    QImage &canvas = buffer->image();
    for (int y = 0; y < canvas.height(); y++)
        for (int x = 0; x < canvas.width(); x++)
            canvas.setPixel(x, y, static_cast<uint>(indexed.pixelIndex(x, y)));

    buffer->notifyModified(canvas.rect());
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

QImage MainWindow::convertToIndexed(const QImage &source) const
{
    QImage rgb = source.convertToFormat(QImage::Format_RGB32);
    const QVector<QRgb> palette = buffer->image().colorTable();
    QImage indexed(rgb.size(), QImage::Format_Indexed8);
    indexed.setColorTable(palette);
    for (int y = 0; y < rgb.height(); y++)
        for (int x = 0; x < rgb.width(); x++)
            indexed.setPixel(x, y, nearestColorIndex(rgb.pixel(x, y), palette));
    return indexed;
}

void MainWindow::brushLoad()
{
    QString path = QFileDialog::getOpenFileName(nullptr, tr("Load Brush"));
    if (path.isEmpty())
        return;
    QImage img(path);
    if (img.isNull())
        return;
    QImage indexed = convertToIndexed(img);
    buffer->setPen(new Brush(indexed, -1, buffer));
}

void MainWindow::brushSave()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (!brush)
        return;
    QString path = QFileDialog::getSaveFileName(nullptr, tr("Save Brush"));
    if (path.isEmpty())
        return;
    QImageWriter writer(path);
    if (!writer.write(brush->image().convertToFormat(QImage::Format_RGB32))) {
        QMessageBox::warning(nullptr, tr("Save Brush"), writer.errorString());
    }
}

void MainWindow::brushCopy()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush)
        QApplication::clipboard()->setImage(brush->image().convertToFormat(QImage::Format_RGB32));
}

void MainWindow::brushPaste()
{
    QImage img = QApplication::clipboard()->image();
    if (img.isNull())
        return;
    QImage indexed = convertToIndexed(img);
    buffer->setPen(new Brush(indexed, -1, buffer));
}

void MainWindow::brushDelete()
{
    buffer->setPen(penTip);
}

void MainWindow::brushRemap()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (!brush)
        return;
    brush->remap(buffer->image().colorTable());
}

void MainWindow::brushBgFgSwap()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (!brush)
        return;
    brush->replaceColor(static_cast<int>(buffer->eraseColor()),
                        static_cast<int>(buffer->paintColor()));
    unsigned fg = buffer->paintColor();
    buffer->setPaintColor(buffer->eraseColor());
    buffer->setEraseColor(fg);
}

void MainWindow::brushAutoBackground()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (!brush)
        return;
    brush->detectBackground();
}

Brush *MainWindow::brushForTransform()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush)
        brush->storeOriginal();
    return brush;
}

void MainWindow::brushFlipHorizontal()
{
    Brush *brush = brushForTransform();
    if (brush) brush->flipHorizontal();
}

void MainWindow::brushFlipVertical()
{
    Brush *brush = brushForTransform();
    if (brush) brush->flipVertical();
}

void MainWindow::brushRotate90CW()
{
    Brush *brush = brushForTransform();
    if (brush) brush->rotate90CW();
}

void MainWindow::brushRotate90CCW()
{
    Brush *brush = brushForTransform();
    if (brush) brush->rotate90CCW();
}

void MainWindow::brushRotateNumeric()
{
    Brush *brush = brushForTransform();
    if (!brush) return;
    bool ok;
    double degrees = QInputDialog::getDouble(this, "Rotate Brush", "Degrees (clockwise):", 45.0, -360.0, 360.0, 1, &ok);
    if (ok) brush->rotateByDegrees(degrees);
}

void MainWindow::brushDouble()
{
    Brush *brush = brushForTransform();
    if (brush) brush->doubleSize();
}

void MainWindow::brushDoubleX()
{
    Brush *brush = brushForTransform();
    if (brush) brush->doubleWidth();
}

void MainWindow::brushDoubleY()
{
    Brush *brush = brushForTransform();
    if (brush) brush->doubleHeight();
}

void MainWindow::brushHalve()
{
    Brush *brush = brushForTransform();
    if (brush) brush->halveSize();
}

void MainWindow::brushHalveX()
{
    Brush *brush = brushForTransform();
    if (brush) brush->halveWidth();
}

void MainWindow::brushHalveY()
{
    Brush *brush = brushForTransform();
    if (brush) brush->halveHeight();
}

void MainWindow::brushScaleToSize()
{
    Brush *brush = brushForTransform();
    if (!brush)
        return;
    bool ok;
    int w = QInputDialog::getInt(this, "Scale Brush", "Width:", brush->image().width(), 1, 4096, 1, &ok);
    if (!ok) return;
    int h = QInputDialog::getInt(this, "Scale Brush", "Height:", brush->image().height(), 1, 4096, 1, &ok);
    if (!ok) return;
    brush->scale(w, h);
}

void MainWindow::brushShearX()
{
    Brush *brush = brushForTransform();
    if (!brush) return;
    bool ok;
    double factor = QInputDialog::getDouble(this, "Shear X", "Factor (-2.0 to 2.0):", 0.5, -2.0, 2.0, 2, &ok);
    if (ok) brush->shearX(factor);
}

void MainWindow::brushShearY()
{
    Brush *brush = brushForTransform();
    if (!brush) return;
    bool ok;
    double factor = QInputDialog::getDouble(this, "Shear Y", "Factor (-2.0 to 2.0):", 0.5, -2.0, 2.0, 2, &ok);
    if (ok) brush->shearY(factor);
}

void MainWindow::brushBendX()
{
    Brush *brush = brushForTransform();
    if (!brush) return;
    bool ok;
    double amount = QInputDialog::getDouble(this, "Bend X", "Amount (-1.0 to 1.0):", 0.3, -1.0, 1.0, 2, &ok);
    if (ok) brush->bendX(amount);
}

void MainWindow::brushBendY()
{
    Brush *brush = brushForTransform();
    if (!brush) return;
    bool ok;
    double amount = QInputDialog::getDouble(this, "Bend Y", "Amount (-1.0 to 1.0):", 0.3, -1.0, 1.0, 2, &ok);
    if (ok) brush->bendY(amount);
}

void MainWindow::brushOutline()
{
    Brush *brush = brushForTransform();
    if (brush) brush->outline(static_cast<int>(buffer->paintColor()));
}

void MainWindow::brushTrim()
{
    Brush *brush = brushForTransform();
    if (brush) brush->trim();
}

void MainWindow::brushTileCut()
{
    Brush *brush = brushForTransform();
    if (brush) brush->tileCut();
}

void MainWindow::brushRestore()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush) brush->restoreOriginal();
}

void MainWindow::pickForegroundColor()
{
    for (Tool *tool : tools) {
        if (PickColorTool *picker = qobject_cast<PickColorTool *>(tool)) {
            picker->activateOneShotForeground(buffer->tool());
            break;
        }
    }
}

void MainWindow::pickBackgroundColor()
{
    for (Tool *tool : tools) {
        if (PickColorTool *picker = qobject_cast<PickColorTool *>(tool)) {
            picker->activateOneShotBackground(buffer->tool());
            break;
        }
    }
}

void MainWindow::paletteSave(const QString &path)
{
    if (path.isEmpty()) {
        savePaletteDialog->show();
        return;
    }
    int n = buffer->image().colorCount();
    QImage paletteImage(n, 1, QImage::Format_Indexed8);
    paletteImage.setColorTable(buffer->image().colorTable());
    for (int i = 0; i < n; i++)
        paletteImage.setPixel(i, 0, static_cast<uint>(i));
    if (!paletteImage.save(path)) {
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to save palette."));
        msgBox.exec();
    }
}

void MainWindow::paletteSpread()
{
    paletteMode = PaletteSpread;
    updateWindowTitle();
}

void MainWindow::paletteDefault()
{
    paletteUndoSnapshot_ = buffer->image().colorTable();
    buffer->resetToDefaultPalette();
}

void MainWindow::paletteRestore()
{
    paletteUndoSnapshot_ = buffer->image().colorTable();
    QVector<QRgb> restore = paletteRestorePoint_;
    int count = buffer->image().colorCount();
    for (int i = 0; i < qMin(restore.size(), count); i++)
        buffer->image().setColor(i, restore[i]);
    emit buffer->paletteModified();
    emit buffer->modified(buffer->image().rect());
}

void MainWindow::paletteUndo()
{
    QVector<QRgb> current = buffer->image().colorTable();
    int count = buffer->image().colorCount();
    for (int i = 0; i < qMin(paletteUndoSnapshot_.size(), count); i++)
        buffer->image().setColor(i, paletteUndoSnapshot_[i]);
    paletteUndoSnapshot_ = current;
    emit buffer->paletteModified();
    emit buffer->modified(buffer->image().rect());
}

void MainWindow::paletteRemapPage()
{
    if (paletteRestorePoint_.isEmpty())
        return;

    const QVector<QRgb> &origPalette = paletteRestorePoint_;
    const int colorCount = buffer->image().colorCount();
    QImage &image = buffer->image();

    // Build a lookup: for each original palette index, find the closest entry in the current palette
    const QVector<QRgb> currentPalette = image.colorTable();
    QVector<int> remap(qMin(origPalette.size(), colorCount));
    for (int i = 0; i < remap.size(); i++)
        remap[i] = nearestColorIndex(origPalette[i], currentPalette);

    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            int idx = image.pixelIndex(x, y);
            if (idx < remap.size())
                image.setPixel(x, y, static_cast<uint>(remap[idx]));
        }
    }
    emit buffer->modified(image.rect());
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{

    if (watched == qApp && event->type() == QEvent::Quit) {
        if (isVisible()) {
            if (close())
                qApp->quit();
            return true;
        }
        return false;
    }

    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        if (activeBufferView) {
            QWidget *fw = QApplication::focusWidget();
            bool inBufferView = fw && (fw == activeBufferView || activeBufferView->isAncestorOf(fw));
            bool inTextInput  = qobject_cast<QLineEdit *>(fw) || qobject_cast<QAbstractSpinBox *>(fw);
            if (!inBufferView && !inTextInput) {
                auto *keyEvent = static_cast<QKeyEvent *>(event);
                if (event->type() == QEvent::KeyPress)
                    activeBufferView->handleKey(keyEvent);
                else
                    activeBufferView->handleKeyRelease(keyEvent);
                return true;
            }
        }
        break;
    }
    case QEvent::WindowActivate:
        foreach (BufferView *bufferView, bufferViews) {
            if (watched == bufferView) {
                activeBufferView = bufferView;
            }
        }
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void MainWindow::updateStatusBarStatic()
{
    if (!buffer) return;

    Tool *tool = buffer->tool();
    statusToolLabel_->setText(tool ? tool->name() : QString());
    statusModeLabel_->setText(effectiveDrawModeName(buffer->paintMode()));

    auto colorAt = [this](unsigned index) {
        return QColor(buffer->image().color(static_cast<int>(index)));
    };
    statusColorsButton_->setPaintColor(buffer->paintColor(), colorAt(buffer->paintColor()));
    statusColorsButton_->setEraseColor(buffer->eraseColor(), colorAt(buffer->eraseColor()));

    bool hasSizeInfo = false;
    if (Brush *brush = qobject_cast<Brush *>(buffer->pen())) {
        statusBrushLabel_->setText(QString("%1\xc3\x97%2").arg(brush->image().width()).arg(brush->image().height()));
        statusBrushLabel_->setToolTip(tr("Brush size"));
        hasSizeInfo = true;
    } else if (PenTip *tip = buffer->penTip()) {
        statusBrushLabel_->setText(QString("%1\xc3\x97%2").arg(tip->width()).arg(tip->height()));
        statusBrushLabel_->setToolTip(tr("Pen size"));
        hasSizeInfo = true;
    } else {
        statusBrushLabel_->setText(QString());
    }
    statusBrushLabel_->setVisible(hasSizeInfo);
    statusBrushSep_->setVisible(hasSizeInfo);
}

void MainWindow::updateCursorStatus(QPoint point, bool valid)
{
    lastCursorPoint_ = point;
    lastCursorValid_ = valid;

    updateStatusBarStatic();

    if (!valid || !buffer) {
        statusCoordsSep_->setVisible(false);
        statusCoordsLabel_->clear();
        return;
    }

    QString toolStatus = buffer->tool() ? buffer->tool()->status() : QString();

    statusCoordsSep_->setVisible(true);
    QString text = QString("%1  %2").arg(point.x()).arg(point.y());
    if (!toolStatus.isEmpty())
        text += "  " + toolStatus;
    statusCoordsLabel_->setText(text);
}

void MainWindow::updateWindowTitle()
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
    case PaletteSpread:
        windowTitle = "Excellence - Spread palette colors";
        break;
    default:
        windowTitle = buffer && buffer->isDirty() ? "*Excellence" : "Excellence";
        break;
    }

    setWindowTitle(windowTitle);
}

void MainWindow::onDirtyChanged(bool)
{
    updateWindowTitle();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (buffer && buffer->isDirty()) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this, tr("Unsaved changes"),
            tr("Save changes before quitting?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);
        if (result == QMessageBox::Save) {
            saveFile();
            if (buffer->isDirty()) {
                event->ignore();
                return;
            }
        } else if (result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    for (BufferView *view : bufferViews)
        view->close();
    event->accept();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Excellence"),
        tr("<b>Excellence</b><br>"
           "Version 1.0<br><br>"
           "A pixel art editor inspired by Brilliance on the Commodore Amiga.<br><br>"
           "Copyright &copy; 2024 Vesa Halttunen"));
}

QVector<QWidget *> MainWindow::collectAndHideToolDialogs()
{
    QVector<QWidget *> result;
    for (Tool *tool : tools) {
        QWidget *w = tool->dockWidget();
        if (w && w->isVisible()) {
            result.append(w);
            w->hide();
        }
    }
    return result;
}

void MainWindow::squashDialogs()
{
    if (squashedDialogs_.isEmpty())
        squashedDialogs_ = collectAndHideToolDialogs();
    else {
        for (QWidget *w : squashedDialogs_)
            w->show();
        squashedDialogs_.clear();
    }
}

void MainWindow::toggleAllDialogs()
{
    if (isVisible()) {
        hiddenDialogs_ = collectAndHideToolDialogs();
        hide();
    } else {
        show();
        for (QWidget *w : hiddenDialogs_)
            w->show();
        hiddenDialogs_.clear();
    }
}
