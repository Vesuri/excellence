#include <QTimer>
#include <QToolButton>
#include "buffer.h"
#include "tool.h"

QList<Tool *> tools;

Tool::Tool(QObject *parent) : QObject(parent),
    mode_(Paint),
    buffer_(0)
{
    QTimer::singleShot(0, this, SLOT(registerTool()));
}

void Tool::setMode(const Mode &mode)
{
    mode_ = mode;
}

void Tool::setBuffer(Buffer *buffer)
{
    buffer_ = buffer;
}

void Tool::registerTool()
{
    button_ = new QToolButton;
    tools.append(this);
}

void Tool::activate()
{
    if (buffer_ != 0) {
        buffer_->setTool(this);
    }
}

void Tool::setCheckedIfEqual(Tool *tool)
{
    button_->setChecked(tool == this);
}
