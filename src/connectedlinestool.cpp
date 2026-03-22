#include <QImage>
#include <QRect>
#include <QGridLayout>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "connectedlinestool.h"

ConnectedLinesTool ConnectedLinesTool::instance;

ConnectedLinesTool::ConnectedLinesTool(QObject *parent) : Tool(parent),
    active_(false)
{
}

void ConnectedLinesTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    active_ = false;
    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect ConnectedLinesTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (mouseButton_ == Qt::RightButton) {
        active_ = false;
        return QRect();
    }

    if (!active_) {
        active_ = true;
        lastPoint_ = point;
        return draw(point);
    }

    // active_: the segment will be committed on release
    return QRect();
}

QRect ConnectedLinesTool::move(const QPoint &point)
{
    if (mouseButton_ != Qt::NoButton || !active_) {
        return QRect();
    }

    // Rubber-band preview: Buffer::move() already saved the hover area, now draw on it
    QRect changedRect;
    Algorithms::line(lastPoint_, point, [this, &changedRect](const QPoint &p) {
        changedRect = changedRect.united(draw(p));
    });
    return changedRect;
}

QRect ConnectedLinesTool::release(const QPoint &point)
{
    if (!active_ || mouseButton_ != Qt::LeftButton) {
        return QRect();
    }

    QRect changedRect;
    Algorithms::line(lastPoint_, point, [this, &changedRect](const QPoint &p) {
        changedRect = changedRect.united(draw(p));
    });
    lastPoint_ = point;
    return changedRect;
}

QRect ConnectedLinesTool::hover(const QPoint &point)
{
    if (!active_) {
        return QRect();
    }
    return lineBoundingRect(lastPoint_, point);
}

QRect ConnectedLinesTool::draw(const QPoint &point)
{
    if (mouseButton_ == Qt::RightButton) {
        return buffer_->pen()->erase(point, buffer_);
    } else {
        return buffer_->pen()->paint(point, buffer_);
    }
}

QRect ConnectedLinesTool::lineBoundingRect(const QPoint &from, const QPoint &to) const
{
    QRect penRect = buffer_->pen()->rect(QPoint(0, 0));
    int penW = penRect.width();
    int penH = penRect.height();
    return QRect(from, to).normalized().adjusted(-penW, -penH, penW, penH);
}

void ConnectedLinesTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/connectedlines.png"));
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void ConnectedLinesTool::activate()
{
    active_ = false;
    Tool::activate();
}

void ConnectedLinesTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 5);
}
