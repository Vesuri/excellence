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
    disconnectToolChecked();
    Tool::setBuffer(buffer);
    connectToolChecked();
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

QRect FillTool::applyGradientFill(const QPoint &gradFrom, const QPoint &gradTo)
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

    const bool hConform = conformFill && activeGradientFillMode == FillHorizontal;
    const bool vConform = conformFill && activeGradientFillMode == FillVertical;

    // For V conform: pre-scan to collect per-column y ranges.
    QVector<int> colY0, colY1;
    if (vConform) {
        const int vw = visitedRect_.width();
        colY0.fill(INT_MAX, vw);
        colY1.fill(INT_MIN, vw);
        for (int y = visitedRect_.top(); y <= visitedRect_.bottom(); y++) {
            for (int x = visitedRect_.left(); x <= visitedRect_.right(); x++) {
                if (y < 0 || y >= visitedH_ || x < 0 || x >= visitedW_) continue;
                if (!visited_[y * visitedW_ + x]) continue;
                const int xi = x - visitedRect_.left();
                colY0[xi] = qMin(colY0[xi], y);
                colY1[xi] = qMax(colY1[xi], y);
            }
        }
    }

    for (int y = visitedRect_.top(); y <= visitedRect_.bottom(); y++) {
        // For H conform: find this row's x extent.
        int rowX0 = visitedRect_.right() + 1, rowX1 = visitedRect_.left() - 1;
        if (hConform) {
            for (int x = visitedRect_.left(); x <= visitedRect_.right(); x++) {
                if (y < 0 || y >= visitedH_ || x < 0 || x >= visitedW_) continue;
                if (!visited_[y * visitedW_ + x]) continue;
                rowX0 = qMin(rowX0, x);
                rowX1 = qMax(rowX1, x);
            }
        }

        for (int x = visitedRect_.left(); x <= visitedRect_.right(); x++) {
            if (y < 0 || y >= visitedH_ || x < 0 || x >= visitedW_) continue;
            if (!visited_[y * visitedW_ + x]) continue;
            QRect pixConform;
            if (hConform && rowX0 <= rowX1)
                pixConform = QRect(rowX0, y, rowX1 - rowX0 + 1, 1);
            else if (vConform) {
                const int xi = x - visitedRect_.left();
                if (xi >= 0 && xi < colY0.size() && colY0[xi] <= colY1[xi])
                    pixConform = QRect(x, colY0[xi], 1, colY1[xi] - colY0[xi] + 1);
            }
            else if (conformFill)
                pixConform = visitedRect_;
            float t = GradientRenderer::computeT(x, y, activeGradientFillMode, gradFrom, gradTo, pixConform);
            int ci = GradientRenderer::colorIndex(t, x, y, range, image);
            image.setPixel(x, y, static_cast<uint>(ci));
        }
    }

    return visitedRect_;
}

QRect FillTool::hover(const QPoint &point)
{
    return rubberBand_.hoverRect(point, buffer_->image().rect());
}

QRect FillTool::move(const QPoint &point)
{
    if (mouseButton_ != Qt::NoButton)
        return QRect();
    return rubberBand_.draw(point, buffer_->image());
}

QString FillTool::status() const
{
    if (!rubberBand_.pending)
        return QString();
    if (activeGradientFillMode == FillLinear)
        return "click to set gradient angle";
    return "click to set gradient center";
}

void FillTool::cancel()
{
    if (rubberBand_.pending) {
        rubberBand_.clear();
        visited_.clear();
        buffer_->clearHoverPreview();
        buffer_->undo();
    }
}

QRect FillTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (!buffer_) return QRect();

    // Second click: confirm rubber band direction / center selection.
    if (rubberBand_.pending) {
        rubberBand_.clear();

        if (activeGradientFillMode == FillLinear) {
            // Any mouse button sets the angle and applies the gradient.
            return applyGradientFill(rubberBand_.from, point);
        } else {
            // Radial/Spherical/Highlight: any click picks the gradient center.
            float r = GradientRenderer::conformRadius(visitedRect_, point);
            return applyGradientFill(point, point + QPoint(qRound(r), 0));
        }
    }

    // Right-click always flat erase; use flat fill when no gradient is active.
    if (mouseButton_ == Qt::RightButton || !gradientFillActive()) {
        int fillColor = static_cast<int>(mouseButton_ == Qt::RightButton
                                         ? buffer_->eraseColor()
                                         : buffer_->paintColor());
        return flatFill(point, fillColor);
    }

    startPoint_ = point;

    // Horizontal/Vertical: apply immediately — no direction selection needed.
    if (activeGradientFillMode == FillHorizontal || activeGradientFillMode == FillVertical) {
        if (collectRegion(point).isEmpty()) return QRect();
        const QImage &image = buffer_->image();
        return applyGradientFill(QPoint(0, 0), QPoint(image.width() - 1, image.height() - 1));
    }

    if (collectRegion(point).isEmpty())
        return QRect();

    // Linear always shows a rubber band so the user can set the gradient direction.
    // Radial/Spherical/Highlight without Center: rubber band lets the user pick the center.
    bool needsRubberBand = (activeGradientFillMode == FillLinear)
                        || (gradientFillIsRadial(activeGradientFillMode) && !centerFill);
    if (needsRubberBand) {
        // For Linear: pre-fill the region with the foreground color so the user can see
        // which area will receive the gradient while choosing the direction.
        if (activeGradientFillMode == FillLinear) {
            const int fillColor = static_cast<int>(buffer_->paintColor());
            QImage &image = buffer_->image();
            for (int y = visitedRect_.top(); y <= visitedRect_.bottom(); y++) {
                for (int x = visitedRect_.left(); x <= visitedRect_.right(); x++) {
                    if (y >= 0 && y < visitedH_ && x >= 0 && x < visitedW_
                            && visited_[y * visitedW_ + x])
                        image.setPixel(x, y, static_cast<uint>(fillColor));
                }
            }
        }
        rubberBand_.start(visitedRect_.center());
        return activeGradientFillMode == FillLinear ? visitedRect_ : QRect();
    }

    // Radial/Spherical/Highlight with Center: apply immediately using bounding rect center.
    QPoint center = visitedRect_.center();
    float r = GradientRenderer::conformRadius(visitedRect_, center);
    return applyGradientFill(center, center + QPoint(qRound(r), 0));
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
    layout->addWidget(button_, 0, 8);
}
