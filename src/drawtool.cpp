#include <QImage>
#include <QRect>
#include <QGridLayout>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "drawtool.h"

DrawTool DrawTool::instance;

DrawTool::DrawTool(QObject *parent) : Tool(parent),
    drawMode(ConnectedDraw)
{
}

void DrawTool::setDrawMode(const DrawMode &drawMode)
{
    this->drawMode = drawMode;
}

QRect DrawTool::press(const QPoint &point)
{
    previousPoint = point;
    return draw(point);
}

QRect DrawTool::move(const QPoint &point)
{
    if (drawMode == Draw) {
        return draw(point);
    } else {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        previousPoint = point;
        return changedRect;
    }
}

QRect DrawTool::release(const QPoint &point)
{
    if (drawMode == Draw) {
        return draw(point);
    } else {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        previousPoint = point;
        return changedRect;
    }
}

QRect DrawTool::draw(const QPoint &point)
{
    if (mode_ == Paint) {
        return buffer_->pen()->paint(point, buffer_);
    } else {
        return buffer_->pen()->erase(point, buffer_);
    }
}

void DrawTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/connecteddraw.png"));
}

void DrawTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 1);
}
