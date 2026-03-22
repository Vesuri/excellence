#include <QRect>
#include <QImage>
#include "buffer.h"
#include "brush.h"

Brush::Brush(const QImage &image, int transparentIndex, QObject *parent) : Pen(parent),
    image_(image),
    transparentIndex_(transparentIndex)
{
}

QRect Brush::paint(const QPoint &point, Buffer *buffer) const
{
    QRect imageRect = buffer->image().rect();
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            int idx = image_.pixelIndex(x, y);
            if (idx == transparentIndex_)
                continue;
            QPoint p(point.x() + x, point.y() + y);
            if (imageRect.contains(p))
                buffer->image().setPixel(p, static_cast<uint>(idx));
        }
    }

    return image_.rect().translated(point).intersected(imageRect);
}

QRect Brush::erase(const QPoint &point, Buffer *buffer) const
{
    QRect imageRect = buffer->image().rect();
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            if (image_.pixelIndex(x, y) == transparentIndex_)
                continue;
            QPoint p(point.x() + x, point.y() + y);
            if (imageRect.contains(p))
                buffer->image().setPixel(p, 0);
        }
    }

    return image_.rect().translated(point).intersected(imageRect);
}

QRect Brush::rect(const QPoint &point) const
{
    return image_.rect().translated(point);
}

const QImage &Brush::image() const
{
    return image_;
}
