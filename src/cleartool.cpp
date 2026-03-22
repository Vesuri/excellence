#include <QGridLayout>
#include "buffer.h"
#include "cleartool.h"

ClearTool ClearTool::instance;

ClearTool::ClearTool(QObject *parent) : Tool(parent)
{
}

void ClearTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(button_, SIGNAL(clicked(bool)), buffer_, SLOT(clear()));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(button_, SIGNAL(clicked(bool)), buffer_, SLOT(clear()));
    }
}

QRect ClearTool::press(const QPoint &, const Qt::KeyboardModifiers &)
{
    return QRect();
}

QRect ClearTool::move(const QPoint &)
{
    return QRect();
}

QRect ClearTool::release(const QPoint &)
{
    return QRect();
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

    // Override default right-click (options widget) with erase-color clear
    disconnect(button_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(toggleOptionsWidget()));
    connect(button_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(clearWithEraseColor()));
}

void ClearTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 0);
}
