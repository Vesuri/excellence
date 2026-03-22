#include <QRect>
#include <QImage>
#include "buffer.h"
#include "brush.h"

Brush::Brush(const QImage &image, int transparentIndex, QObject *parent) : Pen(parent),
    image_(image),
    transparentIndex_(transparentIndex),
    handleOffset_(image.width() / 2, image.height() / 2)
{
}

QPoint Brush::handleOffset() const
{
    return handleOffset_;
}

void Brush::setHandleOffset(const QPoint &offset)
{
    handleOffset_ = offset;
}

QRect Brush::paint(const QPoint &point, Buffer *buffer) const
{
    QPoint origin = point - handleOffset_;
    QRect imageRect = buffer->image().rect();
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            int idx = image_.pixelIndex(x, y);
            if (idx == transparentIndex_)
                continue;
            QPoint p(origin.x() + x, origin.y() + y);
            if (imageRect.contains(p))
                buffer->image().setPixel(p, static_cast<uint>(idx));
        }
    }

    return image_.rect().translated(origin).intersected(imageRect);
}

QRect Brush::erase(const QPoint &point, Buffer *buffer) const
{
    QPoint origin = point - handleOffset_;
    QRect imageRect = buffer->image().rect();
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            if (image_.pixelIndex(x, y) == transparentIndex_)
                continue;
            QPoint p(origin.x() + x, origin.y() + y);
            if (imageRect.contains(p))
                buffer->image().setPixel(p, 0);
        }
    }

    return image_.rect().translated(origin).intersected(imageRect);
}

QRect Brush::rect(const QPoint &point) const
{
    return image_.rect().translated(point - handleOffset_);
}

const QImage &Brush::image() const
{
    return image_;
}
