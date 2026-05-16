#include <QGridLayout>
#include <QRadioButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QtCore/qmath.h>
#include <cmath>
#include <climits>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "ellipsetool.h"
#include "gradientrange.h"
#include "gradientrenderer.h"

EllipseTool EllipseTool::instance;
const char *EllipseTool::icons[] = {
    ":/ellipse.png",
    ":/filledellipse.png"
};

EllipseTool::EllipseTool(QObject *parent) : Tool(parent),
    drawMode_(Ellipse), anchorMode_(CornerToCorner), rotateMode_(false),
    erasing_(false), phase_(0), cx_(0), cy_(0), rx_(0), ry_(0),
    rotationAngle_(0.0), undoBuffer_(nullptr)
{
}

void EllipseTool::resetState()
{
    phase_ = 0;
    erasing_ = false;
    rotationAngle_ = 0.0;
    delete undoBuffer_;
    undoBuffer_ = nullptr;
}

void EllipseTool::cancel()
{
    if (rubberBand_.pending) {
        rubberBand_.clear();
        buffer_->clearHoverPreview();
        buffer_->undo();
        return;
    }
    if (phase_ == 0) return;
    if (undoBuffer_) {
        undoBuffer_->apply(buffer_);
        buffer_->notifyModified(buffer_->image().rect());
    }
    resetState();
}

void EllipseTool::setDrawMode(DrawMode mode)
{
    drawMode_ = mode;
    button_->setIcon(QIcon(icons[mode]));
    button_->setToolTip(mode == Ellipse
        ? "Ellipse [C]  Shift+C: filled  Ctrl: circle\nRight-click: anchor options"
        : "Ellipse – Filled [C]  Ctrl: circle\nRight-click: anchor options");
}

void EllipseTool::setBuffer(Buffer *buffer)
{
    disconnectToolChecked();
    resetState();
    Tool::setBuffer(buffer);
    connectToolChecked();
}

// ── Geometry helpers ───────────────────────────────────────────────────────

void EllipseTool::cornerPoints(const QPoint &current, QPoint &p0, QPoint &p1) const
{
    bool ctrl = QGuiApplication::keyboardModifiers() & Qt::ControlModifier;
    if (anchorMode_ == CornerToCorner) {
        if (ctrl) {
            int dx = current.x() - startPoint_.x();
            int dy = current.y() - startPoint_.y();
            int d = qMin(qAbs(dx), qAbs(dy));
            p0 = startPoint_;
            p1 = QPoint(startPoint_.x() + (dx >= 0 ? d : -d),
                        startPoint_.y() + (dy >= 0 ? d : -d));
        } else {
            p0 = startPoint_;
            p1 = current;
        }
    } else {
        int dx = qAbs(current.x() - startPoint_.x());
        int dy = qAbs(current.y() - startPoint_.y());
        if (ctrl) dx = dy = qMin(dx, dy);
        QRect ir = buffer_->image().rect();
        p0 = QPoint(qMax(ir.left(),   startPoint_.x() - dx),
                    qMax(ir.top(),    startPoint_.y() - dy));
        p1 = QPoint(qMin(ir.right(),  startPoint_.x() + dx),
                    qMin(ir.bottom(), startPoint_.y() + dy));
    }
}

void EllipseTool::computeEllipseParams(const QPoint &p0, const QPoint &p1)
{
    QRect r = QRect(p0, p1).normalized();
    cx_ = r.center().x();
    cy_ = r.center().y();
    rx_ = r.width() / 2;
    ry_ = r.height() / 2;
}

