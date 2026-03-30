#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRect>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QWidget>
#include "brush.h"
#include "buffer.h"
#include "undobuffer.h"
#include "algorithms.h"
#include "brushtool.h"

// ── BrushWellButton ──────────────────────────────────────────────────────────

BrushWellButton::BrushWellButton(QWidget *parent)
    : QAbstractButton(parent), empty_(true)
{
    setFixedSize(40, 40);
}

void BrushWellButton::store(const QImage &image)
{
    brushImage_ = image;
    empty_ = false;
    QImage rgb = image.convertToFormat(QImage::Format_RGB32);
    thumbnail_ = QPixmap::fromImage(
        rgb.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    update();
}

void BrushWellButton::clear()
{
    brushImage_ = QImage();
    thumbnail_ = QPixmap();
    empty_ = true;
    update();
}

bool BrushWellButton::isEmpty() const
{
    return empty_;
}

const QImage &BrushWellButton::brushImage() const
{
    return brushImage_;
}

void BrushWellButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (empty_) {
        p.fillRect(rect(), QColor(100, 100, 100));
    } else {
        p.fillRect(rect(), Qt::white);
        if (!thumbnail_.isNull())
            p.drawPixmap((width() - thumbnail_.width()) / 2,
                         (height() - thumbnail_.height()) / 2, thumbnail_);
    }
    p.setPen(Qt::black);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

void BrushWellButton::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractButton::mouseReleaseEvent(event);
    if (rect().contains(event->pos()) && event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ControlModifier)
            emit ctrlClicked();
        else
            emit clicked();
    }
}

// ── BrushTool ────────────────────────────────────────────────────────────────

// ── BrushHandleWidget ────────────────────────────────────────────────────────

BrushHandleWidget::BrushHandleWidget(BrushTool *tool, QWidget *parent)
    : QWidget(parent), tool_(tool)
{
    setFixedSize(80, 80);
    setCursor(Qt::CrossCursor);
}

void BrushHandleWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    update();
}

void BrushHandleWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QImage img = tool_->currentBrushImage();
    if (!img.isNull()) {
        QImage rgb = img.convertToFormat(QImage::Format_RGB32);
        p.drawImage(rect(), rgb);
    } else {
        p.fillRect(rect(), QColor(80, 80, 80));
    }

    // Draw anchor crosshair
    QPoint handle = tool_->currentHandleOffset();
    if (!img.isNull() && img.width() > 0 && img.height() > 0) {
        int hx = handle.x() * width()  / img.width();
        int hy = handle.y() * height() / img.height();
        p.setPen(QPen(Qt::red, 1));
        p.drawLine(hx - 4, hy, hx + 4, hy);
        p.drawLine(hx, hy - 4, hx, hy + 4);
    }

    p.setPen(Qt::black);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

void BrushHandleWidget::mousePressEvent(QMouseEvent *event)
{
    QImage img = tool_->currentBrushImage();
    if (img.isNull() || width() == 0 || height() == 0)
        return;
    int ox = event->pos().x() * img.width()  / width();
    int oy = event->pos().y() * img.height() / height();
    tool_->setHandle(QPoint(ox, oy));
    update();
}

// ── BrushTool ────────────────────────────────────────────────────────────────

BrushTool BrushTool::instance;

BrushTool::BrushTool(QObject *parent) : Tool(parent),
    mode_(Rectangle),
    undoBuffer_(nullptr),
    handleWidget_(nullptr),
    dimensionsLabel_(nullptr)
{
    for (int i = 0; i < WellCount; i++)
        wellButtons_[i] = nullptr;
}

void BrushTool::cancel()
{
    if (!undoBuffer_) return;
    undoBuffer_->apply(buffer_);
    buffer_->notifyModified(buffer_->image().rect());
    delete undoBuffer_;
    undoBuffer_ = nullptr;
    polygon_.clear();
}

void BrushTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect BrushTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (mode_ == Freehand) {
        polygon_.clear();
        polygon_ << point;
        prevPoint_ = point;
        undoBuffer_ = new UndoBuffer(QPoint(), buffer_->image().copy(), this);
        return buffer_->toolPen()->paint(point, buffer_);
    }

    startPoint_ = point;
    QRect rect = changes(point);
    undoBuffer_ = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
    return draw(point);
}

QRect BrushTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton)
        return QRect();

    if (mode_ == Freehand) {
        polygon_ << point;
        QRect changedRect;
        Algorithms::line(prevPoint_, point, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(buffer_->toolPen()->paint(p, buffer_));
        });
        prevPoint_ = point;
        return changedRect;
    }

    if (!undoBuffer_)
        return QRect();

    undoBuffer_->apply(buffer_);
    delete undoBuffer_;

    QRect changedRect;
    Algorithms::rectangle(startPoint_, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->changes(point)); });
    undoBuffer_ = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
    Algorithms::rectangle(startPoint_, point, [this](const QPoint &point) { this->draw(point); });
    return changedRect;
}

