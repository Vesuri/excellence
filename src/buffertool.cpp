#include <QGridLayout>
#include <QIcon>
#include <QToolButton>
#include "buffertool.h"

BufferTool BufferTool::instance;

BufferTool::BufferTool(QObject *parent) : Tool(parent)
{
}

void BufferTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/buffer.png"));
    button_->setToolTip("Show Buffers");
    connect(button_, &QToolButton::clicked, this, &BufferTool::activate);
    disconnect(button_, &QToolButton::customContextMenuRequested, this, &Tool::toggleOptionsWidget);
    connect(button_, &QToolButton::customContextMenuRequested, this, &BufferTool::activate);
}

void BufferTool::activate()
{
    emit showBuffersRequested();
}

void BufferTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 0);
}
