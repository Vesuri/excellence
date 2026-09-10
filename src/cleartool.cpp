#include <QGridLayout>
#include "buffer.h"
#include "cleartool.h"

ClearTool ClearTool::instance;

ClearTool::ClearTool(QObject *parent) : Tool(parent)
{
}

void ClearTool::setBuffer(Buffer *buffer)
{
    if (buffer_)
        disconnect(button_, &QToolButton::clicked, buffer_, &Buffer::clear);

    Tool::setBuffer(buffer);

    if (buffer_)
        connect(button_, &QToolButton::clicked, buffer_, &Buffer::clear);
}

void ClearTool::clearWithEraseColor()
{
    if (buffer_)
        buffer_->clearWithColor(buffer_->eraseColor());
}

void ClearTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/clear.png"));
    button_->setToolTip("CLR – Clear to color 0 [K]\nRight-click: clear to background color [Shift+K]");

    disconnect(button_, &QToolButton::customContextMenuRequested, this, &Tool::toggleOptionsWidget);
    connect(button_, &QToolButton::customContextMenuRequested,
            this, &ClearTool::clearWithEraseColor);
}

void ClearTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 0);
}