QRect EllipseTool::drawEllipseShape(double angle, bool applyGradient)
{
    QRect changedRect;
    if (drawMode_ == FilledEllipse && applyGradient && !erasing_
        && gradientFillActive()) {
        QImage &image = buffer_->image();
        const GradientRange *range = &gradientRanges[activeGradientRange];
        bool hvMode = activeGradientFillMode == FillHorizontal || activeGradientFillMode == FillVertical;
        bool isRadial = gradientFillIsRadial(activeGradientFillMode);
        QPoint gradFrom = hvMode ? QPoint(cx_ - rx_, cy_ - ry_) : startPoint_;
        if (centerFill && isRadial)
            gradFrom = QPoint(cx_, cy_);
        QPoint gradTo = hvMode ? QPoint(cx_ + rx_, cy_ + ry_) : QPoint(cx_ + rx_, cy_);
        QRect ellipseBbox = QRect(cx_ - rx_, cy_ - ry_, 2 * rx_ + 1, 2 * ry_ + 1);
        QRect conformRect = conformFill ? ellipseBbox : QRect();

        // For H/V conform, the per-row or per-column span matters, not the global bbox.
        // fillEllipse calls pixels in row order, so we can compute the row's x extent
        // once per row by re-solving the same quadratic that fillEllipse uses internally.
        const bool hConform = conformFill && activeGradientFillMode == FillHorizontal;
        const bool vConform = conformFill && activeGradientFillMode == FillVertical;
        const double cosA = std::cos(angle), sinA = std::sin(angle);
        const double rx2 = double(rx_) * rx_, ry2 = double(ry_) * ry_;

        // For V conform: pre-compute per-column y extents.
        // Solving for dy given dx uses the same quadratic with cosA/sinA swapped.
        int xBound = 0;
        QVector<int> colY0, colY1;
        if (vConform) {
            xBound = int(std::ceil(std::sqrt(rx2 * cosA * cosA + ry2 * sinA * sinA))) + 1;
            colY0.fill(INT_MAX, 2 * xBound + 1);
            colY1.fill(INT_MIN, 2 * xBound + 1);
            for (int dx = -xBound; dx <= xBound; dx++) {
                double A = sinA * sinA / rx2 + cosA * cosA / ry2;
                double B = 2.0 * dx * cosA * sinA * (1.0 / rx2 - 1.0 / ry2);
                double C = double(dx) * dx * (cosA * cosA / rx2 + sinA * sinA / ry2) - 1.0;
                double disc = B * B - 4.0 * A * C;
                if (disc < 0) continue;
                double sqrtD = std::sqrt(disc);
                int y1 = cy_ + qRound((-B - sqrtD) / (2.0 * A));
                int y2 = cy_ + qRound((-B + sqrtD) / (2.0 * A));
                if (y1 > y2) qSwap(y1, y2);
                colY0[dx + xBound] = y1;
                colY1[dx + xBound] = y2;
            }
        }

        int lastY = INT_MIN;
        int rowX0 = 0, rowX1 = 0;
        auto fn = [&](const QPoint &p) {
            QRect pixConform = conformRect;
            if (hConform) {
                if (p.y() != lastY) {
                    lastY = p.y();
                    int dy = p.y() - cy_;
                    double A = cosA * cosA / rx2 + sinA * sinA / ry2;
                    double B = 2.0 * dy * cosA * sinA * (1.0 / rx2 - 1.0 / ry2);
                    double C = double(dy) * dy * (sinA * sinA / rx2 + cosA * cosA / ry2) - 1.0;
                    double disc = B * B - 4.0 * A * C;
                    if (disc >= 0) {
                        double sqrtD = std::sqrt(disc);
                        rowX0 = cx_ + qRound((-B - sqrtD) / (2.0 * A));
                        rowX1 = cx_ + qRound((-B + sqrtD) / (2.0 * A));
                        if (rowX0 > rowX1) qSwap(rowX0, rowX1);
                    } else {
                        rowX0 = rowX1 = p.x();
                    }
                }
                pixConform = QRect(rowX0, p.y(), rowX1 - rowX0 + 1, 1);
            } else if (vConform) {
                int dxi = p.x() - cx_ + xBound;
                if (dxi >= 0 && dxi < colY0.size() && colY0[dxi] <= colY1[dxi])
                    pixConform = QRect(p.x(), colY0[dxi], 1, colY1[dxi] - colY0[dxi] + 1);
            }
            float t = GradientRenderer::computeT(p.x(), p.y(), activeGradientFillMode, gradFrom, gradTo, pixConform);
            int ci = GradientRenderer::colorIndex(t, p.x(), p.y(), range, image);
            image.setPixel(p.x(), p.y(), static_cast<uint>(ci));
            changedRect = changedRect.united(QRect(p, p));
        };
        Algorithms::fillEllipse(cx_, cy_, rx_, ry_, angle, fn);
    } else {
        auto fn = [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(draw(p));
        };
        if (drawMode_ == Ellipse) {
            Algorithms::ellipse(cx_, cy_, rx_, ry_, angle, fn);
        } else {
            Algorithms::fillEllipse(cx_, cy_, rx_, ry_, angle, fn);
        }
    }
    return changedRect;
}

