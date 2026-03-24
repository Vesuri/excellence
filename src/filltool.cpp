#include <QStack>
#include <QVector>
#include <QGridLayout>
#include "buffer.h"
#include "filltool.h"
#include "gradientrange.h"
#include "gradientrenderer.h"

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

QRect FillTool::flatFill(const QPoint &seed, int fillColor)
{
    QImage &image = buffer_->image();
    if (!image.rect().contains(seed))
        return QRect();

    int targetColor = image.pixelIndex(seed);
    if (targetColor == fillColor)
        return QRect();

    QStack<QPoint> stack;
    stack.push(seed);
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

QRect FillTool::collectRegion(const QPoint &seed)
{
    QImage &image = buffer_->image();
    if (!image.rect().contains(seed))
        return QRect();

    visitedW_ = image.width();
    visitedH_ = image.height();
    visited_.fill(0, visitedW_ * visitedH_);

    int targetColor = image.pixelIndex(seed);

    QStack<QPoint> stack;
    stack.push(seed);
    QRect changedRect;

    while (!stack.isEmpty()) {
        QPoint p = stack.pop();
        if (!image.rect().contains(p) || image.pixelIndex(p) != targetColor)
            continue;
        if (visited_[p.y() * visitedW_ + p.x()])
            continue;

        int x1 = p.x();
        while (x1 > 0 && image.pixelIndex(x1 - 1, p.y()) == targetColor
               && !visited_[p.y() * visitedW_ + x1 - 1]) x1--;
        int x2 = p.x();
        while (x2 < visitedW_ - 1 && image.pixelIndex(x2 + 1, p.y()) == targetColor
               && !visited_[p.y() * visitedW_ + x2 + 1]) x2++;

        for (int x = x1; x <= x2; x++)
            visited_[p.y() * visitedW_ + x] = 1;
        changedRect = changedRect.united(QRect(x1, p.y(), x2 - x1 + 1, 1));

        bool prevAbove = false, prevBelow = false;
        for (int x = x1; x <= x2; x++) {
            if (p.y() > 0) {
                bool above = image.pixelIndex(x, p.y() - 1) == targetColor
                             && !visited_[(p.y() - 1) * visitedW_ + x];
                if (above && !prevAbove) stack.push(QPoint(x, p.y() - 1));
                prevAbove = above;
            }
            if (p.y() < visitedH_ - 1) {
                bool below = image.pixelIndex(x, p.y() + 1) == targetColor
                             && !visited_[(p.y() + 1) * visitedW_ + x];
                if (below && !prevBelow) stack.push(QPoint(x, p.y() + 1));
                prevBelow = below;
            }
        }
    }

    visitedRect_ = changedRect;
    return changedRect;
}

QRect FillTool::applyGradientFill(const QPoint &endPoint)
{
    if (visited_.isEmpty() || visitedRect_.isEmpty())
        return QRect();

    QImage &image = buffer_->image();
    const GradientRange *range = &gradientRanges[activeGradientRange];

    if (activeGradientFillMode == FillHighlight) {
        // BFS distance transform from boundary pixels inward.
        // Boundary = visited pixel with at least one non-visited 4-neighbor.
        QVector<int> dist(visitedW_ * visitedH_, -1);
        QVector<int> queue;
        queue.reserve(visitedRect_.width() * visitedRect_.height());

        for (int y = visitedRect_.top(); y <= visitedRect_.bottom(); y++) {
            for (int x = visitedRect_.left(); x <= visitedRect_.right(); x++) {
                if (!visited_[y * visitedW_ + x]) continue;
                bool boundary =
                    (x == 0               || !visited_[y * visitedW_ + x - 1]) ||
                    (x == visitedW_ - 1   || !visited_[y * visitedW_ + x + 1]) ||
                    (y == 0               || !visited_[(y - 1) * visitedW_ + x]) ||
                    (y == visitedH_ - 1   || !visited_[(y + 1) * visitedW_ + x]);
                if (boundary) {
                    dist[y * visitedW_ + x] = 0;
                    queue.append(y * visitedW_ + x);
                }
            }
        }

        static const int ddx[] = {-1, 1, 0, 0};
        static const int ddy[] = {0, 0, -1, 1};
        int maxDist = 0;
        for (int qi = 0; qi < queue.size(); qi++) {
            int idx = queue[qi];
            int cx = idx % visitedW_;
            int cy = idx / visitedW_;
            int d = dist[idx];
            for (int i = 0; i < 4; i++) {
                int nx = cx + ddx[i], ny = cy + ddy[i];
                if (nx < 0 || ny < 0 || nx >= visitedW_ || ny >= visitedH_) continue;
                int ni = ny * visitedW_ + nx;
                if (!visited_[ni] || dist[ni] >= 0) continue;
                dist[ni] = d + 1;
                if (dist[ni] > maxDist) maxDist = dist[ni];
                queue.append(ni);
            }
        }

        for (int y = visitedRect_.top(); y <= visitedRect_.bottom(); y++) {
            for (int x = visitedRect_.left(); x <= visitedRect_.right(); x++) {
                if (y < 0 || y >= visitedH_ || x < 0 || x >= visitedW_) continue;
                if (!visited_[y * visitedW_ + x]) continue;
                float t = maxDist > 0 ? 1.0f - float(dist[y * visitedW_ + x]) / float(maxDist) : 0.0f;
                int ci = GradientRenderer::colorIndex(t, x, y, range, image);
                image.setPixel(x, y, static_cast<uint>(ci));
            }
        }

        return visitedRect_;
    }

    for (int y = visitedRect_.top(); y <= visitedRect_.bottom(); y++) {
        for (int x = visitedRect_.left(); x <= visitedRect_.right(); x++) {
            if (y < 0 || y >= visitedH_ || x < 0 || x >= visitedW_) continue;
            if (!visited_[y * visitedW_ + x]) continue;
            float t = GradientRenderer::computeT(x, y, image.width(), image.height(),
                                                  activeGradientFillMode, startPoint_, endPoint);
            int ci = GradientRenderer::colorIndex(t, x, y, range, image);
            image.setPixel(x, y, static_cast<uint>(ci));
        }
    }

    return visitedRect_;
}

QRect FillTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (!buffer_) return QRect();
    gradientPending_ = false;

    // Right-click always flat erase; use flat fill when no gradient is active
    if (mouseButton_ == Qt::RightButton || !gradientFillActive()) {
        int fillColor = static_cast<int>(mouseButton_ == Qt::RightButton
                                         ? buffer_->eraseColor()
                                         : buffer_->paintColor());
        return flatFill(point, fillColor);
    }

    startPoint_ = point;

    // Horizontal/Vertical: apply immediately (no drag needed)
    if (activeGradientFillMode == FillHorizontal || activeGradientFillMode == FillVertical) {
        if (collectRegion(point).isEmpty()) return QRect();
        return applyGradientFill(point);
    }

    // Linear/Radial/Spherical: collect region now, apply on release
    if (!collectRegion(point).isEmpty())
        gradientPending_ = true;
    return QRect();
}

QRect FillTool::move(const QPoint &)
{
    return QRect();
}

QRect FillTool::release(const QPoint &point)
{
    if (!gradientPending_)
        return QRect();
    gradientPending_ = false;
    return applyGradientFill(point);
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
    layout->addWidget(button_, 0, 8);
}
