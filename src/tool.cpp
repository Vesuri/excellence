#include <QTimer>
#include <QToolButton>
#include "buffer.h"
#include "tool.h"

QList<Tool *> tools;

Tool::Tool(QObject *parent) : QObject(parent),
    mouseButton_(Qt::NoButton),
    buffer_(nullptr)
{
    QTimer::singleShot(0, this, SLOT(registerTool()));
}

void Tool::setMouseButton(const Qt::MouseButton &mouseButton)
{
    mouseButton_ = mouseButton;
}

void Tool::setBuffer(Buffer *buffer)
{
    buffer_ = buffer;
}

Tool::Type Tool::type() const
{
    return Modify;
}

void Tool::registerTool()
{
    button_ = new QToolButton;
    tools.append(this);
}

void Tool::activate()
{
    buffer_->setTool(this);
}

void Tool::setCheckedIfEqual(Tool *tool)
{
    button_->setChecked(tool == this);
}
