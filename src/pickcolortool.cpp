#include <QGridLayout>
#include "buffer.h"
#include "pickcolortool.h"

PickColorTool PickColorTool::instance;

PickColorTool::PickColorTool(QObject *parent) : Tool(parent),
    oneShotTarget_(None),
    previousTool_(nullptr)
{
}

void PickColorTool::setBuffer(Buffer *buffer)
{
    disconnectToolChecked();
    Tool::setBuffer(buffer);
    connectToolChecked();
}

QRect PickColorTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    const QImage &image = buffer_->image();
    if (!image.rect().contains(point))
        return QRect();

    unsigned index = static_cast<unsigned>(image.pixelIndex(point));
    if (oneShotTarget_ == Foreground || (oneShotTarget_ == None && mouseButton_ != Qt::RightButton))
        buffer_->setPaintColor(index);
    else
        buffer_->setEraseColor(index);

    if (oneShotTarget_ != None) {
        Tool *prev = previousTool_;
        oneShotTarget_ = None;
        previousTool_ = nullptr;
        if (prev)
            buffer_->setTool(prev);
    }

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
    button_->setToolTip("Pick Color [,]");
    button_->setCheckable(true);
    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void PickColorTool::activate()
{
    oneShotTarget_ = None;
    previousTool_ = nullptr;
    Tool::activate();
}

void PickColorTool::addButtonToGridLayout(QGridLayout *)
{
    // No toolbar button — activated via foreground/background color rectangles
}

void PickColorTool::activateOneShotForeground(Tool *previousTool)
{
    oneShotTarget_ = Foreground;
    previousTool_ = previousTool;
    Tool::activate();
}

void PickColorTool::activateOneShotBackground(Tool *previousTool)
{
    oneShotTarget_ = Background;
    previousTool_ = previousTool;
    Tool::activate();
}
