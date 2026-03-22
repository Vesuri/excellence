#include <QRect>
#include <QImage>
#include "buffer.h"
#include "pentip.h"

PenTip::PenTip(QObject *parent) : Pen(parent),
    paintColor_(1),
    eraseColor_(0),
    size_(1)
{
}

void PenTip::setPaintColor(unsigned paintColor) { paintColor_ = paintColor; }
void PenTip::setEraseColor(unsigned eraseColor) { eraseColor_ = eraseColor; }

int PenTip::size() const { return size_; }

void PenTip::setSize(int size)
{
    // Clamp to supported sizes: 1, 3, 5, 7
    if (size <= 1)      size_ = 1;
    else if (size <= 3) size_ = 3;
    else if (size <= 5) size_ = 5;
    else                size_ = 7;
}

void PenTip::applyColor(const QPoint &point, Buffer *buffer, unsigned color) const
{
    if (size_ == 1) {
        if (buffer->image().rect().contains(point))
            buffer->image().setPixel(point, color);
        return;
    }
    int r = size_ / 2;
    QRect imageRect = buffer->image().rect();
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r + r / 2) {
                QPoint p(point.x() + dx, point.y() + dy);
                if (imageRect.contains(p))
                    buffer->image().setPixel(p, color);
            }
        }
    }
}

QRect PenTip::paint(const QPoint &point, Buffer *buffer) const
{
    applyColor(point, buffer, paintColor_);
    return rect(point).intersected(buffer->image().rect());
}

QRect PenTip::erase(const QPoint &point, Buffer *buffer) const
{
    applyColor(point, buffer, eraseColor_);
    return rect(point).intersected(buffer->image().rect());
}

QRect PenTip::rect(const QPoint &point) const
{
    if (size_ == 1)
        return QRect(point, point);
    int r = size_ / 2;
    return QRect(point.x() - r, point.y() - r, size_, size_);
}
