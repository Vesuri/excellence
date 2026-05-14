#include <cmath>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QToolButton>
#include <QWidget>
#include "brush.h"
#include "buffer.h"
#include "pentip.h"
#include "pentiptool.h"
#include "undobuffer.h"

PenTipTool PenTipTool::instance;

PenTipTool::PenTipTool(QObject *parent) : Tool(parent),
    sizingMode_(SizingNone),
    lastCircleSize_(13),
    lastRectSize_(8),
    undoBuffer_(nullptr)
{
}

static QPixmap renderTip(PenTip::Shape shape, int w, int h)
{
    const int W = 14, H = 13;
    QPixmap pm(W, H);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    const int cx = W / 2, cy = H / 2;
    int hw = w / 2, hh = h / 2;
    for (int dy = -hh; dy <= hh; dy++) {
        for (int dx = -hw; dx <= hw; dx++) {
            bool inside = (shape == PenTip::Square)
                          ? true
                          : (dx * dx + dy * dy <= hw * hw + hw / 2);
            if (inside)
                p.fillRect(cx + dx, cy + dy, 1, 1, Qt::black);
        }
    }
    return pm;
}

// Returns the bounding rect of a preview shape centered on `center`.
QRect PenTipTool::centeredPreviewRect(const QPoint &center, int size)
{
    int r = size / 2;
    return QRect(center.x() - r, center.y() - r, size, size);
}

// Draws a filled circle of `size` centered at `center` using the paint color.
QRect PenTipTool::drawCirclePreview(const QPoint &center, int size)
{
    QImage &image = buffer_->image();
    const QRect imageRect = image.rect();
    const int color = static_cast<int>(buffer_->paintColor());
    const int r = size / 2;
    QRect changed;
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r + r / 2) {
                QPoint p(center.x() + dx, center.y() + dy);
                if (imageRect.contains(p)) {
                    image.setPixel(p, static_cast<uint>(color));
                    changed = changed.isNull() ? QRect(p, p) : changed.united(QRect(p, p));
                }
            }
        }
    }
    return changed;
}

// Draws a filled square of `size` centered at `center` using the paint color.
QRect PenTipTool::drawRectPreview(const QPoint &center, int size)
{
    QImage &image = buffer_->image();
    const int color = static_cast<int>(buffer_->paintColor());
    QRect box = centeredPreviewRect(center, size).intersected(image.rect());
    for (int y = box.top(); y <= box.bottom(); y++)
        for (int x = box.left(); x <= box.right(); x++)
            image.setPixel(x, y, static_cast<uint>(color));
    return box;
}

// ── Tool interface ────────────────────────────────────────────────────────────

QRect PenTipTool::hover(const QPoint &point)
{
    if (sizingMode_ == SizingNone) return QRect();
    int size = (sizingMode_ == SizingCircle) ? lastCircleSize_ : lastRectSize_;
    return centeredPreviewRect(point, size).intersected(buffer_->image().rect());
}

QRect PenTipTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (sizingMode_ == SizingNone) return QRect();
    startPoint_ = point;

    QRect saveRect = QRect(point, point).intersected(buffer_->image().rect());
    if (!saveRect.isEmpty())
        undoBuffer_ = new UndoBuffer(saveRect.topLeft(), buffer_->image().copy(saveRect), this);

    // Draw 1×1 dot as the initial anchor
    if (buffer_->image().rect().contains(point))
        buffer_->image().setPixel(point, buffer_->paintColor());

    return saveRect;
}

