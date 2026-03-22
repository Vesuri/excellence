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
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect DrawTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    startingPoint = point;
    previousPoint = point;
    lastStampedPoint = point;
    return draw(point);
}

QRect DrawTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton) {
        return draw(point);
    } else if (drawMode == Dotted) {
        QPoint delta = point - lastStampedPoint;
        QRect penRect = buffer_->pen()->rect(QPoint(0, 0));
        int threshold = qMax(1, qMax(penRect.width(), penRect.height()));
        if (delta.x() * delta.x() + delta.y() * delta.y() >= threshold * threshold) {
            lastStampedPoint = point;
            return draw(point);
        }
        return QRect();
    } else {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        previousPoint = point;
        return changedRect;
    }
}

QRect DrawTool::hover(const QPoint &point)
{
    return buffer_->pen()->rect(point);
}

QRect DrawTool::release(const QPoint &point)
{
    if (drawMode == Dotted) {
        return QRect();
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
    if (mouseButton_ == Qt::RightButton) {
        return buffer_->pen()->erase(point, buffer_);
    } else {
        return buffer_->pen()->paint(point, buffer_);
    }
}

void DrawTool::registerTool()
{
    Tool::registerTool();

    setDrawMode(Dotted);
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void DrawTool::activate()
{
    if (buffer_->tool() == this) {
        setDrawMode(static_cast<DrawMode>((drawMode + 1) % (FilledShape + 1)));
        button_->setChecked(true);
    }

    Tool::activate();
}

void DrawTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 2);
}
