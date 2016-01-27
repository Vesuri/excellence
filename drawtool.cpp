#include <QImage>
#include <QRect>
#include "drawtool.h"

DrawTool::DrawTool(QObject *parent) : QObject(parent)
{

}

QRect DrawTool::press(const QPoint &point, QImage &image)
{
    image.setPixel(point, 1);
    return QRect(point, point);
}

QRect DrawTool::move(const QPoint &point, QImage &image)
{
    image.setPixel(point, 1);
    return QRect(point, point);
}

QRect DrawTool::release(const QPoint &point, QImage &image)
{
    image.setPixel(point, 1);
    return QRect(point, point);
}
