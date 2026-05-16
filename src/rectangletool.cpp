#include <QImage>
#include <QRect>
#include <QGridLayout>
#include <QRadioButton>
#include <QVBoxLayout>
#include "ui_rectangletool.h"
#include "pen.h"
#include "buffer.h"
#include "undobuffer.h"
#include "algorithms.h"
#include "rectangletool.h"
#include "gradientrange.h"
#include "gradientrenderer.h"

RectangleTool RectangleTool::instance;
const char *RectangleTool::icons[] = {
    ":/rectangle.png",
    ":/filledrectangle.png"
};

RectangleTool::RectangleTool(QObject *parent) : Tool(parent),
    anchorMode_(CornerToCorner), undoBuffer(nullptr)
{
}

void RectangleTool::setDrawMode(const DrawMode &drawMode)
{
    this->drawMode = drawMode;

    button_->setIcon(QIcon(icons[drawMode]));

    static const char *tips[] = {
        "Rectangle – Outline [R]  Shift+R: filled\nRight-click: anchor options",
        "Rectangle – Filled [R]\nRight-click: anchor options"
    };
    button_->setToolTip(tips[drawMode]);
}

QString RectangleTool::name() const
{
    return drawMode == FilledRectangle ? "Filled Rectangle" : "Rectangle";
}

void RectangleTool::setBuffer(Buffer *buffer)
{
    disconnectToolChecked();
    Tool::setBuffer(buffer);
    connectToolChecked();
}

QRect RectangleTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (rubberBand_.pending) {
        QPoint savedFrom = rubberBand_.from;
        QRect savedFill = pendingFillRect_;
        rubberBand_.clear();
        // The rectangle was pre-filled before the rubber band; merge that undo entry
        // so the whole operation collapses to a single undo step.
        buffer_->mergeLastUndo();
        if (gradientFillIsRadial(activeGradientFillMode)) {
            float r = GradientRenderer::conformRadius(savedFill, point);
            return applyGradientRect(savedFill, point, point + QPoint(qRound(r), 0));
        }
        return applyGradientRect(savedFill, savedFrom, point);
    }

    startPoint = point;

    QRect rect = changes(point);
    undoBuffer = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
    return draw(point);
}

QRect RectangleTool::hover(const QPoint &point)
{
    if (rubberBand_.pending)
        return rubberBand_.hoverRect(point, buffer_->image().rect());
    Pen *p = drawMode == FilledRectangle ? buffer_->toolPen() : buffer_->pen();
    return p->rect(point);
}

QRect RectangleTool::move(const QPoint &point)
{
    if (mouseButton_ == Qt::NoButton) {
        if (rubberBand_.pending)
            return rubberBand_.draw(point, buffer_->image());
        Pen *p = drawMode == FilledRectangle ? buffer_->toolPen() : buffer_->pen();
        return p->paint(point, buffer_);
    }

    if (!undoBuffer)
        return QRect();

    undoBuffer->apply(buffer_);
    delete undoBuffer;

    QPoint p0, p1;
    cornerPoints(point, p0, p1);
    QRect changedRect;
    auto changesLambda = [this, &changedRect](const QPoint &p) { changedRect = changedRect.united(this->changes(p)); };
    auto drawLambda = [this](const QPoint &p) { this->draw(p); };
    if (drawMode == Rectangle) {
        Algorithms::rectangle(p0, p1, changesLambda);
        undoBuffer = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
        Algorithms::rectangle(p0, p1, drawLambda);
    } else if (drawMode == FilledRectangle && mouseButton_ == Qt::LeftButton
               && gradientFillActive()) {
        changedRect = QRect(p0, p1).normalized().intersected(buffer_->image().rect());
        undoBuffer = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
        bool needsRubberBand = gradientNeedsRubberBand();
        if (needsRubberBand) {
            // Show flat fill during drag; direction/center chosen via rubber band after release.
            Algorithms::fillRectangle(p0, p1, drawLambda);
        } else {
            drawGradientRect(changedRect, point);
        }
    } else {
        Algorithms::fillRectangle(p0, p1, changesLambda);
        undoBuffer = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
        Algorithms::fillRectangle(p0, p1, drawLambda);
    }
    return changedRect;
}

QRect RectangleTool::release(const QPoint &point)
{
    if (!undoBuffer)
        return QRect();

    undoBuffer->apply(buffer_);
    delete undoBuffer;
    undoBuffer = nullptr;

    QPoint p0, p1;
    cornerPoints(point, p0, p1);
    QRect changedRect;
    if (drawMode == Rectangle) {
        Algorithms::rectangle(p0, p1, [this, &changedRect](const QPoint &pt) { changedRect = changedRect.united(this->draw(pt)); });
    } else if (drawMode == FilledRectangle && mouseButton_ == Qt::LeftButton
               && gradientFillActive()) {
        QRect fillRect = QRect(p0, p1).normalized().intersected(buffer_->image().rect());
        bool needsRubberBand = gradientNeedsRubberBand();
        if (needsRubberBand) {
            // Flat fill now; rubber band selects gradient direction/center.
            Algorithms::fillRectangle(p0, p1, [this, &changedRect](const QPoint &pt) {
                changedRect = changedRect.united(draw(pt));
            });
            pendingFillRect_ = fillRect;
            rubberBand_.start(fillRect.center());
        } else {
            changedRect = drawGradientRect(fillRect, point);
        }
    } else {
        Algorithms::fillRectangle(p0, p1, [this, &changedRect](const QPoint &pt) { changedRect = changedRect.united(this->draw(pt)); });
    }
    return changedRect;
}

