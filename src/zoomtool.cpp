#include <QGridLayout>
#include "buffer.h"
#include "zoomtool.h"

ZoomTool ZoomTool::instance;

ZoomTool::ZoomTool(QObject *parent) : Tool(parent)
{
}

void ZoomTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect ZoomTool::press(const QPoint &point)
{
    return mouseButton_ == Qt::LeftButton ? QRect(point, QSize()) : QRect();
}

QRect ZoomTool::move(const QPoint &)
{
    return QRect();
}

QRect ZoomTool::release(const QPoint &)
{
    return QRect();
}

void ZoomTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/zoom.png"));

    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void ZoomTool::activate()
{
    if (buffer_->tool() == this) {
        button_->setChecked(true);
    }

    Tool::activate();
}

void ZoomTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 3);
}

Tool::Type ZoomTool::type() const
{
    return Zoom;
}
