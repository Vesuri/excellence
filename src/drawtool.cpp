#include <QImage>
#include <QRect>
#include <QGridLayout>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "drawtool.h"

DrawTool DrawTool::instance;
const char *DrawTool::icons[] = {
    ":/draw.png",
    ":/connecteddraw.png",
    ":/filledshape.png"
};

DrawTool::DrawTool(QObject *parent) : Tool(parent)
{
}

void DrawTool::setDrawMode(const DrawMode &drawMode)
{
    this->drawMode = drawMode;

    button_->setIcon(QIcon(icons[drawMode]));
}

void DrawTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != 0) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != 0) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect DrawTool::press(const QPoint &point)
{
    startingPoint = point;
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
    } else if (drawMode == ConnectedDraw) {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        return changedRect;
    } else {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        Algorithms::line(point, startingPoint, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
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

    setDrawMode(ConnectedDraw);
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void DrawTool::activate()
{
    if (buffer_->tool() == this) {
        setDrawMode((DrawMode)(((int)drawMode + 1) % ((int)FilledShape + 1)));
        button_->setChecked(true);
    }

    Tool::activate();
}

void DrawTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 1);
}
