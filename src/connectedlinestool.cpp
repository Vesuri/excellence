#include <QImage>
#include <QRect>
#include <QGridLayout>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "undobuffer.h"
#include "connectedlinestool.h"

ConnectedLinesTool ConnectedLinesTool::instance;

ConnectedLinesTool::ConnectedLinesTool(QObject *parent) : Tool(parent),
    drawMode_(Lines), active_(false), dragUndoBuffer_(nullptr)
{
}

void ConnectedLinesTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    active_ = false;
    vertices_.clear();
    delete dragUndoBuffer_;
    dragUndoBuffer_ = nullptr;
    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect ConnectedLinesTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (mouseButton_ == Qt::RightButton) {
        delete dragUndoBuffer_;
        dragUndoBuffer_ = nullptr;
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
    if (!active_)
        return QRect();

    if (mouseButton_ != Qt::NoButton) {
        // Drag preview: restore previous, save new area, draw preview line
        if (dragUndoBuffer_ != nullptr) {
            dragUndoBuffer_->apply(buffer_);
            delete dragUndoBuffer_;
            dragUndoBuffer_ = nullptr;
        }
        QRect area = lineBoundingRect(lastPoint_, point).intersected(buffer_->image().rect());
        if (!area.isEmpty()) {
            dragUndoBuffer_ = new UndoBuffer(area.topLeft(), buffer_->image().copy(area), this);
            Algorithms::line(lastPoint_, point, [this](const QPoint &p) {
                drawMode_ == Lines ? draw(p) : paint(p);
            });
        }
        return area;
    }

    // No button held: hover preview (buffer's moveUndo mechanism handles restore)
    QRect changedRect;
    Algorithms::line(lastPoint_, point, [this, &changedRect](const QPoint &p) {
        changedRect = changedRect.united(drawMode_ == Lines ? draw(p) : paint(p));
    });
    return changedRect;
}

QRect ConnectedLinesTool::release(const QPoint &point)
{
    if (dragUndoBuffer_ != nullptr) {
        dragUndoBuffer_->apply(buffer_);
        delete dragUndoBuffer_;
        dragUndoBuffer_ = nullptr;
    }

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
    int fillColor = static_cast<int>(buffer_->paintColor());

    // Try seed first, then search outward if it lands on an outline pixel
    static const int offsets[][2] = {
        {0,0},{1,0},{-1,0},{0,1},{0,-1},{2,0},{-2,0},{0,2},{0,-2},
        {1,1},{-1,1},{1,-1},{-1,-1},{3,0},{-3,0},{0,3},{0,-3}
    };
    for (auto &off : offsets) {
        QPoint candidate(seed.x() + off[0], seed.y() + off[1]);
        if (!image.rect().contains(candidate))
            continue;
        int targetColor = image.pixelIndex(candidate);
        if (targetColor == fillColor)
            continue;
        return Algorithms::floodFill(image, candidate, targetColor, fillColor);
    }
    return QRect();
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
    delete dragUndoBuffer_;
    dragUndoBuffer_ = nullptr;
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