QRect BrushTool::release(const QPoint &point)
{
    if (mode_ == Freehand) {
        polygon_ << point;

        if (!undoBuffer_)
            return QRect();

        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;

        QRect bounds = polygon_.boundingRect().intersected(buffer_->image().rect());
        if (bounds.isEmpty())
            return buffer_->image().rect();

        QImage areaImage = buffer_->image().copy(bounds);

        QImage mask(bounds.size(), QImage::Format_ARGB32);
        mask.fill(Qt::transparent);
        {
            QPainter p(&mask);
            p.setPen(Qt::NoPen);
            p.setBrush(Qt::white);
            p.drawPolygon(polygon_.translated(-bounds.topLeft()));
        }

        int eraseIdx = static_cast<int>(buffer_->eraseColor());
        for (int y = 0; y < bounds.height(); y++)
            for (int x = 0; x < bounds.width(); x++)
                if (qAlpha(mask.pixel(x, y)) == 0)
                    areaImage.setPixel(x, y, static_cast<uint>(eraseIdx));

        buffer_->setPen(new Brush(areaImage, eraseIdx, buffer_));
        buffer_->setPaintMode(Buffer::BrushMode);
        buffer_->setTool(tools.at(0));
        return buffer_->image().rect();
    }

    if (!undoBuffer_)
        return QRect();

    QRect changedRect = undoBuffer_->rect();
    undoBuffer_->apply(buffer_);
    delete undoBuffer_;
    undoBuffer_ = nullptr;

    QImage image = buffer_->image().copy(QRect(startPoint_, point));
    buffer_->setPen(new Brush(image, static_cast<int>(buffer_->eraseColor())));
    buffer_->setPaintMode(Buffer::BrushMode);
    buffer_->setTool(tools.at(0));

    if (mouseButton_ == Qt::RightButton)
        Algorithms::fillRectangle(startPoint_, point, [this](const QPoint &p) { draw(p); });

    return changedRect;
}

QRect BrushTool::changes(const QPoint &point)
{
    return buffer_->toolPen()->rect(point);
}

QRect BrushTool::draw(const QPoint &point)
{
    if (mouseButton_ == Qt::LeftButton) {
        return buffer_->toolPen()->paint(point, buffer_);
    } else {
        return buffer_->toolPen()->erase(point, buffer_);
    }
}

QImage BrushTool::currentBrushImage() const
{
    Brush *brush = qobject_cast<Brush *>(buffer_ ? buffer_->pen() : nullptr);
    return brush ? brush->image() : QImage();
}

QPoint BrushTool::currentHandleOffset() const
{
    Brush *brush = qobject_cast<Brush *>(buffer_ ? buffer_->pen() : nullptr);
    return brush ? brush->handleOffset() : QPoint();
}

void BrushTool::setHandle(const QPoint &offset)
{
    Brush *brush = qobject_cast<Brush *>(buffer_ ? buffer_->pen() : nullptr);
    if (!brush)
        return;
    brush->setHandleOffset(offset);
    if (handleWidget_)
        handleWidget_->update();
}

void BrushTool::setHandleTopLeft()     { setHandle(QPoint(0, 0)); }
void BrushTool::setHandleTopRight()    { QImage img = currentBrushImage(); setHandle(QPoint(img.width() - 1, 0)); }
void BrushTool::setHandleCenter()      { QImage img = currentBrushImage(); setHandle(QPoint(img.width() / 2, img.height() / 2)); }
void BrushTool::setHandleBottomLeft()  { QImage img = currentBrushImage(); setHandle(QPoint(0, img.height() - 1)); }
void BrushTool::setHandleBottomRight() { QImage img = currentBrushImage(); setHandle(QPoint(img.width() - 1, img.height() - 1)); }

void BrushTool::storeToWell(int index)
{
    Brush *brush = qobject_cast<Brush *>(buffer_->pen());
    if (!brush)
        return;
    wells_[index] = brush->image();
    if (wellButtons_[index])
        wellButtons_[index]->store(wells_[index]);
}

void BrushTool::wellClicked(int index)
{
    if (wellButtons_[index]->isEmpty())
        storeToWell(index);
    else
        buffer_->setPen(new Brush(wells_[index], -1, buffer_));
}

void BrushTool::wellCtrlClicked(int index)
{
    storeToWell(index);
}

