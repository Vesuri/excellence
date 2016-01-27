#include <QImage>
#include <QRect>
#include "algorithms.h"
#include "drawtool.h"

DrawTool::DrawTool(QObject *parent) : Tool(parent),
    mode(ConnectedDraw)
{

}

QRect DrawTool::press(const QPoint &point, QImage &image)
{
    image.setPixel(point, 1);
    previousPoint = point;
    return QRect(point, point);
}

QRect DrawTool::move(const QPoint &point, QImage &image)
{
    if (mode == Draw) {
        image.setPixel(point, 1);
        return QRect(point, point);
    } else {
        Algorithms::drawLine(image, previousPoint, point);
        QPoint oldPoint = previousPoint;
        previousPoint = point;
        return QRect(oldPoint, point);
    }
}

QRect DrawTool::release(const QPoint &point, QImage &image)
{
    if (mode == Draw) {
        image.setPixel(point, 1);
        return QRect(point, point);
    } else {
        Algorithms::drawLine(image, previousPoint, point);
        QPoint oldPoint = previousPoint;
        previousPoint = point;
        return QRect(oldPoint, point);
    }
}

void DrawTool::setMode(const Mode &mode)
{
    this->mode = mode;
}
