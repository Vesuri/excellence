#include <QGridLayout>
#include "buffer.h"
#include "undotool.h"

UndoTool UndoTool::instance;

UndoTool::UndoTool(QObject *parent) : Tool(parent)
{
}

void UndoTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(button_, SIGNAL(clicked(bool)), buffer_, SLOT(undo()));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(button_, SIGNAL(clicked(bool)), buffer_, SLOT(undo()));
    }
}

QRect UndoTool::press(const QPoint &, const Qt::KeyboardModifiers &)
{
    return QRect();
}

QRect UndoTool::move(const QPoint &)
{
    return QRect();
}

QRect UndoTool::release(const QPoint &)
{
    return QRect();
}

void UndoTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/undo.png"));
}

void UndoTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 5);
}