QRect PenTipTool::move(const QPoint &point)
{
    if (sizingMode_ == SizingNone) return QRect();

    if (mouseButton_ == Qt::NoButton) {
        // Hover preview: Buffer has already saved the hover rect; just draw.
        int size = (sizingMode_ == SizingCircle) ? lastCircleSize_ : lastRectSize_;
        if (sizingMode_ == SizingCircle)
            return drawCirclePreview(point, size);
        else
            return drawRectPreview(point, size);
    }

    // Drag in progress: restore previous preview, compute new size, draw new preview.
    if (undoBuffer_) {
        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;
    }

    int size;
    QPoint previewCenter;
    if (sizingMode_ == SizingCircle) {
        QPoint d = point - startPoint_;
        int r = static_cast<int>(std::round(std::sqrt(double(d.x()*d.x() + d.y()*d.y()))));
        size = qMax(1, 2 * r + 1);
        previewCenter = startPoint_;
    } else {
        QPoint d = point - startPoint_;
        size = qMax(1, qMax(qAbs(d.x()), qAbs(d.y())));
        // Rect anchored at startPoint_ (top-left corner); compute its centre.
        int r = size / 2;
        previewCenter = QPoint(startPoint_.x() + r, startPoint_.y() + r);
    }

    QRect saveRect = centeredPreviewRect(previewCenter, size).intersected(buffer_->image().rect());
    if (!saveRect.isEmpty())
        undoBuffer_ = new UndoBuffer(saveRect.topLeft(), buffer_->image().copy(saveRect), this);

    if (sizingMode_ == SizingCircle)
        return drawCirclePreview(previewCenter, size);
    else
        return drawRectPreview(previewCenter, size);
}

QRect PenTipTool::release(const QPoint &point)
{
    if (sizingMode_ == SizingNone) return QRect();

    // Compute the final size from the drag.
    int size;
    if (sizingMode_ == SizingCircle) {
        QPoint d = point - startPoint_;
        int r = static_cast<int>(std::round(std::sqrt(double(d.x()*d.x() + d.y()*d.y()))));
        size = qMax(1, 2 * r + 1);
        lastCircleSize_ = size;
    } else {
        QPoint d = point - startPoint_;
        size = qMax(1, qMax(qAbs(d.x()), qAbs(d.y())));
        lastRectSize_ = size;
    }

    // Restore canvas — no permanent pixels left behind.
    QRect changedRect;
    if (undoBuffer_) {
        changedRect = undoBuffer_->rect();
        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;
    }

    // Apply the new pen tip size and shape.
    PenTip *tip = qobject_cast<PenTip *>(buffer_->pen());
    if (tip) {
        tip->setSize(size, size);
        tip->setShape(sizingMode_ == SizingCircle ? PenTip::Circle : PenTip::Square);
        updateButtonIcon();
    }

    sizingMode_ = SizingNone;
    buffer_->setTool(tools.at(0));
    return changedRect;
}

void PenTipTool::cancel()
{
    if (!undoBuffer_) return;
    undoBuffer_->apply(buffer_);
    buffer_->notifyModified(undoBuffer_->rect());
    delete undoBuffer_;
    undoBuffer_ = nullptr;
    sizingMode_ = SizingNone;
    buffer_->setTool(tools.at(0));
}

void PenTipTool::activateSizing(SizingMode mode)
{
    if (!qobject_cast<PenTip *>(buffer_->pen()) && buffer_->penTip())
        buffer_->setPen(buffer_->penTip());
    sizingMode_ = mode;
    buffer_->setTool(this);
}

// ── Boilerplate ───────────────────────────────────────────────────────────────

void PenTipTool::updateButtonIcon()
{
    PenTip *tip = buffer_ ? qobject_cast<PenTip *>(buffer_->pen()) : nullptr;
    button_->setChecked(tip != nullptr);
    if (tip)
        button_->setIcon(QIcon(renderTip(tip->shape(), tip->width(), tip->height())));
    else
        button_->setIcon(QIcon(":/pentip.png"));
}

void PenTipTool::setBuffer(Buffer *buffer)
{
    if (buffer_)
        disconnect(buffer_, &Buffer::penChanged, this, &PenTipTool::onPenChanged);
    Tool::setBuffer(buffer);
    if (buffer_)
        connect(buffer_, &Buffer::penChanged, this, &PenTipTool::onPenChanged);
    updateButtonIcon();
}