QRect EllipseTool::ellipseBoundingRect(double angle) const
{
    Pen *p = drawMode_ == FilledEllipse ? buffer_->toolPen() : buffer_->pen();
    QRect penRect = p->rect(QPoint(0, 0));
    int margin = qMax(penRect.width(), penRect.height()) + 2;
    double cosA = qCos(angle), sinA = qSin(angle);
    int hw = qRound(qSqrt((double)rx_ * rx_ * cosA * cosA + (double)ry_ * ry_ * sinA * sinA)) + margin;
    int hh = qRound(qSqrt((double)rx_ * rx_ * sinA * sinA + (double)ry_ * ry_ * cosA * cosA)) + margin;
    return QRect(cx_ - hw, cy_ - hh, 2 * hw + 1, 2 * hh + 1).intersected(buffer_->image().rect());
}

QRect EllipseTool::draw(const QPoint &point)
{
    Pen *p = drawMode_ == FilledEllipse ? buffer_->toolPen() : buffer_->pen();
    if (erasing_)
        return p->erase(point, buffer_);
    return p->paint(point, buffer_);
}

// ── press ──────────────────────────────────────────────────────────────────

QRect EllipseTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (rubberBand_.pending) {
        QPoint savedFrom = rubberBand_.from;
        double savedAngle = pendingAngle_;
        rubberBand_.clear();
        return applyGradientEllipse(savedAngle, savedFrom, point);
    }

    if (rotateMode_ && phase_ == 2) {
        if (mouseButton_ == Qt::RightButton) {
            resetState();
            return QRect();
        }
        if (!erasing_ && gradientFillActive() && activeGradientFillMode == FillLinear) {
            QRect r = drawEllipseShape(rotationAngle_, false);
            pendingAngle_ = rotationAngle_;
            rubberBand_.start(QPoint(cx_, cy_));
            resetState();
            return r;
        }
        QRect r = drawEllipseShape(rotationAngle_, true);
        resetState();
        return r;
    }

    erasing_ = (mouseButton_ == Qt::RightButton);
    phase_ = 1;
    startPoint_ = point;
    Pen *p = drawMode_ == FilledEllipse ? buffer_->toolPen() : buffer_->pen();
    QRect dotRect = p->rect(point).intersected(buffer_->image().rect());
    if (dotRect.isEmpty()) return QRect();
    undoBuffer_ = new UndoBuffer(dotRect.topLeft(), buffer_->image().copy(dotRect), this);
    return draw(point);
}

// ── move ───────────────────────────────────────────────────────────────────

QRect EllipseTool::move(const QPoint &point)
{
    if (rubberBand_.pending) {
        if (mouseButton_ == Qt::NoButton)
            return rubberBand_.draw(point, buffer_->image());
        return {};
    }

    if (rotateMode_ && phase_ == 2 && mouseButton_ == Qt::NoButton) {
        rotationAngle_ = qAtan2((double)(point.y() - cy_), (double)(point.x() - cx_));
        return drawEllipseShape(rotationAngle_, true);
    }

    if (mouseButton_ == Qt::NoButton) {
        if (phase_ == 0) {
            Pen *p = drawMode_ == FilledEllipse ? buffer_->toolPen() : buffer_->pen();
            return p->paint(point, buffer_);
        }
        return QRect();
    }
    if (phase_ != 1)
        return QRect();

    if (!undoBuffer_)
        return QRect();

    undoBuffer_->apply(buffer_);
    delete undoBuffer_;

    QPoint p0, p1;
    cornerPoints(point, p0, p1);
    computeEllipseParams(p0, p1);

    QRect changedRect = ellipseBoundingRect(0.0);
    undoBuffer_ = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
    // Use flat fill during drag for linear gradient; gradient applied after rubber band.
    bool useGradient = !(activeGradientFillMode == FillLinear && gradientFillActive() && !erasing_);
    drawEllipseShape(0.0, useGradient);
    return changedRect;
}

