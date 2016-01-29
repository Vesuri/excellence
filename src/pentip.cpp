#include <QRect>
#include <QImage>
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

QRect PenTip::paint(const QPoint &point, QImage &image)
{
    image.setPixel(point, paintColor_);
    return QRect(point, point);
}

QRect PenTip::erase(const QPoint &point, QImage &image)
{
    image.setPixel(point, eraseColor_);
    return QRect(point, point);
}