QWidget *BrushTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Brush");

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(4);
    vbox->setContentsMargins(4, 4, 4, 4);

    // ── Dimensions + Tile Cut ──────────────────────────────────────────────
    {
        QHBoxLayout *dimRow = new QHBoxLayout;
        Brush *brush = qobject_cast<Brush *>(buffer_ ? buffer_->pen() : nullptr);
        QString dimText = brush
            ? QString("%1 × %2").arg(brush->image().width()).arg(brush->image().height())
            : QString("– × –");
        dimensionsLabel_ = new QLabel(dimText, w);
        dimRow->addWidget(dimensionsLabel_);
        dimRow->addStretch();
        QPushButton *tileCutBtn = new QPushButton("Tile Cut", w);
        connect(tileCutBtn, SIGNAL(clicked()), this, SLOT(brushTileCut()));
        dimRow->addWidget(tileCutBtn);
        vbox->addLayout(dimRow);
    }

    // ── Wells row ──────────────────────────────────────────────────────────
    QHBoxLayout *wellsRow = new QHBoxLayout;
    wellsRow->setSpacing(2);

    QSignalMapper *clickMapper = new QSignalMapper(w);
    QSignalMapper *ctrlMapper  = new QSignalMapper(w);
    connect(clickMapper, SIGNAL(mapped(int)), this, SLOT(wellClicked(int)));
    connect(ctrlMapper,  SIGNAL(mapped(int)), this, SLOT(wellCtrlClicked(int)));

    for (int i = 0; i < WellCount; i++) {
        BrushWellButton *btn = new BrushWellButton(w);
        wellButtons_[i] = btn;
        if (!wells_[i].isNull())
            btn->store(wells_[i]);
        clickMapper->setMapping(btn, i);
        ctrlMapper->setMapping(btn, i);
        connect(btn, SIGNAL(clicked()),     clickMapper, SLOT(map()));
        connect(btn, SIGNAL(ctrlClicked()), ctrlMapper,  SLOT(map()));
        wellsRow->addWidget(btn);
    }
    vbox->addLayout(wellsRow);

    // ── Separator ──────────────────────────────────────────────────────────
    QFrame *sep = new QFrame(w);
    sep->setFrameShape(QFrame::HLine);
    vbox->addWidget(sep);

    // ── Handle section ─────────────────────────────────────────────────────
    QHBoxLayout *handleRow = new QHBoxLayout;

    // Preset buttons arranged as a 3x2 grid (TL, TR / C / BL, BR)
    QGridLayout *presetGrid = new QGridLayout;
    presetGrid->setSpacing(2);
    auto makePreset = [&](const QString &label, const char *slot, int row, int col) {
        QPushButton *btn = new QPushButton(label, w);
        btn->setFixedSize(28, 22);
        connect(btn, SIGNAL(clicked()), this, slot);
        presetGrid->addWidget(btn, row, col);
    };
    makePreset("TL", SLOT(setHandleTopLeft()),     0, 0);
    makePreset("TR", SLOT(setHandleTopRight()),    0, 1);
    makePreset("C",  SLOT(setHandleCenter()),      1, 0);
    makePreset("BL", SLOT(setHandleBottomLeft()),  2, 0);
    makePreset("BR", SLOT(setHandleBottomRight()), 2, 1);

    handleRow->addLayout(presetGrid);

    // Clickable preview
    handleWidget_ = new BrushHandleWidget(this, w);
    handleRow->addWidget(handleWidget_);

    vbox->addLayout(handleRow);

    // ── Transform section ──────────────────────────────────────────────────
    QFrame *sep2 = new QFrame(w);
    sep2->setFrameShape(QFrame::HLine);
    vbox->addWidget(sep2);

    QGridLayout *xformGrid = new QGridLayout;
    xformGrid->setSpacing(2);
    auto makeXform = [&](const QString &label, const char *slot, int row, int col) {
        QPushButton *btn = new QPushButton(label, w);
        btn->setFixedSize(52, 22);
        connect(btn, SIGNAL(clicked()), this, slot);
        xformGrid->addWidget(btn, row, col);
    };
    makeXform("Flip H",    SLOT(brushFlipH()),       0, 0);
    makeXform("Flip V",    SLOT(brushFlipV()),       0, 1);
    makeXform("Rot CW",    SLOT(brushRotate90CW()),  0, 2);
    makeXform("Rot CCW",   SLOT(brushRotate90CCW()), 0, 3);
    makeXform("Double",    SLOT(brushDouble()),      1, 0);
    makeXform("Halve",     SLOT(brushHalve()),       1, 1);
    makeXform("Shear X+",  SLOT(brushShearXPlus()),  2, 0);
    makeXform("Shear X-",  SLOT(brushShearXMinus()), 2, 1);
    makeXform("Shear Y+",  SLOT(brushShearYPlus()),  2, 2);
    makeXform("Shear Y-",  SLOT(brushShearYMinus()), 2, 3);
    makeXform("Bend X+",   SLOT(brushBendXPlus()),   3, 0);
    makeXform("Bend X-",   SLOT(brushBendXMinus()),  3, 1);
    makeXform("Bend Y+",   SLOT(brushBendYPlus()),   3, 2);
    makeXform("Bend Y-",   SLOT(brushBendYMinus()),  3, 3);
    makeXform("Outline",   SLOT(brushOutline()),     4, 0);
    makeXform("Trim",      SLOT(brushTrim()),        4, 1);
    makeXform("Restore",   SLOT(brushRestore()),     4, 2);
    vbox->addLayout(xformGrid);

    return w;
}

