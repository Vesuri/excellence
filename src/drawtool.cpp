#include <algorithm>
#include <QGridLayout>
#include <QImage>
#include <QRect>
#include <QVector>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "drawtool.h"

DrawTool DrawTool::instance;
const char *DrawTool::icons[] = {
    ":/draw.png",
    ":/connecteddraw.png",
    ":/filledshape.png"
};

DrawTool::DrawTool(QObject *parent) : Tool(parent)
{
}

void DrawTool::setDrawMode(const DrawMode &drawMode)
{
    this->drawMode = drawMode;

    button_->setIcon(QIcon(icons[drawMode]));

    static const char *tips[] = {
        "Draw – Dotted [D]",
        "Draw – Connected [D]",
        "Draw – Filled Shape [D]"
    };
    button_->setToolTip(tips[drawMode]);
}

void DrawTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect DrawTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    startingPoint = point;
    previousPoint = point;
    lastStampedPoint = point;
    buffer_->setSmearDirection(QPoint(0, 0));
    buffer_->resetCycle();
    drawnBounds_ = buffer_->pen()->rect(point).intersected(buffer_->image().rect());
    pathPoints_.clear();
    pathPoints_.append(point);
    return draw(point);
}

QRect DrawTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton) {
        return draw(point);
    } else if (drawMode == Dotted) {
        QPoint delta = point - lastStampedPoint;
        constexpr int threshold = 1;
        if (delta.x() * delta.x() + delta.y() * delta.y() >= threshold * threshold) {
            buffer_->setSmearDirection(point - lastStampedPoint);
            lastStampedPoint = point;
            return draw(point);
        }
        return QRect();
    } else {
        QRect changedRect;
        buffer_->setSmearDirection(point - previousPoint);
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        previousPoint = point;
        drawnBounds_ = drawnBounds_.united(changedRect);
        if (drawMode == FilledShape)
            pathPoints_.append(point);
        return changedRect;
    }
}

QRect DrawTool::hover(const QPoint &point)
{
    return buffer_->pen()->rect(point);
}

QRect DrawTool::release(const QPoint &point)
{
    if (drawMode == Dotted) {
        return QRect();
    } else if (drawMode == ConnectedDraw) {
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        return changedRect;
    } else {
        pathPoints_.append(point);
        QRect changedRect;
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        Algorithms::line(point, startingPoint, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        int fillColor = static_cast<int>(mouseButton_ == Qt::RightButton
                                         ? buffer_->eraseColor()
                                         : buffer_->paintColor());
        changedRect = changedRect.united(polygonFill(fillColor));
        return changedRect;
    }
}

QRect DrawTool::draw(const QPoint &point)
{
    if (mouseButton_ == Qt::RightButton) {
        return buffer_->pen()->erase(point, buffer_);
    } else {
        return buffer_->pen()->paint(point, buffer_);
    }
}

QRect DrawTool::polygonFill(int fillColor)
{
    if (pathPoints_.size() < 3)
        return QRect();

    QImage &image = buffer_->image();
    const QRect imageRect = image.rect();
    const int n = pathPoints_.size();

    int minY = imageRect.bottom(), maxY = imageRect.top();
    for (const QPoint &v : pathPoints_) {
        minY = qMin(minY, v.y());
        maxY = qMax(maxY, v.y());
    }
    minY = qMax(minY, imageRect.top());
    maxY = qMin(maxY, imageRect.bottom());

    QRect changedRect;
    for (int y = minY; y <= maxY; y++) {
        QVector<int> xs;
        for (int i = 0; i < n; i++) {
            const QPoint &p1 = pathPoints_[i];
            const QPoint &p2 = pathPoints_[(i + 1) % n];
            if ((p1.y() <= y && p2.y() > y) || (p2.y() <= y && p1.y() > y)) {
                int x = p1.x() + (y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y());
                xs.append(x);
            }
        }
        std::sort(xs.begin(), xs.end());
        for (int i = 0; i + 1 < xs.size(); i += 2) {
            int x1 = qMax(xs[i], imageRect.left());
            int x2 = qMin(xs[i + 1], imageRect.right());
            for (int x = x1; x <= x2; x++)
                image.setPixel(x, y, static_cast<uint>(fillColor));
            if (x1 <= x2)
                changedRect = changedRect.united(QRect(x1, y, x2 - x1 + 1, 1));
        }
    }
    return changedRect;
}

void DrawTool::registerTool()
{
    Tool::registerTool();

    setDrawMode(Dotted);
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void DrawTool::activate()
{
    if (buffer_->tool() == this) {
        setDrawMode(static_cast<DrawMode>((drawMode + 1) % (FilledShape + 1)));
        button_->setChecked(true);
    }

    Tool::activate();
}


void DrawTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 2);
}
