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
    // Buffer tool has no options widget — right-click also opens the buffers window.
    disconnect(button_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(toggleOptionsWidget()));
    connect(button_, &QToolButton::customContextMenuRequested, this, &BufferTool::activate);
}

void BufferTool::activate()
{
    emit showBuffersRequested();
    // Do NOT call Tool::activate() — Buffer tool does not change the active drawing tool.
}

void BufferTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 0);
}
