#include "palettebutton.h"
#include <QInputDialog>
#include <QSizePolicy>
#include <QTimer>
#include <QFileDialog>
#include <QImageWriter>
#include <QImageReader>
#include <QMessageBox>
#include <QApplication>
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

    connect(openDialog, SIGNAL(fileSelected(QString)), this, SLOT(openFile(QString)));
    connect(loadPaletteDialog, SIGNAL(fileSelected(QString)), this, SLOT(loadPalette(QString)));
    savePaletteDialog->setAcceptMode(QFileDialog::AcceptSave);
    connect(savePaletteDialog, SIGNAL(fileSelected(QString)), this, SLOT(paletteSave(QString)));
    connect(ui->actionFileQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionFileNew, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->actionFileOpen, SIGNAL(triggered()), openDialog, SLOT(show()));
    connect(ui->actionFileSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(ui->actionFileSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
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
    connect(ui->actionBrushDouble, SIGNAL(triggered()), this, SLOT(brushDouble()));
    connect(ui->actionBrushHalve, SIGNAL(triggered()), this, SLOT(brushHalve()));
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
    QStringList arguments = qApp->arguments();
    openFile(arguments.length() > 1 ? arguments.last() : QString());

    penTip->setPaintColor(buffer->paintColor());
    penTip->setEraseColor(buffer->eraseColor());
    toolPenTip->setPaintColor(1);

    newWindow();
    activateWindow();
    raise();

    for (int i = 0; i < tools.count(); i++) {
        tools.at(i)->addButtonToGridLayout(ui->toolsLayout);
        tools.at(i)->setBuffer(buffer);
    }

    connect(ui->currentColorsButton, SIGNAL(foregroundClicked()), this, SLOT(pickForegroundColor()));
    connect(ui->currentColorsButton, SIGNAL(backgroundClicked()), this, SLOT(pickBackgroundColor()));

    for (Tool *tool : tools) {
        if (qobject_cast<DrawTool *>(tool)) { buffer->setTool(tool); break; }
    }
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
    paletteRestorePoint_ = buffer->image().colorTable();
    paletteUndoSnapshot_ = paletteRestorePoint_;
    connect(buffer, SIGNAL(paintColorChanged(unsigned, QColor)), penTip, SLOT(setPaintColor(unsigned)));
    connect(buffer, SIGNAL(eraseColorChanged(unsigned, QColor)), penTip, SLOT(setEraseColor(unsigned)));
    connect(buffer, SIGNAL(paintColorChanged(unsigned, QColor)), ui->currentColorsButton, SLOT(setPaintColor(unsigned, QColor)));
    connect(buffer, SIGNAL(eraseColorChanged(unsigned, QColor)), ui->currentColorsButton, SLOT(setEraseColor(unsigned, QColor)));

    delete oldBuffer;
}

void MainWindow::updatePalette()
{
    for (int i = 0; i < buffer->image().colorCount(); i++) {
        PaletteButton *button = static_cast<PaletteButton *>(ui->paletteLayout->itemAt(i)->widget());
        button->setColor(QColor(buffer->image().color(i)));
    }
    ui->currentColorsButton->setPaintColor(buffer->paintColor(), QColor(buffer->image().color(static_cast<int>(buffer->paintColor()))));
    ui->currentColorsButton->setEraseColor(buffer->eraseColor(), QColor(buffer->image().color(static_cast<int>(buffer->eraseColor()))));
}

void MainWindow::openFile(const QString &path)
{
    setBuffer(new Buffer(path, this));
    openDialog->setDirectory(path);
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
    QString path = QFileDialog::getSaveFileName(nullptr, tr("Open file"), buffer->path());

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
    QImage indexed(rgb.size(), QImage::Format_Indexed8);
    indexed.setColorTable(buffer->image().colorTable());
    for (int y = 0; y < rgb.height(); y++) {
        for (int x = 0; x < rgb.width(); x++) {
            QRgb pixel = rgb.pixel(x, y);
            int bestIdx = 0, bestDist = INT_MAX;
            for (int i = 0; i < buffer->image().colorCount(); i++) {
                QRgb c = buffer->image().color(i);
                int dr = qRed(pixel) - qRed(c);
                int dg = qGreen(pixel) - qGreen(c);
                int db = qBlue(pixel) - qBlue(c);
                int dist = dr*dr + dg*dg + db*db;
                if (dist < bestDist) { bestDist = dist; bestIdx = i; }
            }
            indexed.setPixel(x, y, bestIdx);
        }
    }
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

void MainWindow::brushFlipHorizontal()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush) brush->flipHorizontal();
}

void MainWindow::brushFlipVertical()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush) brush->flipVertical();
}

void MainWindow::brushRotate90CW()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush) brush->rotate90CW();
}

void MainWindow::brushRotate90CCW()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush) brush->rotate90CCW();
}

void MainWindow::brushDouble()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush) brush->doubleSize();
}

void MainWindow::brushHalve()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush) brush->halveSize();
}

void MainWindow::brushScaleToSize()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
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
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (!brush) return;
    bool ok;
    double factor = QInputDialog::getDouble(this, "Shear X", "Factor (-2.0 to 2.0):", 0.5, -2.0, 2.0, 2, &ok);
    if (ok) brush->shearX(factor);
}

void MainWindow::brushShearY()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (!brush) return;
    bool ok;
    double factor = QInputDialog::getDouble(this, "Shear Y", "Factor (-2.0 to 2.0):", 0.5, -2.0, 2.0, 2, &ok);
    if (ok) brush->shearY(factor);
}

void MainWindow::brushBendX()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (!brush) return;
    bool ok;
    double amount = QInputDialog::getDouble(this, "Bend X", "Amount (-1.0 to 1.0):", 0.3, -1.0, 1.0, 2, &ok);
    if (ok) brush->bendX(amount);
}

void MainWindow::brushBendY()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (!brush) return;
    bool ok;
    double amount = QInputDialog::getDouble(this, "Bend Y", "Amount (-1.0 to 1.0):", 0.3, -1.0, 1.0, 2, &ok);
    if (ok) brush->bendY(amount);
}

void MainWindow::brushOutline()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush) brush->outline(static_cast<int>(buffer->paintColor()));
}

void MainWindow::brushTrim()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
    if (brush) brush->trim();
}

void MainWindow::brushTileCut()
{
    Brush *brush = qobject_cast<Brush *>(buffer->pen());
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
    QVector<int> remap(qMin(origPalette.size(), colorCount));
    for (int i = 0; i < remap.size(); i++) {
        QRgb origColor = origPalette[i];
        int bestIndex = i;
        int bestDist = INT_MAX;
        for (int j = 0; j < colorCount; j++) {
            QRgb c = image.color(j);
            int dr = qRed(origColor)   - qRed(c);
            int dg = qGreen(origColor) - qGreen(c);
            int db = qBlue(origColor)  - qBlue(c);
            int dist = dr*dr + dg*dg + db*db;
            if (dist < bestDist) {
                bestDist = dist;
                bestIndex = j;
            }
        }
        remap[i] = bestIndex;
    }

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
    case PaletteSpread:
        windowTitle = "Excellence - Spread palette colors";
        break;
    default:
        windowTitle = "Excellence";
        break;
    }

    if (paletteIndex >= 0) {
        windowTitle.append(paletteMode == Pick ? QString(" %1").arg(paletteIndex) : QString(" %1->%2").arg(buffer->paintColor()).arg(paletteIndex));
    }

    setWindowTitle(windowTitle);
}