// ── Transform helpers ─────────────────────────────────────────────────────

static Brush *currentBrush(Buffer *buf)
{
    return qobject_cast<Brush *>(buf ? buf->pen() : nullptr);
}

#define BRUSH_TRANSFORM(method) \
    Brush *brush = currentBrush(buffer_); \
    if (!brush) return; \
    if (!brush->hasOriginal()) brush->storeOriginal(); \
    brush->method; \
    if (handleWidget_) handleWidget_->update(); \
    if (dimensionsLabel_) dimensionsLabel_->setText(QString("%1 × %2").arg(brush->image().width()).arg(brush->image().height()));

void BrushTool::brushFlipH()       { BRUSH_TRANSFORM(flipHorizontal()) }
void BrushTool::brushFlipV()       { BRUSH_TRANSFORM(flipVertical()) }
void BrushTool::brushRotate90CW()  { BRUSH_TRANSFORM(rotate90CW()) }
void BrushTool::brushRotate90CCW() { BRUSH_TRANSFORM(rotate90CCW()) }
void BrushTool::brushDouble()      { BRUSH_TRANSFORM(doubleSize()) }
void BrushTool::brushHalve()       { BRUSH_TRANSFORM(halveSize()) }
void BrushTool::brushShearXPlus()  { BRUSH_TRANSFORM(shearX(0.25)) }
void BrushTool::brushShearXMinus() { BRUSH_TRANSFORM(shearX(-0.25)) }
void BrushTool::brushShearYPlus()  { BRUSH_TRANSFORM(shearY(0.25)) }
void BrushTool::brushShearYMinus() { BRUSH_TRANSFORM(shearY(-0.25)) }
void BrushTool::brushBendXPlus()   { BRUSH_TRANSFORM(bendX(4.0)) }
void BrushTool::brushBendXMinus()  { BRUSH_TRANSFORM(bendX(-4.0)) }
void BrushTool::brushBendYPlus()   { BRUSH_TRANSFORM(bendY(4.0)) }
void BrushTool::brushBendYMinus()  { BRUSH_TRANSFORM(bendY(-4.0)) }
void BrushTool::brushOutline()
{
    Brush *brush = currentBrush(buffer_);
    if (!brush || !buffer_) return;
    if (!brush->hasOriginal()) brush->storeOriginal();
    brush->outline(static_cast<int>(buffer_->paintColor()));
    if (handleWidget_) handleWidget_->update();
    if (dimensionsLabel_) dimensionsLabel_->setText(QString("%1 × %2").arg(brush->image().width()).arg(brush->image().height()));
}
void BrushTool::brushTrim() { BRUSH_TRANSFORM(trim()) }
void BrushTool::brushRestore()
{
    Brush *brush = currentBrush(buffer_);
    if (!brush) return;
    brush->restoreOriginal();
    if (handleWidget_) handleWidget_->update();
    if (dimensionsLabel_) dimensionsLabel_->setText(QString("%1 × %2").arg(brush->image().width()).arg(brush->image().height()));
}

void BrushTool::brushTileCut() { BRUSH_TRANSFORM(tileCut()) }

#undef BRUSH_TRANSFORM

void BrushTool::activate()
{
    if (buffer_->tool() == this) {
        mode_ = (mode_ == Rectangle) ? Freehand : Rectangle;
    }
    updateButton();
    Tool::activate();
}

void BrushTool::updateButton()
{
    if (mode_ == Freehand) {
        button_->setIcon(QIcon(":/carvebrush.png"));
        button_->setToolTip("Brush – Freehand selection [B]");
    } else {
        button_->setIcon(QIcon(":/cutbrush.png"));
        button_->setToolTip("Brush – Rectangle selection [B]");
    }
}

void BrushTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/cutbrush.png"));
    button_->setToolTip("Brush – Rectangle selection [B]");
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void BrushTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 10);
}
