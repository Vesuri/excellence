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

QRect ClearTool::press(const QPoint &)
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

void ClearTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/clear.png"));
}

void ClearTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 0);
}
