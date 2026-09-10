#include <QGridLayout>
#include "buffer.h"
#include "undotool.h"

UndoTool UndoTool::instance;

UndoTool::UndoTool(QObject *parent) : Tool(parent)
{
}

void UndoTool::setBuffer(Buffer *buffer)
{
    if (buffer_)
        disconnect(button_, &QToolButton::clicked, buffer_, &Buffer::undo);

    Tool::setBuffer(buffer);

    if (buffer_)
        connect(button_, &QToolButton::clicked, buffer_, &Buffer::undo);
}

void UndoTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/undo.png"));
    button_->setToolTip("Undo [U]\nShift+U: Redo  Alt+U: Undo All  Alt+Shift+U: Redo All  Ctrl+U: Clear Undo Buffer");
}

void UndoTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 11);
}
