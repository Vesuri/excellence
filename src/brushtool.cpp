#include <QImage>
#include <QRect>
#include <QGridLayout>
#include "brush.h"
#include "buffer.h"
#include "undobuffer.h"
#include "algorithms.h"
#include "brushtool.h"

BrushTool BrushTool::instance;

BrushTool::BrushTool(QObject *parent) : Tool(parent),
    undoBuffer(nullptr)
{
}

void BrushTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect BrushTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    startPoint = point;

    QRect rect = changes(point);
    undoBuffer = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
    return draw(point);
}

QRect BrushTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton) {
        return QRect();
    }

    undoBuffer->apply(buffer_);
    delete undoBuffer;

    QRect changedRect;
    Algorithms::rectangle(startPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->changes(point)); });
    undoBuffer = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
    Algorithms::rectangle(startPoint, point, [this](const QPoint &point) { this->draw(point); });
    return changedRect;
}

QRect BrushTool::release(const QPoint &point)
{
    QRect changedRect = undoBuffer->rect();
    undoBuffer->apply(buffer_);
    delete undoBuffer;
    undoBuffer = nullptr;

    QImage image = buffer_->image().copy(QRect(startPoint, point));
    buffer_->setPen(new Brush(image));

    if (mouseButton_ == Qt::RightButton) {
        Algorithms::fillRectangle(startPoint, point, [this](const QPoint &point) { this->draw(point); });
    }

    return changedRect;
}

QRect BrushTool::changes(const QPoint &point)
{
    return buffer_->toolPen()->rect(point);
}

QRect BrushTool::draw(const QPoint &point)
{
    if (mouseButton_ == Qt::LeftButton) {
        return buffer_->toolPen()->paint(point, buffer_);
    } else {
        return buffer_->toolPen()->erase(point, buffer_);
    }
}

void BrushTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/cutbrush.png"));
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void BrushTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 4);
}
