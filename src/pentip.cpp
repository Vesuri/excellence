#include <QRect>
#include <QImage>
#include "buffer.h"
#include "pentip.h"

PenTip::PenTip(QObject *parent) : Pen(parent),
    paintColor_(1),
    eraseColor_(0),
    width_(1),
    height_(1),
    shape_(Circle)
{
}

void PenTip::setPaintColor(unsigned paintColor) { paintColor_ = paintColor; }
void PenTip::setEraseColor(unsigned eraseColor) { eraseColor_ = eraseColor; }

int PenTip::size() const { return width_; }
int PenTip::width() const { return width_; }
int PenTip::height() const { return height_; }
PenTip::Shape PenTip::shape() const { return shape_; }

void PenTip::setSize(int w, int h)
{
    w = qMax(1, w);
    h = qMax(1, h);
    if (w == width_ && h == height_)
        return;
    width_ = w;
    height_ = h;
    emit sizeChanged(width_, height_);
}

void PenTip::setShape(Shape shape) { shape_ = shape; }

bool PenTip::inTip(int dx, int dy, int hw, int hh) const
{
    Q_UNUSED(hh);
    switch (shape_) {
    case Square: return true;
    default:     return dx * dx + dy * dy <= hw * hw + hw / 2;
    }
}

void PenTip::applyColor(const QPoint &point, Buffer *buffer, unsigned color) const
{
    if (width_ == 1 && height_ == 1) {
        if (buffer->image().rect().contains(point))
            buffer->image().setPixel(point, color);
        return;
    }
    int hw = width_ / 2, hh = height_ / 2;
    QRect imageRect = buffer->image().rect();
    for (int dy = -hh; dy <= hh; dy++) {
        for (int dx = -hw; dx <= hw; dx++) {
            if (inTip(dx, dy, hw, hh)) {
                QPoint p(point.x() + dx, point.y() + dy);
                if (imageRect.contains(p))
                    buffer->image().setPixel(p, color);
            }
        }
    }
}

void PenTip::applyBrushMode(const QPoint &point, Buffer *buffer) const
{
    const QImage &stamp = buffer->brushStamp();
    if (stamp.isNull()) {
        applyColor(point, buffer, paintColor_);
        return;
    }
    const int bw = stamp.width(), bh = stamp.height();
    const int transparent = buffer->brushTransparentIndex();
    QRect imageRect = buffer->image().rect();
    auto doPixel = [&](const QPoint &p) {
        if (!imageRect.contains(p)) return;
        int bx = ((p.x() % bw) + bw) % bw;
        int by = ((p.y() % bh) + bh) % bh;
        int ci = stamp.pixelIndex(bx, by);
        if (ci == transparent) return;
        buffer->image().setPixel(p, static_cast<uint>(ci));
    };
    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x() + dx, point.y() + dy));
}

void PenTip::applyTipAt(const QPoint &point, Buffer *buffer,
                        Buffer::PaintMode mode, bool isErase,
                        unsigned paintColor, unsigned eraseColor) const
{
    auto doPixel = [&](const QPoint &p) {
        Pen::applyPixelMode(p, buffer, mode, isErase, paintColor, eraseColor);
    };
    if (width_ == 1 && height_ == 1) { doPixel(point); return; }
    int hw = width_ / 2, hh = height_ / 2;
    for (int dy = -hh; dy <= hh; dy++)
        for (int dx = -hw; dx <= hw; dx++)
            if (inTip(dx, dy, hw, hh))
                doPixel(QPoint(point.x()+dx, point.y()+dy));
}

static void collectMirrorPoints(const QPoint &point, Buffer *buffer, QPoint pts[4], bool active[4])
{
    int cx = buffer->mirrorCenterX(), cy = buffer->mirrorCenterY();
    pts[0] = point;                                              active[0] = true;
    pts[1] = QPoint(2*cx - point.x(), point.y());               active[1] = buffer->mirrorX();
    pts[2] = QPoint(point.x(), 2*cy - point.y());               active[2] = buffer->mirrorY();
    pts[3] = QPoint(2*cx - point.x(), 2*cy - point.y());        active[3] = buffer->mirrorX() && buffer->mirrorY();
}

QRect PenTip::paint(const QPoint &point, Buffer *buffer) const
{
    if (!buffer->segmentCheck(point)) return QRect();
    Buffer::PaintMode mode = buffer->paintMode();
    bool isErase = false;
    unsigned paintC = Pen::resolveDrawColor(buffer, mode, isErase, paintColor_);

    QPoint pts[4]; bool active[4];
    collectMirrorPoints(point, buffer, pts, active);

    QRect changed;
    for (int i = 0; i < 4; i++) {
        if (!active[i]) continue;
        if (mode == Buffer::BrushMode)
            applyBrushMode(pts[i], buffer);
        else
            applyTipAt(pts[i], buffer, mode, isErase, paintC, eraseColor_);
        changed = changed.united(rect(pts[i]));
    }
    return changed.intersected(buffer->image().rect());
}

QRect PenTip::paintAsColor(const QPoint &point, Buffer *buffer) const
{
    if (!buffer->segmentCheck(point)) return QRect();
    QPoint pts[4]; bool active[4];
    collectMirrorPoints(point, buffer, pts, active);
    QRect changed;
    for (int i = 0; i < 4; i++) {
        if (!active[i]) continue;
        applyTipAt(pts[i], buffer, Buffer::Color, false, paintColor_, eraseColor_);
        changed = changed.united(rect(pts[i]));
    }
    return changed.intersected(buffer->image().rect());
}

QRect PenTip::erase(const QPoint &point, Buffer *buffer) const
{
    if (!buffer->segmentCheck(point)) return QRect();
    Buffer::PaintMode mode = buffer->paintMode();
    bool isErase = true;
    unsigned paintC = Pen::resolveDrawColor(buffer, mode, isErase, paintColor_);

    QPoint pts[4]; bool active[4];
    collectMirrorPoints(point, buffer, pts, active);

    QRect changed;
    for (int i = 0; i < 4; i++) {
        if (!active[i]) continue;
        if (mode == Buffer::BrushMode)
            applyBrushMode(pts[i], buffer);
        else
            applyTipAt(pts[i], buffer, mode, isErase, paintC, eraseColor_);
        changed = changed.united(rect(pts[i]));
    }
    return changed.intersected(buffer->image().rect());
}

QRect PenTip::rect(const QPoint &point) const
{
    int hw = width_ / 2, hh = height_ / 2;
    return QRect(point.x() - hw, point.y() - hh, 2 * hw + 1, 2 * hh + 1);
}
