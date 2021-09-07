#include <QRect>
#include <QImage>
#include "buffer.h"
#include "brush.h"

Brush::Brush(const QImage &image, QObject *parent) : Pen(parent),
    image_(image)
{
}

QRect Brush::paint(const QPoint &point, Buffer *buffer) const
{
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            buffer->image().setPixel(point.x() + x, point.y() + y, static_cast<uint>(image_.pixelIndex(x, y)));
        }
    }

    return image_.rect().translated(point);
}

QRect Brush::erase(const QPoint &point, Buffer *buffer) const
{
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            buffer->image().setPixel(point.x() + x, point.y() + y, 0);
        }
    }

    return image_.rect().translated(point);
}

QRect Brush::rect(const QPoint &point) const
{
    return image_.rect().translated(point);
}
