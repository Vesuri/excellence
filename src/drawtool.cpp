#include <QImage>
#include <QRect>
#include "pen.h"
#include "algorithms.h"
#include "drawtool.h"

DrawTool::DrawTool(QObject *parent) : Tool(parent),
    drawMode(ConnectedDraw),
    pen(0)
{
}

void DrawTool::setDrawMode(const DrawMode &drawMode)
{
    this->drawMode = drawMode;
}

void DrawTool::setPen(Pen *pen)
{
    this->pen = pen;
}

QRect DrawTool::press(const QPoint &point, QImage &image)
{
    previousPoint = point;
    return draw(point, image);
}

QRect DrawTool::move(const QPoint &point, QImage &image)
{
    if (drawMode == Draw) {
        return draw(point, image);
    } else {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &image, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point, image)); });
        previousPoint = point;
        return changedRect;
    }
}

QRect DrawTool::release(const QPoint &point, QImage &image)
{
    if (drawMode == Draw) {
        return draw(point, image);
    } else {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &image, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point, image)); });
        previousPoint = point;
        return changedRect;
    }
}

QRect DrawTool::draw(const QPoint &point, QImage &image)
{
    if (mode_ == Paint) {
        return pen->paint(point, image);
    } else {
        return pen->erase(point, image);
    }
}