// ── release ────────────────────────────────────────────────────────────────

QRect EllipseTool::release(const QPoint &point)
{
    if (phase_ != 1) return QRect();

    if (!undoBuffer_) return QRect();

    QPoint p0, p1;
    cornerPoints(point, p0, p1);
    computeEllipseParams(p0, p1);

    undoBuffer_->apply(buffer_);
    delete undoBuffer_;
    undoBuffer_ = nullptr;

    if (rotateMode_) {
        rotationAngle_ = 0.0;
        phase_ = 2;
        return QRect();
    }

    if (!erasing_ && gradientFillActive() && activeGradientFillMode == FillLinear
        && drawMode_ == FilledEllipse) {
        QRect r = drawEllipseShape(0.0, false);  // flat fill
        pendingAngle_ = 0.0;
        rubberBand_.start(QPoint(cx_, cy_));
        resetState();  // clears phase_, undoBuffer_, but cx_/cy_/rx_/ry_ are not members of resetState
        return r;
    }

    QRect r = drawEllipseShape(0.0, true);
    resetState();
    return r;
}

// ── hover ──────────────────────────────────────────────────────────────────

QRect EllipseTool::hover(const QPoint &point)
{
    if (rubberBand_.pending)
        return rubberBand_.hoverRect(point, buffer_->image().rect());
    if (phase_ == 0) {
        Pen *p = drawMode_ == FilledEllipse ? buffer_->toolPen() : buffer_->pen();
        return p->rect(point);
    }
    if (rotateMode_ && phase_ == 2) {
        double angle = qAtan2((double)(point.y() - cy_), (double)(point.x() - cx_));
        return ellipseBoundingRect(angle);
    }
    return QRect();
}

// ── options window ─────────────────────────────────────────────────────────

QWidget* EllipseTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Ellipse");
    QVBoxLayout *layout = new QVBoxLayout(w);
    layout->setSpacing(8);
    layout->setContentsMargins(6, 6, 6, 6);

    QRadioButton *cornerBtn = new QRadioButton("Corner to Corner");
    QRadioButton *centerBtn = new QRadioButton("Center to Corner");
    cornerBtn->setChecked(anchorMode_ == CornerToCorner);
    centerBtn->setChecked(anchorMode_ == CenterToCorner);
    connect(centerBtn, &QRadioButton::toggled, this, &EllipseTool::setAnchorMode);

    QCheckBox *rotateBox = new QCheckBox("Rotate");
    rotateBox->setChecked(rotateMode_);
    connect(rotateBox, &QCheckBox::toggled, this, &EllipseTool::setRotateMode);

    layout->addWidget(cornerBtn);
    layout->addWidget(centerBtn);
    layout->addWidget(rotateBox);
    return w;
}

void EllipseTool::setAnchorMode(bool centerToCorner)
{
    anchorMode_ = centerToCorner ? CenterToCorner : CornerToCorner;
}

void EllipseTool::setRotateMode(bool rotate)
{
    rotateMode_ = rotate;
    if (!rotate) resetState();
}

QString EllipseTool::status() const
{
    return rubberBand_.status();
}

