#include <QGridLayout>
#include <QImage>
#include <QRect>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "drawtool.h"
#include "gradientrange.h"
#include "gradientrenderer.h"

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
    disconnectToolChecked();
    Tool::setBuffer(buffer);
    connectToolChecked();
}

QRect DrawTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (rubberBand_.pending) {
        QPoint savedFrom = rubberBand_.from;
        QList<QPoint> savedPath = pendingPathPoints_;
        rubberBand_.clear();
        // Reset draw state so the subsequent release() produces a no-op shape
        // instead of splicing the confirmation click into the previous polygon.
        startingPoint = point;
        previousPoint = point;
        pathPoints_.clear();
        return applyPolygonGradient(savedPath, savedFrom, point);
    }

    startingPoint = point;
    previousPoint = point;
    lastStampedPoint = point;
    buffer_->setSmearDirection(QPoint(0, 0));
    buffer_->resetCycle();
    Pen *p = drawMode == FilledShape ? buffer_->toolPen() : buffer_->pen();
    drawnBounds_ = p->rect(point).intersected(buffer_->image().rect());
    pathPoints_.clear();
    pathPoints_.append(point);
    return draw(point);
}

QRect DrawTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton) {
        if (rubberBand_.pending)
            return rubberBand_.draw(point, buffer_->image());
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
        // pathPoints_ is empty after a rubber band confirmation (press cleared it but didn't
        // re-append the start point). Don't begin a new shape until the next press.
        if (pathPoints_.isEmpty())
            return QRect();
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
    if (rubberBand_.pending)
        return rubberBand_.hoverRect(point, buffer_->image().rect());
    Pen *p = drawMode == FilledShape ? buffer_->toolPen() : buffer_->pen();
    return p->rect(point);
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
        if (pathPoints_.size() >= 2) {
            Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
            Algorithms::line(point, startingPoint, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        }
        int fillColor = static_cast<int>(mouseButton_ == Qt::RightButton
                                         ? buffer_->eraseColor()
                                         : buffer_->paintColor());
        if (mouseButton_ == Qt::LeftButton && gradientFillActive() && activeGradientFillMode == FillLinear
            && pathPoints_.size() >= 3) {
            // Flat fill, then rubber band for direction.
            QRect polyBbox;
            for (const QPoint &p : pathPoints_) polyBbox = polyBbox.united(QRect(p, p));
            changedRect = changedRect.united(GradientRenderer::polygonFillScanline(
                buffer_->image(), pathPoints_, fillColor, false, nullptr,
                FillFlat, QPoint(), QPoint(), QRect()));
            pendingPathPoints_ = pathPoints_;
            rubberBand_.start(polyBbox.center());
        } else {
            changedRect = changedRect.united(polygonFill(fillColor, point));
        }
        return changedRect;
    }
}

QRect DrawTool::draw(const QPoint &point)
{
    Pen *p = drawMode == FilledShape ? buffer_->toolPen() : buffer_->pen();
    if (mouseButton_ == Qt::RightButton) {
        return p->erase(point, buffer_);
    }
    if (drawMode == FilledShape && gradientFillActive()) {
        return p->paintAsColor(point, buffer_);
    }
    return p->paint(point, buffer_);
}

void DrawTool::cancel()
{
    if (rubberBand_.pending) {
        rubberBand_.clear();
        pendingPathPoints_.clear();
        buffer_->clearHoverPreview();
        buffer_->undo();
    }
}

QString DrawTool::status() const
{
    return rubberBand_.status();
}

QRect DrawTool::applyPolygonGradient(const QList<QPoint> &path, const QPoint &gradFrom, const QPoint &gradTo)
{
    return GradientRenderer::applyPolygonGradient(buffer_->image(), path,
        static_cast<int>(buffer_->paintColor()), &gradientRanges[activeGradientRange],
        activeGradientFillMode, gradFrom, gradTo, conformFill);
}

QRect DrawTool::polygonFill(int fillColor, const QPoint &to)
{
    const bool useGradient = gradientFillActive();
    QImage &image = buffer_->image();

    QRect polyBbox;
    for (const QPoint &p : pathPoints_)
        polyBbox = polyBbox.united(QRect(p, p));

    auto [gradFrom, gradTo] = gradientEndpoints(polyBbox, startingPoint, to);

    if (useGradient)
        return GradientRenderer::applyPolygonGradient(image, pathPoints_, fillColor,
            &gradientRanges[activeGradientRange], activeGradientFillMode,
            gradFrom, gradTo, conformFill);
    return GradientRenderer::polygonFillScanline(image, pathPoints_, fillColor,
        false, nullptr, activeGradientFillMode, gradFrom, gradTo, QRect());
}

void DrawTool::registerTool()
{
    Tool::registerTool();

    setDrawMode(ConnectedDraw);
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
