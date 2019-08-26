#include <QImage>
#include <QRect>
#include <QPainter>
#include <QGridLayout>
#include "pen.h"
#include "buffer.h"
#include "undobuffer.h"
#include "algorithms.h"
#include "linetool.h"

LineTool LineTool::instance;

LineTool::LineTool(QObject *parent) : Tool(parent),
    undoBuffer(nullptr)
{
}

void LineTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect LineTool::press(const QPoint &point)
{
    startPoint = point;

    QRect rect = changes(point);
    undoBuffer = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
    return draw(point);
}

QRect LineTool::move(const QPoint &point)
{
    undoBuffer->apply(buffer_);
    delete undoBuffer;

    QRect changedRect;
    Algorithms::line(startPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->changes(point)); });
    undoBuffer = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
    Algorithms::line(startPoint, point, [this](const QPoint &point) { this->draw(point); });
    return changedRect;
}

QRect LineTool::release(const QPoint &point)
{
    undoBuffer->apply(buffer_);
    delete undoBuffer;
    undoBuffer = nullptr;

    QRect changedRect;
    Algorithms::line(startPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
    return changedRect;
}

QRect LineTool::changes(const QPoint &point)
{
    return buffer_->pen()->rect(point);
}

QRect LineTool::draw(const QPoint &point)
{
    if (mouseButton_ == Qt::LeftButton) {
        return buffer_->pen()->paint(point, buffer_);
    } else {
        return buffer_->pen()->erase(point, buffer_);
    }
}

void LineTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/line.png"));
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void LineTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 2);
}