QRect EllipseTool::applyGradientEllipse(double angle, const QPoint &gradFrom, const QPoint &gradTo)
{
    QImage &image = buffer_->image();
    const GradientRange *range = &gradientRanges[activeGradientRange];
    QRect ellipseBbox = QRect(cx_ - rx_, cy_ - ry_, 2 * rx_ + 1, 2 * ry_ + 1);
    QRect conformRect = conformFill ? ellipseBbox : QRect();
    const bool hConform = conformFill && activeGradientFillMode == FillHorizontal;
    const bool vConform = conformFill && activeGradientFillMode == FillVertical;
    const double cosA = std::cos(angle), sinA = std::sin(angle);
    const double rx2 = double(rx_) * rx_, ry2 = double(ry_) * ry_;

    int xBound = 0;
    QVector<int> colY0, colY1;
    if (vConform) {
        xBound = int(std::ceil(std::sqrt(rx2 * cosA * cosA + ry2 * sinA * sinA))) + 1;
        colY0.fill(INT_MAX, 2 * xBound + 1);
        colY1.fill(INT_MIN, 2 * xBound + 1);
        for (int dx = -xBound; dx <= xBound; dx++) {
            double A = sinA * sinA / rx2 + cosA * cosA / ry2;
            double B = 2.0 * dx * cosA * sinA * (1.0 / rx2 - 1.0 / ry2);
            double C = double(dx) * dx * (cosA * cosA / rx2 + sinA * sinA / ry2) - 1.0;
            double disc = B * B - 4.0 * A * C;
            if (disc < 0) continue;
            double sqrtD = std::sqrt(disc);
            int y1 = cy_ + qRound((-B - sqrtD) / (2.0 * A));
            int y2 = cy_ + qRound((-B + sqrtD) / (2.0 * A));
            if (y1 > y2) qSwap(y1, y2);
            colY0[dx + xBound] = y1;
            colY1[dx + xBound] = y2;
        }
    }

    int lastY = INT_MIN;
    int rowX0 = 0, rowX1 = 0;
    QRect changedRect;
    Algorithms::fillEllipse(cx_, cy_, rx_, ry_, angle, [&](const QPoint &p) {
        QRect pixConform = conformRect;
        if (hConform) {
            if (p.y() != lastY) {
                lastY = p.y();
                int dy = p.y() - cy_;
                double A = cosA * cosA / rx2 + sinA * sinA / ry2;
                double B = 2.0 * dy * cosA * sinA * (1.0 / rx2 - 1.0 / ry2);
                double C = double(dy) * dy * (sinA * sinA / rx2 + cosA * cosA / ry2) - 1.0;
                double disc = B * B - 4.0 * A * C;
                if (disc >= 0) {
                    double sqrtD = std::sqrt(disc);
                    rowX0 = cx_ + qRound((-B - sqrtD) / (2.0 * A));
                    rowX1 = cx_ + qRound((-B + sqrtD) / (2.0 * A));
                    if (rowX0 > rowX1) qSwap(rowX0, rowX1);
                } else {
                    rowX0 = rowX1 = p.x();
                }
            }
            pixConform = QRect(rowX0, p.y(), rowX1 - rowX0 + 1, 1);
        } else if (vConform) {
            int dxi = p.x() - cx_ + xBound;
            if (dxi >= 0 && dxi < colY0.size() && colY0[dxi] <= colY1[dxi])
                pixConform = QRect(p.x(), colY0[dxi], 1, colY1[dxi] - colY0[dxi] + 1);
        }
        float t = GradientRenderer::computeT(p.x(), p.y(), activeGradientFillMode, gradFrom, gradTo, pixConform);
        int ci = GradientRenderer::colorIndex(t, p.x(), p.y(), range, image);
        image.setPixel(p.x(), p.y(), static_cast<uint>(ci));
        changedRect = changedRect.united(QRect(p, p));
    });
    return changedRect;
}

// ── Tool registration ──────────────────────────────────────────────────────

void EllipseTool::registerTool()
{
    Tool::registerTool();
    button_->setCheckable(true);
    setDrawMode(drawMode_);
    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void EllipseTool::activate()
{
    resetState();
    if (buffer_->tool() == this) {
        setDrawMode(drawMode_ == Ellipse ? FilledEllipse : Ellipse);
        button_->setChecked(true);
    }
    Tool::activate();
}

void EllipseTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 6);
}
