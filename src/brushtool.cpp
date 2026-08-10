#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
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
#include "ui_brushtool.h"

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

BrushHandleWidget::BrushHandleWidget(QWidget *parent)
    : QWidget(parent), tool_(nullptr)
{
    setFixedSize(80, 80);
    setCursor(Qt::CrossCursor);
}

void BrushHandleWidget::setTool(BrushTool *tool)
{
    tool_ = tool;
}

void BrushHandleWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    update();
}

void BrushHandleWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (!tool_) { p.fillRect(rect(), QColor(80, 80, 80)); return; }
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
    if (!tool_) return;
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
    undoBuffer_(nullptr)
{
    for (int i = 0; i < WellCount; i++)
        wellButtons_[i] = nullptr;
}

void BrushTool::cancel()
{
    if (distortMode_ != NoDistort) { distortCancel(); return; }
    if (!undoBuffer_) return;
    undoBuffer_->apply(buffer_);
    buffer_->notifyModified(buffer_->image().rect());
    delete undoBuffer_;
    undoBuffer_ = nullptr;
    polygon_.clear();
}

QString BrushTool::name() const
{
    return mode_ == Freehand ? "Carve Brush" : "Cut Brush";
}

void BrushTool::setBuffer(Buffer *buffer)
{
    disconnectToolChecked();
    if (buffer_) {
        disconnect(buffer_, &Buffer::penChanged, this, &BrushTool::refreshBrushDisplay);
        disconnect(buffer_, &Buffer::penModified, this, &BrushTool::refreshBrushDisplay);
    }
    Tool::setBuffer(buffer);
    if (buffer_) {
        connect(buffer_, &Buffer::penChanged, this, &BrushTool::refreshBrushDisplay);
        connect(buffer_, &Buffer::penModified, this, &BrushTool::refreshBrushDisplay);
    }
    connectToolChecked();
    refreshBrushDisplay();
}

QRect BrushTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (distortMode_ != NoDistort)
        return distortPress(point);

    if (mode_ == Freehand) {
        polygon_.clear();
        polygon_ << point;
        prevPoint_ = point;
        undoBuffer_ = new UndoBuffer(QPoint(), buffer_->image().copy(), this);
        return buffer_->toolPen()->paint(point, buffer_);
    }

    startPoint_ = point;
    currentPoint_ = point;
    QRect rect = changes(point);
    undoBuffer_ = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
    return draw(point);
}

QRect BrushTool::move(const QPoint &point)
{
    if (distortMode_ != NoDistort)
        return distortMove(point);

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

    // No rectangle preview — guides show the selection area instead.
    // Restore the initial press dot but keep undoBuffer_ for release().
    currentPoint_ = point;
    if (undoBuffer_)
        undoBuffer_->apply(buffer_);
    return QRect();
}

QRect BrushTool::release(const QPoint &point)
{
    if (distortMode_ != NoDistort)
        return distortRelease(point);

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

        Brush *brush = new Brush(areaImage, eraseIdx, buffer_);
        if (autoBg_) brush->detectBackground();
        if (tileCut_ && buffer_->gridEnabled()) brush->tileCut();
        buffer_->setPen(brush);
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
    Brush *brush = new Brush(image, static_cast<int>(buffer_->eraseColor()));
    if (autoBg_) brush->detectBackground();
    if (tileCut_ && buffer_->gridEnabled()) brush->tileCut();
    buffer_->setPen(brush);
    buffer_->setPaintMode(Buffer::BrushMode);

    if (mouseButton_ == Qt::RightButton)
        Algorithms::fillRectangle(startPoint_, point, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(draw(p));
        });

    // setTool() synchronously emits toolChanged, which BufferView::onToolChanged()
    // handles by calling buffer_->move() to refresh the new tool's hover preview.
    // That must happen after the erase above, otherwise the preview's undo snapshot
    // captures the pre-erase pixels and restores them over the just-cleared area on
    // the next mouse move.
    buffer_->setTool(tools.at(0));

    return changedRect;
}

