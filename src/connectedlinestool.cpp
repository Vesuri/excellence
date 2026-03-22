#include <QImage>
#include <QRect>
#include <QGridLayout>
#include <QStack>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "connectedlinestool.h"

ConnectedLinesTool ConnectedLinesTool::instance;

ConnectedLinesTool::ConnectedLinesTool(QObject *parent) : Tool(parent),
    drawMode_(Lines), active_(false)
{
}

void ConnectedLinesTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    active_ = false;
    vertices_.clear();
    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect ConnectedLinesTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (mouseButton_ == Qt::RightButton) {
        if (drawMode_ == FilledPolygon && active_) {
            active_ = false;

            if (lastPoint_ == firstPoint_) {
                vertices_.clear();
                return QRect();
            }

            QRect changedRect;
            Algorithms::line(lastPoint_, firstPoint_, [this, &changedRect](const QPoint &p) {
                changedRect = changedRect.united(paint(p));
            });
            changedRect = changedRect.united(floodFill(centroid()));
            vertices_.clear();
            return changedRect;
        }
        active_ = false;
        vertices_.clear();
        return QRect();
    }

    // Left button
    if (!active_) {
        active_ = true;
        lastPoint_ = point;
        if (drawMode_ == FilledPolygon) {
            firstPoint_ = point;
            vertices_.clear();
            vertices_.append(point);
            return paint(point);
        }
        return draw(point);
    }

    // Active: segment commits on release
    return QRect();
}

QRect ConnectedLinesTool::move(const QPoint &point)
{
    if (mouseButton_ != Qt::NoButton || !active_) {
        return QRect();
    }

    QRect changedRect;
    Algorithms::line(lastPoint_, point, [this, &changedRect](const QPoint &p) {
        changedRect = changedRect.united(drawMode_ == Lines ? draw(p) : paint(p));
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
        changedRect = changedRect.united(drawMode_ == Lines ? draw(p) : paint(p));
    });
    lastPoint_ = point;
    if (drawMode_ == FilledPolygon) {
        vertices_.append(point);
    }
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

QRect ConnectedLinesTool::paint(const QPoint &point)
{
    return buffer_->pen()->paint(point, buffer_);
}

QRect ConnectedLinesTool::lineBoundingRect(const QPoint &from, const QPoint &to) const
{
    QRect penRect = buffer_->pen()->rect(QPoint(0, 0));
    int penW = penRect.width();
    int penH = penRect.height();
    return QRect(from, to).normalized().adjusted(-penW, -penH, penW, penH);
}

QPoint ConnectedLinesTool::centroid() const
{
    if (vertices_.isEmpty()) {
        return firstPoint_;
    }
    int sumX = 0, sumY = 0;
    for (const QPoint &v : vertices_) {
        sumX += v.x();
        sumY += v.y();
    }
    return QPoint(sumX / vertices_.size(), sumY / vertices_.size());
}

QRect ConnectedLinesTool::floodFill(const QPoint &seed)
{
    QImage &image = buffer_->image();
    if (!image.rect().contains(seed)) {
        return QRect();
    }

    int targetColor = image.pixelIndex(seed);
    int fillColor = static_cast<int>(buffer_->paintColor());
    if (targetColor == fillColor) {
        return QRect();
    }

    QStack<QPoint> stack;
    stack.push(seed);
    QRect changedRect;

    while (!stack.isEmpty()) {
        QPoint p = stack.pop();
        if (!image.rect().contains(p) || image.pixelIndex(p) != targetColor) {
            continue;
        }

        int x1 = p.x();
        while (x1 > 0 && image.pixelIndex(x1 - 1, p.y()) == targetColor) x1--;
        int x2 = p.x();
        while (x2 < image.width() - 1 && image.pixelIndex(x2 + 1, p.y()) == targetColor) x2++;

        for (int x = x1; x <= x2; x++) {
            image.setPixel(x, p.y(), static_cast<uint>(fillColor));
        }
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

void ConnectedLinesTool::setDrawMode(DrawMode mode)
{
    drawMode_ = mode;
    button_->setIcon(QIcon(mode == Lines ? ":/connectedlines.png" : ":/filledlines.png"));
    button_->setToolTip(mode == Lines ? "Connected Lines [W]" : "Connected Lines – Filled Polygon [W]");
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
    vertices_.clear();
    if (buffer_->tool() == this) {
        setDrawMode(drawMode_ == Lines ? FilledPolygon : Lines);
        button_->setChecked(true);
    }
    Tool::activate();
}

void ConnectedLinesTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 4);
}
