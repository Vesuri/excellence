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
        Algorithms::line(previousPoint, point, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        Algorithms::line(point, startingPoint, [this, &changedRect](const QPoint &point) { changedRect = changedRect.united(this->draw(point)); });
        int fillColor = static_cast<int>(mouseButton_ == Qt::RightButton
                                         ? buffer_->eraseColor()
                                         : buffer_->paintColor());
        changedRect = changedRect.united(polygonFill(fillColor, point));
        return changedRect;
    }
}

QRect DrawTool::draw(const QPoint &point)
{
    Pen *p = drawMode == FilledShape ? buffer_->toolPen() : buffer_->pen();
    if (mouseButton_ == Qt::RightButton) {
        return p->erase(point, buffer_);
    } else {
        return p->paint(point, buffer_);
    }
}

QRect DrawTool::polygonFill(int fillColor, const QPoint &to)
{
    const bool useGradient = gradientFillActive();
    const GradientRange *range = useGradient ? &gradientRanges[activeGradientRange] : nullptr;
    bool hvMode = activeGradientFillMode == FillHorizontal || activeGradientFillMode == FillVertical;
    bool isRadial = gradientFillIsRadial(activeGradientFillMode);
    QImage &image = buffer_->image();

    QRect polyBbox;
    for (const QPoint &p : pathPoints_)
        polyBbox = polyBbox.united(QRect(p, p));

    QPoint gradFrom = hvMode ? QPoint(0, 0) : startingPoint;
    if (centerFill && isRadial)
        gradFrom = polyBbox.center();
    QPoint gradTo = hvMode ? QPoint(image.width() - 1, image.height() - 1) : to;
    QRect conformRect = conformFill ? polyBbox : QRect();
    return GradientRenderer::polygonFillScanline(image, pathPoints_, fillColor,
        useGradient, range, activeGradientFillMode, gradFrom, gradTo, conformRect);
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
