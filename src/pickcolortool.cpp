#include <QGridLayout>
#include "buffer.h"
#include "pickcolortool.h"

PickColorTool PickColorTool::instance;

PickColorTool::PickColorTool(QObject *parent) : Tool(parent)
{
}

void PickColorTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr)
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    Tool::setBuffer(buffer);
    if (buffer_ != nullptr)
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
}

QRect PickColorTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    const QImage &image = buffer_->image();
    if (!image.rect().contains(point))
        return QRect();

    unsigned index = static_cast<unsigned>(image.pixelIndex(point));
    if (mouseButton_ == Qt::RightButton)
        buffer_->setEraseColor(index);
    else
        buffer_->setPaintColor(index);

    return QRect();
}

QRect PickColorTool::move(const QPoint &)
{
    return QRect();
}

QRect PickColorTool::release(const QPoint &)
{
    return QRect();
}

void PickColorTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/eyedropper.png"));
    button_->setToolTip("Pick Color [,]");
    button_->setCheckable(true);
    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void PickColorTool::activate()
{
    Tool::activate();
}

void PickColorTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 8);
}