QString BrushTool::status() const
{
    if (distortMode_ != NoDistort || mode_ != Rectangle || mouseButton_ == Qt::NoButton || !undoBuffer_)
        return QString();
    QRect r = QRect(startPoint_, currentPoint_).normalized();
    return QString("%1 × %2").arg(r.width()).arg(r.height());
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
    if (ui_)
        ui_->handleWidget->update();
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
    ui_ = new Ui::BrushToolOptions;
    ui_->setupUi(w);

    ui_->handleWidget->setTool(this);

    BrushWellButton *wellPtrs[] = {
        ui_->well0, ui_->well1, ui_->well2, ui_->well3,
        ui_->well4, ui_->well5, ui_->well6, ui_->well7
    };
    QSignalMapper *clickMapper = new QSignalMapper(w);
    QSignalMapper *ctrlMapper  = new QSignalMapper(w);
    connect(clickMapper, SIGNAL(mapped(int)), this, SLOT(wellClicked(int)));
    connect(ctrlMapper,  SIGNAL(mapped(int)), this, SLOT(wellCtrlClicked(int)));
    for (int i = 0; i < WellCount; i++) {
        wellButtons_[i] = wellPtrs[i];
        if (!wells_[i].isNull())
            wellButtons_[i]->store(wells_[i]);
        clickMapper->setMapping(wellButtons_[i], i);
        ctrlMapper->setMapping(wellButtons_[i], i);
        connect(wellButtons_[i], SIGNAL(clicked()),     clickMapper, SLOT(map()));
        connect(wellButtons_[i], SIGNAL(ctrlClicked()), ctrlMapper,  SLOT(map()));
    }

    ui_->tileCutCheck->setChecked(tileCut_);
    ui_->autoBgCheck->setChecked(autoBg_);
    connect(ui_->tileCutCheck, &QCheckBox::toggled, this, &BrushTool::setTileCut);
    connect(ui_->autoBgCheck,  &QCheckBox::toggled, this, &BrushTool::setAutoBg);

    ui_->handleTL->setIcon(QIcon(":/topleft.png"));
    ui_->handleTR->setIcon(QIcon(":/topright.png"));
    ui_->handleBL->setIcon(QIcon(":/bottomleft.png"));
    ui_->handleBR->setIcon(QIcon(":/bottomright.png"));

    connect(ui_->handleTL,     SIGNAL(clicked()), this, SLOT(setHandleTopLeft()));
    connect(ui_->handleTR,     SIGNAL(clicked()), this, SLOT(setHandleTopRight()));
    connect(ui_->handleCenter, SIGNAL(clicked()), this, SLOT(setHandleCenter()));
    connect(ui_->handleBL,     SIGNAL(clicked()), this, SLOT(setHandleBottomLeft()));
    connect(ui_->handleBR,     SIGNAL(clicked()), this, SLOT(setHandleBottomRight()));

    refreshBrushDisplay();
    return w;
}

// ── Transform helpers ─────────────────────────────────────────────────────

static Brush *currentBrush(Buffer *buf)
{
    return qobject_cast<Brush *>(buf ? buf->pen() : nullptr);
}

// Brush::image()-mutating methods all emit imageChanged(), which Buffer relays
// as penModified(); setPen() itself emits penChanged(). BrushTool listens to
// both (see setBuffer()) and funnels them here, so no call site needs to
// manually refresh the preview/size display.
void BrushTool::refreshBrushDisplay()
{
    if (!ui_) return;
    ui_->handleWidget->update();
    Brush *brush = currentBrush(buffer_);
    if (brush) {
        ui_->dimensionsLabel->setText(QString("%1 × %2").arg(brush->image().width()).arg(brush->image().height()));
        ui_->dimensionsLabel->setVisible(true);
    } else {
        ui_->dimensionsLabel->setVisible(false);
    }
}

#define BRUSH_TRANSFORM(method) \
    Brush *brush = currentBrush(buffer_); \
    if (!brush) return; \
    brush->storeOriginal(); \
    brush->method;

