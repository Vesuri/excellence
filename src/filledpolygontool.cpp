#include <QImage>
#include <QRect>
#include <QGridLayout>
#include <QStack>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "filledpolygontool.h"

FilledPolygonTool FilledPolygonTool::instance;

FilledPolygonTool::FilledPolygonTool(QObject *parent) : Tool(parent),
    active_(false)
{
}

void FilledPolygonTool::setBuffer(Buffer *buffer)
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

QRect FilledPolygonTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (mouseButton_ == Qt::RightButton) {
        if (!active_) {
            return QRect();
        }

        active_ = false;

        // Need at least two distinct vertices to close and fill
        if (lastPoint_ == firstPoint_) {
            vertices_.clear();
            return QRect();
        }

        QRect changedRect;

        // Close the polygon: draw line from lastPoint back to firstPoint
        Algorithms::line(lastPoint_, firstPoint_, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(paint(p));
        });

        // Flood-fill the enclosed area from the centroid
        changedRect = changedRect.united(floodFill(centroid()));

        vertices_.clear();
        return changedRect;
    }

    // Left button
    if (!active_) {
        active_ = true;
        firstPoint_ = point;
        lastPoint_ = point;
        vertices_.clear();
        vertices_.append(point);
        return paint(point);
    }

    // active_: segment commits on release
    return QRect();
}

QRect FilledPolygonTool::move(const QPoint &point)
{
    if (mouseButton_ != Qt::NoButton || !active_) {
        return QRect();
    }

    // Rubber-band preview; Buffer::move() saved the hover area already
    QRect changedRect;
    Algorithms::line(lastPoint_, point, [this, &changedRect](const QPoint &p) {
        changedRect = changedRect.united(paint(p));
    });
    return changedRect;
}

QRect FilledPolygonTool::release(const QPoint &point)
{
    if (!active_ || mouseButton_ != Qt::LeftButton) {
        return QRect();
    }

    QRect changedRect;
    Algorithms::line(lastPoint_, point, [this, &changedRect](const QPoint &p) {
        changedRect = changedRect.united(paint(p));
    });
    lastPoint_ = point;
    vertices_.append(point);
    return changedRect;
}

QRect FilledPolygonTool::hover(const QPoint &point)
{
    if (!active_) {
        return QRect();
    }
    return lineBoundingRect(lastPoint_, point);
}

QRect FilledPolygonTool::paint(const QPoint &point)
{
    return buffer_->pen()->paint(point, buffer_);
}

QRect FilledPolygonTool::lineBoundingRect(const QPoint &from, const QPoint &to) const
{
    QRect penRect = buffer_->pen()->rect(QPoint(0, 0));
    int penW = penRect.width();
    int penH = penRect.height();
    return QRect(from, to).normalized().adjusted(-penW, -penH, penW, penH);
}

QPoint FilledPolygonTool::centroid() const
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

QRect FilledPolygonTool::floodFill(const QPoint &seed)
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

        // Scan left
        int x1 = p.x();
        while (x1 > 0 && image.pixelIndex(x1 - 1, p.y()) == targetColor) {
            x1--;
        }

        // Scan right
        int x2 = p.x();
        while (x2 < image.width() - 1 && image.pixelIndex(x2 + 1, p.y()) == targetColor) {
            x2++;
        }

        // Fill the span
        for (int x = x1; x <= x2; x++) {
            image.setPixel(x, p.y(), static_cast<uint>(fillColor));
        }
        changedRect = changedRect.united(QRect(x1, p.y(), x2 - x1 + 1, 1));

        // Seed spans above and below, pushing one point per new contiguous run
        bool prevAbove = false, prevBelow = false;
        for (int x = x1; x <= x2; x++) {
            if (p.y() > 0) {
                bool above = image.pixelIndex(x, p.y() - 1) == targetColor;
                if (above && !prevAbove) {
                    stack.push(QPoint(x, p.y() - 1));
                }
                prevAbove = above;
            }
            if (p.y() < image.height() - 1) {
                bool below = image.pixelIndex(x, p.y() + 1) == targetColor;
                if (below && !prevBelow) {
                    stack.push(QPoint(x, p.y() + 1));
                }
                prevBelow = below;
            }
        }
    }

    return changedRect;
}

void FilledPolygonTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/filledlines.png"));
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void FilledPolygonTool::activate()
{
    active_ = false;
    vertices_.clear();
    Tool::activate();
}

void FilledPolygonTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 5);
}