void PenTipTool::onPenChanged(Pen *pen)
{
    button_->setChecked(qobject_cast<PenTip *>(pen) != nullptr);
    updateButtonIcon();
}

void PenTipTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/pentip.png"));
    button_->setToolTip("Pen Tip");
    button_->setCheckable(true);
    connect(button_, &QPushButton::clicked, this, &PenTipTool::activate);
}

void PenTipTool::activate()
{
    if (!buffer_) return;
    if (qobject_cast<PenTip *>(buffer_->pen())) {
        // Pen tip active — switch to brush if one exists
        if (!buffer_->brush()) {
            button_->setChecked(true);  // no brush: keep button pressed
            return;
        }
        buffer_->setPen(buffer_->brush());
        buffer_->setPaintMode(Buffer::BrushMode);
    } else {
        // Brush active — switch to pen tip
        if (!buffer_->penTip()) return;
        buffer_->setPen(buffer_->penTip());
        // DrawModeTool::onPenChanged handles the BrushMode → Normal transition
    }
}

QWidget *PenTipTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Pen Tip");

    QHBoxLayout *hbox = new QHBoxLayout(w);
    hbox->setSpacing(2);
    hbox->setContentsMargins(4, 4, 4, 4);

    // Fixed-size preset buttons
    struct Preset { PenTip::Shape shape; int pw; int ph; };
    const Preset presets[] = {
        { PenTip::Circle, 1,  1  },
        { PenTip::Circle, 5,  5  },
        { PenTip::Circle, 9,  9  },
        { PenTip::Circle, 13, 13 },
        { PenTip::Circle, 17, 17 },
        { PenTip::Square, 4,  4  },
        { PenTip::Square, 6,  6  },
        { PenTip::Square, 8,  8  },
        { PenTip::Square, 10, 10 },
        { PenTip::Square, 12, 12 },
        { PenTip::Square, 1,  13 },
        { PenTip::Square, 18, 1  },
    };

    for (const auto &preset : presets) {
        QToolButton *btn = new QToolButton(w);
        btn->setIcon(QIcon(renderTip(preset.shape, preset.pw, preset.ph)));
        PenTip::Shape shape = preset.shape;
        int pw = preset.pw, ph = preset.ph;
        connect(btn, &QPushButton::clicked, [this, shape, pw, ph]() {
            PenTip *tip = qobject_cast<PenTip *>(buffer_->pen());
            if (!tip) {
                tip = buffer_->penTip();
                if (!tip) return;
                buffer_->setPen(tip);
            }
            tip->setSize(pw, ph);
            tip->setShape(shape);
            updateButtonIcon();
        });
        hbox->addWidget(btn);
    }

    // Separator before the interactive sizers
    QFrame *sep = new QFrame(w);
    sep->setFrameShape(QFrame::VLine);
    sep->setFrameShadow(QFrame::Sunken);
    hbox->addWidget(sep);

    // Interactive circle sizer button
    QToolButton *circleBtn = new QToolButton(w);
    circleBtn->setIcon(QIcon(renderTip(PenTip::Circle, 9, 9)));
    circleBtn->setToolTip("Size Circle Pen Tip – drag from centre on canvas");
    connect(circleBtn, &QPushButton::clicked, [this]() { activateSizing(SizingCircle); });
    hbox->addWidget(circleBtn);

    // Interactive rect sizer button
    QToolButton *rectBtn = new QToolButton(w);
    rectBtn->setIcon(QIcon(renderTip(PenTip::Square, 8, 8)));
    rectBtn->setToolTip("Size Rectangle Pen Tip – drag from bottom-right corner on canvas");
    connect(rectBtn, &QPushButton::clicked, [this]() { activateSizing(SizingRect); });
    hbox->addWidget(rectBtn);

    hbox->addStretch();
    return w;
}

void PenTipTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 9);
}
