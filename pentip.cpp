#include <QRect>
#include <QImage>
#include "pentip.h"

PenTip::PenTip(QObject *parent) : Pen(parent),
    color(1)
{
}

void PenTip::setColor(int color)
{
    this->color = color;
}

QRect PenTip::draw(const QPoint &point, QImage &image)
{
    image.setPixel(point, color);
    return QRect(point, point);
}
