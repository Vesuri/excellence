#include <QImage>
#include <QRect>
#include <QGridLayout>
#include "pen.h"
#include "buffer.h"
#include "undobuffer.h"
#include "algorithms.h"
#include "rectangletool.h"

RectangleTool RectangleTool::instance;
const char *RectangleTool::icons[] = {
    ":/rectangle.png",
    ":/filledrectangle.png"
};

RectangleTool::RectangleTool(QObject *parent) : Tool(parent),
    undoBuffer(nullptr)
{
}

void RectangleTool::setDrawMode(const DrawMode &drawMode)
{
    this->drawMode = drawMode;

    button_->setIcon(QIcon(icons[drawMode]));
}

void RectangleTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect RectangleTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    startPoint = point;

    QRect rect = changes(point);
    undoBuffer = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
    return draw(point);
}

QRect RectangleTool::move(const QPoint &point)
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

QRect RectangleTool::release(const QPoint &point)
{
    undoBuffer->apply(buffer_);
    delete undoBuffer;
    undoBuffer = nullptr;

    QRect changedRect;
    if (drawMode == Rectangle) {
        Algorithms::rectangle(startPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
    } else {
        Algorithms::fillRectangle(startPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
    }
    return changedRect;
}

QRect RectangleTool::changes(const QPoint &point)
{
    return buffer_->pen()->rect(point);
}

QRect RectangleTool::draw(const QPoint &point)
{
    if (mouseButton_ == Qt::LeftButton) {
        return buffer_->pen()->paint(point, buffer_);
    } else {
        return buffer_->pen()->erase(point, buffer_);
    }
}

void RectangleTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/rectangle.png"));
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void RectangleTool::activate()
{
    if (buffer_->tool() == this) {
        setDrawMode(static_cast<DrawMode>((drawMode + 1) % (FilledRectangle + 1)));
        button_->setChecked(true);
    }

    Tool::activate();
}

void RectangleTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 6);
}
