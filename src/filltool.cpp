#include <QStack>
#include <QGridLayout>
#include "buffer.h"
#include "filltool.h"

FillTool FillTool::instance;

FillTool::FillTool(QObject *parent) : Tool(parent)
{
}

void FillTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr)
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    Tool::setBuffer(buffer);
    if (buffer_ != nullptr)
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
}

QRect FillTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    QImage &image = buffer_->image();
    if (!image.rect().contains(point))
        return QRect();

    int targetColor = image.pixelIndex(point);
    int fillColor = static_cast<int>(mouseButton_ == Qt::RightButton
                                     ? buffer_->eraseColor()
                                     : buffer_->paintColor());
    if (targetColor == fillColor)
        return QRect();

    QStack<QPoint> stack;
    stack.push(point);
    QRect changedRect;

    while (!stack.isEmpty()) {
        QPoint p = stack.pop();
        if (!image.rect().contains(p) || image.pixelIndex(p) != targetColor)
            continue;

        int x1 = p.x();
        while (x1 > 0 && image.pixelIndex(x1 - 1, p.y()) == targetColor) x1--;
        int x2 = p.x();
        while (x2 < image.width() - 1 && image.pixelIndex(x2 + 1, p.y()) == targetColor) x2++;

        for (int x = x1; x <= x2; x++)
            image.setPixel(x, p.y(), static_cast<uint>(fillColor));
        changedRect = changedRect.united(QRect(x1, p.y(), x2 - x1 + 1, 1));

        bool prevAbove = false, prevBelow = false;
        for (int x = x1; x <= x2; x++) {
            if (p.y() > 0) {
                bool above = image.pixelIndex(x, p.y() - 1) == targetColor;
                if (above && !prevAbove) stack.push(QPoint(x, p.y() - 1));
                prevAbove = above;
            }
            if (p.y() < image.height() - 1) {
                bool below = image.pixelIndex(x, p.y() + 1) == targetColor;
                if (below && !prevBelow) stack.push(QPoint(x, p.y() + 1));
                prevBelow = below;
            }
        }
    }

    return changedRect;
}

QRect FillTool::move(const QPoint &)
{
    return QRect();
}

QRect FillTool::release(const QPoint &)
{
    return QRect();
}

void FillTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/fill.png"));
    button_->setToolTip("Fill [F]");
    button_->setCheckable(true);
    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void FillTool::activate()
{
    Tool::activate();
}

void FillTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 9);
}
