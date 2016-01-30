#include <QImage>
#include <QRect>
#include <QPainter>
#include "pen.h"
#include "undobuffer.h"
#include "algorithms.h"
#include "linetool.h"

LineTool::LineTool(QObject *parent) : Tool(parent),
    pen(0),
    undoBuffer(0)
{
}

void LineTool::setPen(Pen *pen)
{
    this->pen = pen;
}

QRect LineTool::press(const QPoint &point, QImage &image)
{
    startPoint = point;

    QRect rect = changes(point);
    undoBuffer = new UndoBuffer(rect.topLeft(), image.copy(rect), this);
    return draw(point, image);
}

QRect LineTool::move(const QPoint &point, QImage &image)
{
    undoBuffer->apply(image);
    delete undoBuffer;

    QRect changedRect;
    Algorithms::line(startPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->changes(point)); });
    undoBuffer = new UndoBuffer(changedRect.topLeft(), image.copy(changedRect), this);
    Algorithms::line(startPoint, point, [this, &image](const QPoint &point) { this->draw(point, image); });
    return changedRect;
}

QRect LineTool::release(const QPoint &point, QImage &image)
{
    undoBuffer->apply(image);
    delete undoBuffer;
    undoBuffer = 0;

    QRect changedRect;
    Algorithms::line(startPoint, point, [this, &image, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point, image)); });
    return changedRect;
}

QRect LineTool::changes(const QPoint &point)
{
    return pen->rect(point);
}

QRect LineTool::draw(const QPoint &point, QImage &image)
{
    if (mode_ == Paint) {
        return pen->paint(point, image);
    } else {
        return pen->erase(point, image);
    }
}
