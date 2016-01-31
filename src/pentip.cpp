#include <QRect>
#include <QImage>
#include "buffer.h"
#include "pentip.h"

PenTip::PenTip(QObject *parent) : Pen(parent),
    paintColor_(1),
    eraseColor_(0)
{
}

void PenTip::setPaintColor(unsigned paintColor)
{
    this->paintColor_ = paintColor;
}

void PenTip::setEraseColor(unsigned eraseColor)
{
    this->eraseColor_ = eraseColor;
}

QRect PenTip::paint(const QPoint &point, Buffer *buffer) const
{
    buffer->image().setPixel(point, paintColor_);
    return QRect(point, point);
}

QRect PenTip::erase(const QPoint &point, Buffer *buffer) const
{
    buffer->image().setPixel(point, eraseColor_);
    return QRect(point, point);
}

QRect PenTip::rect(const QPoint &point) const
{
    return QRect(point, point);
}
