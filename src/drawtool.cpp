#include <QImage>
#include <QRect>
#include "pen.h"
#include "algorithms.h"
#include "drawtool.h"

DrawTool::DrawTool(QObject *parent) : Tool(parent),
    mode(ConnectedDraw),
    pen(0)
{
}

void DrawTool::setMode(const Mode &mode)
{
    this->mode = mode;
}

void DrawTool::setPen(Pen *pen)
{
    this->pen = pen;
}

QRect DrawTool::press(const QPoint &point, QImage &image)
{
    previousPoint = point;
    return pen->paint(point, image);
}

QRect DrawTool::move(const QPoint &point, QImage &image)
{
    if (mode == Draw) {
        return pen->paint(point, image);
    } else {
        Pen *drawPen = pen;
        QRect changedRect;
        Algorithms::line(previousPoint, point, [drawPen, &image, &changedRect](const QPoint &point) { changedRect = changedRect.united(drawPen->paint(point, image)); });
        previousPoint = point;
        return changedRect;
    }
}

QRect DrawTool::release(const QPoint &point, QImage &image)
{
    if (mode == Draw) {
        return pen->paint(point, image);
    } else {
        Pen *drawPen = pen;
        QRect changedRect;
        Algorithms::line(previousPoint, point, [drawPen, &image, &changedRect](const QPoint &point) { changedRect = changedRect.united(drawPen->paint(point, image)); });
        previousPoint = point;
        return changedRect;
    }
}
