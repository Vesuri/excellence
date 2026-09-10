#include <QGridLayout>
#include "buffer.h"
#include "fixbackgroundtool.h"

FixBackgroundTool FixBackgroundTool::instance;

FixBackgroundTool::FixBackgroundTool(QObject *parent) : Tool(parent)
{
}

void FixBackgroundTool::setBuffer(Buffer *buffer)
{
    if (buffer_)
        disconnect(buffer_, &Buffer::fixBackgroundChanged,
                   this, &FixBackgroundTool::syncButtonState);
    Tool::setBuffer(buffer);
    if (buffer_) {
        connect(buffer_, &Buffer::fixBackgroundChanged,
                this, &FixBackgroundTool::syncButtonState);
        syncButtonState();
    }
}

void FixBackgroundTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/fixbackground.png"));
    button_->setToolTip("Fix Background");
    button_->setCheckable(true);
    connect(button_, &QToolButton::clicked, this, &FixBackgroundTool::activate);
}

void FixBackgroundTool::activate()
{
    if (buffer_)
        buffer_->setFixBackgroundLocked(!buffer_->fixBackgroundLocked());
}

void FixBackgroundTool::syncButtonState()
{
    if (!buffer_)
        return;
    bool locked = buffer_->fixBackgroundLocked();
    button_->setChecked(locked);
    button_->setIcon(QIcon(locked ? ":/fixedbackground.png" : ":/fixbackground.png"));
}

void FixBackgroundTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 3);
}
