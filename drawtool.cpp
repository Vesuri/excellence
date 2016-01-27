#include <QImage>
#include <QRect>
#include "algorithms.h"
#include "drawtool.h"

DrawTool::DrawTool(QObject *parent) : Tool(parent),
    mode(ConnectedDraw),
    color(1)
{

}

QRect DrawTool::press(const QPoint &point, QImage &image)
{
    image.setPixel(point, color);
    previousPoint = point;
    return QRect(point, point);
}

QRect DrawTool::move(const QPoint &point, QImage &image)
{
    if (mode == Draw) {
        image.setPixel(point, color);
        return QRect(point, point);
    } else {
        int drawColor = color;
        Algorithms::line(previousPoint, point, [&image, drawColor](const QPoint &point) { image.setPixel(point, drawColor); });
        QPoint oldPoint = previousPoint;
        previousPoint = point;
        return QRect(oldPoint, point);
    }
}

QRect DrawTool::release(const QPoint &point, QImage &image)
{
    if (mode == Draw) {
        image.setPixel(point, color);
        return QRect(point, point);
    } else {
        int drawColor = color;
        Algorithms::line(previousPoint, point, [&image, drawColor](const QPoint &point) { image.setPixel(point, drawColor); });
        QPoint oldPoint = previousPoint;
        previousPoint = point;
        return QRect(oldPoint, point);
    }
}

void DrawTool::setMode(const Mode &mode)
{
    this->mode = mode;
}

void DrawTool::setColor(int color)
{
    this->color = color;
}
