#include <QGridLayout>
#include <QRadioButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QtCore/qmath.h>
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
        QPoint gradFrom = hvMode ? QPoint(cx_ - rx_, cy_ - ry_) : startPoint_;
        QPoint gradTo   = hvMode ? QPoint(cx_ + rx_, cy_ + ry_) : QPoint(cx_ + rx_, cy_);
        auto fn = [&](const QPoint &p) {
            float t = GradientRenderer::computeT(p.x(), p.y(), activeGradientFillMode, gradFrom, gradTo);
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
    if (rotateMode_ && phase_ == 2) {
        if (mouseButton_ == Qt::RightButton) {
            resetState();
            return QRect();
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
    if (rotateMode_ && phase_ == 2 && mouseButton_ == Qt::NoButton) {
        rotationAngle_ = qAtan2((double)(point.y() - cy_), (double)(point.x() - cx_));
        return drawEllipseShape(rotationAngle_);
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
    drawEllipseShape(0.0);
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

    QRect r = drawEllipseShape(0.0, true);
    resetState();
    return r;
}

// ── hover ──────────────────────────────────────────────────────────────────

QRect EllipseTool::hover(const QPoint &point)
{
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