void BrushTool::brushOutline()
{
    Brush *brush = currentBrush(buffer_);
    if (!brush || !buffer_) return;
    brush->storeOriginal();
    brush->outline(static_cast<int>(buffer_->paintColor()));
}
void BrushTool::brushTrim() { BRUSH_TRANSFORM(trim()) }
void BrushTool::brushRestore()
{
    Brush *brush = currentBrush(buffer_);
    if (!brush) return;
    brush->restoreOriginal();
}

#undef BRUSH_TRANSFORM

void BrushTool::setTileCut(bool enabled) { tileCut_ = enabled; }
void BrushTool::setAutoBg(bool enabled)  { autoBg_ = enabled; }

// ── Interactive Shear/Bend (Brush menu) ──────────────────────────────────────
//
// Triggered from the Brush menu; drags in the buffer view control the amount
// while the previous tool is temporarily suspended. The distorted shape is
// live-previewed by stamping the brush at the drag-start point and reverting
// on every subsequent move, exactly like the tool-managed drag-preview pattern
// used by LineTool/CurveTool -- except the "permanent result" here is a
// mutation of the Brush's own image, not a canvas paint, so release() reverts
// the preview stamp too and leaves the canvas untouched.

static void applyDistort(Brush *brush, BrushTool::DistortMode mode, const QPoint &delta)
{
    brush->restoreOriginal();
    switch (mode) {
    case BrushTool::ShearX: brush->shearX(qBound(-2.0, delta.x() / 100.0, 2.0)); break;
    case BrushTool::ShearY: brush->shearY(qBound(-2.0, delta.y() / 100.0, 2.0)); break;
    case BrushTool::BendX:  brush->bendX(qBound(-1.0, delta.x() / 100.0, 1.0)); break;
    case BrushTool::BendY:  brush->bendY(qBound(-1.0, delta.y() / 100.0, 1.0)); break;
    default: break;
    }
}

void BrushTool::startDistort(DistortMode mode)
{
    Brush *brush = currentBrush(buffer_);
    if (!brush || !buffer_) return;
    brush->storeOriginal();
    distortMode_ = mode;
    distortPreviousTool_ = buffer_->tool();
    buffer_->setTool(this);
}

void BrushTool::startShearX() { startDistort(ShearX); }
void BrushTool::startShearY() { startDistort(ShearY); }
void BrushTool::startBendX()  { startDistort(BendX); }
void BrushTool::startBendY()  { startDistort(BendY); }

QRect BrushTool::distortPress(const QPoint &point)
{
    distortStartPoint_ = point;
    return QRect();
}

QRect BrushTool::distortMove(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton)
        return QRect();

    Brush *brush = currentBrush(buffer_);
    if (!brush) return QRect();
    applyDistort(brush, distortMode_, point - distortStartPoint_);

    if (undoBuffer_) {
        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;
    }
    QRect stampRect = brush->rect(distortStartPoint_).intersected(buffer_->image().rect());
    QRect changed = stampRect;
    if (!stampRect.isEmpty()) {
        undoBuffer_ = new UndoBuffer(stampRect.topLeft(), buffer_->image().copy(stampRect), this);
        changed = changed.united(brush->paint(distortStartPoint_, buffer_));
    }
    return changed;
}

QRect BrushTool::distortRelease(const QPoint &point)
{
    Brush *brush = currentBrush(buffer_);
    if (brush)
        applyDistort(brush, distortMode_, point - distortStartPoint_);

    QRect changed;
    if (undoBuffer_) {
        changed = undoBuffer_->rect();
        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;
    }

    distortMode_ = NoDistort;
    Tool *prev = distortPreviousTool_;
    distortPreviousTool_ = nullptr;
    if (prev) buffer_->setTool(prev);
    return changed;
}

void BrushTool::distortCancel()
{
    if (undoBuffer_) {
        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;
    }
    Brush *brush = currentBrush(buffer_);
    if (brush) brush->restoreOriginal();
    distortMode_ = NoDistort;
    Tool *prev = distortPreviousTool_;
    distortPreviousTool_ = nullptr;
    if (prev) buffer_->setTool(prev);
}

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