void RectangleTool::cornerPoints(const QPoint &current, QPoint &p0, QPoint &p1) const
{
    if (anchorMode_ == CornerToCorner) {
        p0 = startPoint;
        p1 = current;
    } else {
        int dx = qAbs(current.x() - startPoint.x());
        int dy = qAbs(current.y() - startPoint.y());
        QRect ir = buffer_->image().rect();
        p0 = QPoint(qMax(ir.left(),   startPoint.x() - dx),
                    qMax(ir.top(),    startPoint.y() - dy));
        p1 = QPoint(qMin(ir.right(),  startPoint.x() + dx),
                    qMin(ir.bottom(), startPoint.y() + dy));
    }
}

QWidget* RectangleTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    ui_ = new Ui::RectangleToolOptions;
    ui_->setupUi(w);
    ui_->cornerBtn->setChecked(anchorMode_ == CornerToCorner);
    ui_->centerBtn->setChecked(anchorMode_ == CenterToCorner);
    connect(ui_->centerBtn, &QRadioButton::toggled, this, &RectangleTool::setAnchorMode);
    return w;
}

void RectangleTool::setAnchorMode(bool centerToCorner)
{
    anchorMode_ = centerToCorner ? CenterToCorner : CornerToCorner;
}

QRect RectangleTool::applyGradientRect(const QRect &fillRect, const QPoint &gradFrom, const QPoint &gradTo)
{
    QImage &image = buffer_->image();
    const GradientRange *range = &gradientRanges[activeGradientRange];
    const bool isHighlight = activeGradientFillMode == FillHighlight;
    // Radial/Spherical with conform also normalize per-direction to the actual shape boundary.
    const bool useShapeConform = isHighlight || (conformFill && gradientFillIsRadial(activeGradientFillMode));
    QList<QPoint> rectPoly;
    if (useShapeConform)
        rectPoly = {fillRect.topLeft(), fillRect.topRight(),
                    fillRect.bottomRight(), fillRect.bottomLeft()};
    QRect conformRect = (conformFill && !useShapeConform) ? fillRect : QRect();
    for (int y = fillRect.top(); y <= fillRect.bottom(); y++) {
        for (int x = fillRect.left(); x <= fillRect.right(); x++) {
            float t;
            if (useShapeConform) {
                t = GradientRenderer::highlightTPolygon(x, y, gradFrom, rectPoly);
                if (activeGradientFillMode == FillSpherical)
                    t = sphericalT(t);
            } else {
                t = GradientRenderer::computeT(x, y, activeGradientFillMode, gradFrom, gradTo, conformRect);
            }
            int ci = GradientRenderer::colorIndex(t, x, y, range, image);
            image.setPixel(x, y, static_cast<uint>(ci));
        }
    }
    return fillRect;
}

QRect RectangleTool::drawGradientRect(const QRect &fillRect, const QPoint &current)
{
    bool isRadial = gradientFillIsRadial(activeGradientFillMode);
    QPoint from = (centerFill && isRadial) ? fillRect.center() : startPoint;
    return applyGradientRect(fillRect, from, current);
}

void RectangleTool::cancel()
{
    if (rubberBand_.pending) {
        rubberBand_.clear();
        buffer_->clearHoverPreview();
        buffer_->undo();
    }
}

QString RectangleTool::status() const
{
    return rubberBand_.status();
}

QRect RectangleTool::changes(const QPoint &point)
{
    Pen *p = drawMode == FilledRectangle ? buffer_->toolPen() : buffer_->pen();
    return p->rect(point);
}

QRect RectangleTool::draw(const QPoint &point)
{
    Pen *p = drawMode == FilledRectangle ? buffer_->toolPen() : buffer_->pen();
    if (mouseButton_ == Qt::LeftButton) {
        if (drawMode == FilledRectangle && gradientFillActive())
            return p->paintAsColor(point, buffer_);
        return p->paint(point, buffer_);
    } else {
        return p->erase(point, buffer_);
    }
}

void RectangleTool::registerTool()
{
    Tool::registerTool();

    button_->setCheckable(true);
    setDrawMode(drawMode);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void RectangleTool::activate()
{
    if (buffer_->tool() == this) {
        setDrawMode(static_cast<DrawMode>((drawMode + 1) % (FilledRectangle + 1)));
        button_->setChecked(true);
    }

    Tool::activate();
}

void RectangleTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 5);
}
