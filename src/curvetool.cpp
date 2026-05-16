#include <QGridLayout>
#include <QtCore/qmath.h>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "curvetool.h"

CurveTool CurveTool::instance;

CurveTool::CurveTool(QObject *parent) : Tool(parent),
    curveMode_(Quadratic), phase_(0), erasing_(false), draggedHandle_(-1), undoBuffer_(nullptr)
{
}

void CurveTool::resetState()
{
    phase_ = 0;
    erasing_ = false;
    draggedHandle_ = -1;
    delete undoBuffer_;
    undoBuffer_ = nullptr;
}

void CurveTool::cancel()
{
    if (phase_ == 0) return;
    if (undoBuffer_) {
        undoBuffer_->apply(buffer_);
        buffer_->notifyModified(buffer_->image().rect());
    }
    resetState();
}

QRect CurveTool::doubleClick(const QPoint &point)
{
    Q_UNUSED(point)
    if (curveMode_ == Bezier && phase_ == 4) {
        QPoint savedP0 = p0_, savedP1 = p1_, savedP2 = p2_, savedP3 = p3_;
        resetState();
        return drawCubicBezier(savedP0, savedP1, savedP2, savedP3);
    }
    return QRect();
}

QString CurveTool::name() const
{
    return curveMode_ == Quadratic ? "Elliptical Curve" : "Bezier Curve";
}

void CurveTool::setBuffer(Buffer *buffer)
{
    disconnectToolChecked();
    resetState();
    Tool::setBuffer(buffer);
    connectToolChecked();
}

// ── press ──────────────────────────────────────────────────────────────────

QRect CurveTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (curveMode_ == Quadratic) {
        if (phase_ == 0) {
            erasing_ = (mouseButton_ == Qt::RightButton);
            phase_ = 1;
            p0_ = point;
            QRect dotRect = buffer_->pen()->rect(point);
            undoBuffer_ = new UndoBuffer(dotRect.topLeft(), buffer_->image().copy(dotRect), this);
            return draw(point);
        }

        if (phase_ == 1) {
            delete undoBuffer_;
            undoBuffer_ = nullptr;
            p2_ = point;
            return QRect();
        }

        // phase_ == 2: commit on release
        return QRect();
    }

    // Bezier
    if (mouseButton_ == Qt::RightButton) {
        if (phase_ == 4) {
            QPoint savedP0 = p0_, savedP1 = p1_, savedP2 = p2_, savedP3 = p3_;
            resetState();
            return drawCubicBezier(savedP0, savedP1, savedP2, savedP3);
        }
        resetState();
        return QRect();
    }

    switch (phase_) {
    case 0: p0_ = point; phase_ = 1; break;
    case 1: p3_ = point; phase_ = 2; break;
    case 2: p1_ = point; phase_ = 3; break;
    case 3: p2_ = point; phase_ = 4; break;
    case 4: {
        draggedHandle_ = nearestHandle(point);
        if (draggedHandle_ >= 0) {
            QRect rect = bezierBoundingRect(point).intersected(buffer_->image().rect());
            undoBuffer_ = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
            return drawBezierPreview(point);
        }
        break;
    }
    default: break;
    }
    return QRect();
}

// ── move ───────────────────────────────────────────────────────────────────

QRect CurveTool::move(const QPoint &point)
{
    if (curveMode_ == Quadratic) {
        if (mouseButton_ != Qt::NoButton) {
            if (phase_ == 1) {
                if (!undoBuffer_)
                    return QRect();
                undoBuffer_->apply(buffer_);
                delete undoBuffer_;

                QRect changedRect;
                Algorithms::line(p0_, point, [this, &changedRect](const QPoint &p) {
                    changedRect = changedRect.united(buffer_->pen()->rect(p));
                });
                undoBuffer_ = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
                Algorithms::line(p0_, point, [this](const QPoint &p) { draw(p); });
                return changedRect;
            }
            return QRect();
        }

        if (phase_ == 0)
            return buffer_->pen()->paint(point, buffer_);

        if (phase_ == 1) {
            QRect changedRect;
            Algorithms::line(p0_, point, [this, &changedRect](const QPoint &p) {
                changedRect = changedRect.united(draw(p));
            });
            return changedRect;
        }

        if (phase_ == 2) {
            return drawQuadraticCurve(p0_, p2_, point);
        }

        return QRect();
    }

    // Bezier
    if (mouseButton_ != Qt::NoButton) {
        if (phase_ == 4 && draggedHandle_ >= 0) {
            if (!undoBuffer_)
                return QRect();
            undoBuffer_->apply(buffer_);
            delete undoBuffer_;
            undoBuffer_ = nullptr;

            switch (draggedHandle_) {
            case 0: p0_ = point; break;
            case 1: p1_ = point; break;
            case 2: p2_ = point; break;
            case 3: p3_ = point; break;
            }

            QRect rect = bezierBoundingRect(point).intersected(buffer_->image().rect());
            undoBuffer_ = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
            return drawBezierPreview(point);
        }
        return QRect();
    }

    if (phase_ == 0)
        return buffer_->pen()->paint(point, buffer_);

    return drawBezierPreview(point);
}

// ── release ────────────────────────────────────────────────────────────────

QRect CurveTool::release(const QPoint &point)
{
    if (curveMode_ == Quadratic) {
        if (mouseButton_ != Qt::LeftButton) {
            return QRect();
        }

        if (phase_ == 1) {
            if (!undoBuffer_)
                return QRect();
            undoBuffer_->apply(buffer_);
            delete undoBuffer_;
            undoBuffer_ = nullptr;
            p2_ = point;
            phase_ = 2;
            return QRect();
        }

        if (phase_ == 2) {
            phase_ = 0;
            return drawQuadraticCurve(p0_, p2_, point);
        }

        return QRect();
    }

    // Bezier
    if (mouseButton_ == Qt::LeftButton && phase_ == 4 && draggedHandle_ >= 0) {
        if (!undoBuffer_)
            return QRect();
        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;
        draggedHandle_ = -1;
    }
    return QRect();
}

// ── hover ──────────────────────────────────────────────────────────────────

QRect CurveTool::hover(const QPoint &point)
{
    if (phase_ == 0)
        return buffer_->pen()->rect(point);

    if (curveMode_ == Quadratic) {
        if (phase_ == 1) {
            return quadraticBoundingRect(p0_, point, point).intersected(buffer_->image().rect());
        }
        if (phase_ == 2) {
            return quadraticBoundingRect(p0_, p2_, point).intersected(buffer_->image().rect());
        }
        return QRect();
    }

    return bezierBoundingRect(point).intersected(buffer_->image().rect());
}

// ── Quadratic helpers ──────────────────────────────────────────────────────

QRect CurveTool::draw(const QPoint &point)
{
    if (erasing_) {
        return buffer_->pen()->erase(point, buffer_);
    }
    return buffer_->pen()->paint(point, buffer_);
}

QRect CurveTool::drawQuadraticCurve(const QPoint &p0, const QPoint &p2, const QPoint &controlMid)
{
    qreal p1x = 2.0 * controlMid.x() - 0.5 * (p0.x() + p2.x());
    qreal p1y = 2.0 * controlMid.y() - 0.5 * (p0.y() + p2.y());

    int steps = qMax(1, qMax(qAbs(p2.x() - p0.x()), qAbs(p2.y() - p0.y())));

    QRect changedRect;
    QPoint prev = p0;
    for (int i = 1; i <= steps; i++) {
        qreal t = static_cast<qreal>(i) / steps;
        qreal u = 1.0 - t;
        qreal x = u * u * p0.x() + 2.0 * u * t * p1x + t * t * p2.x();
        qreal y = u * u * p0.y() + 2.0 * u * t * p1y + t * t * p2.y();
        QPoint cur(qFloor(x), qFloor(y));
        Algorithms::line(prev, cur, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(draw(p));
        });
        prev = cur;
    }
    return changedRect;
}

QRect CurveTool::quadraticBoundingRect(const QPoint &p0, const QPoint &p2, const QPoint &controlMid) const
{
    QRect penRect = buffer_->pen()->rect(QPoint(0, 0));
    int penW = penRect.width();
    int penH = penRect.height();
    int p1x = qRound(2.0 * controlMid.x() - 0.5 * (p0.x() + p2.x()));
    int p1y = qRound(2.0 * controlMid.y() - 0.5 * (p0.y() + p2.y()));
    int minX = qMin(p0.x(), qMin(p1x, p2.x()));
    int minY = qMin(p0.y(), qMin(p1y, p2.y()));
    int maxX = qMax(p0.x(), qMax(p1x, p2.x()));
    int maxY = qMax(p0.y(), qMax(p1y, p2.y()));
    return QRect(minX - penW, minY - penH, maxX - minX + 2 * penW + 1, maxY - minY + 2 * penH + 1);
}

// ── Bezier (cubic) helpers ─────────────────────────────────────────────────

QRect CurveTool::drawCubicBezier(const QPoint &p0, const QPoint &p1,
                                  const QPoint &p2, const QPoint &p3)
{
    int span   = qAbs(p3.x() - p0.x()) + qAbs(p3.y() - p0.y());
    int ctrl01 = qAbs(p1.x() - p0.x()) + qAbs(p1.y() - p0.y());
    int ctrl32 = qAbs(p2.x() - p3.x()) + qAbs(p2.y() - p3.y());
    int steps  = qMax(1, qMax(span, qMax(ctrl01, ctrl32)));

    QRect changedRect;
    QPoint prev = p0;
    for (int i = 1; i <= steps; i++) {
        qreal t = static_cast<qreal>(i) / steps;
        qreal u = 1.0 - t;
        qreal x = u*u*u * p0.x() + 3.0*u*u*t * p1.x() + 3.0*u*t*t * p2.x() + t*t*t * p3.x();
        qreal y = u*u*u * p0.y() + 3.0*u*u*t * p1.y() + 3.0*u*t*t * p2.y() + t*t*t * p3.y();
        QPoint cur(qRound(x), qRound(y));
        Algorithms::line(prev, cur, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(buffer_->pen()->paint(p, buffer_));
        });
        prev = cur;
    }
    return changedRect;
}

QRect CurveTool::drawHandle(const QPoint &center)
{
    QRect changedRect;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            QPoint p(center.x() + dx, center.y() + dy);
            buffer_->pen()->paint(p, buffer_);
            changedRect = changedRect.united(QRect(p, p));
        }
    }
    return changedRect;
}

QRect CurveTool::drawDashedLine(const QPoint &from, const QPoint &to)
{
    QRect changedRect;
    int i = 0;
    Algorithms::line(from, to, [this, &changedRect, &i](const QPoint &p) {
        if ((i & 1) == 0) {
            buffer_->pen()->paint(p, buffer_);
            changedRect = changedRect.united(QRect(p, p));
        }
        i++;
    });
    return changedRect;
}

QRect CurveTool::drawBezierPreview(const QPoint &cursor)
{
    QRect changedRect;
    switch (phase_) {
    case 1:
        changedRect = changedRect.united(drawHandle(p0_));
        changedRect = changedRect.united(drawDashedLine(p0_, cursor));
        break;
    case 2:
        changedRect = changedRect.united(drawHandle(p0_));
        changedRect = changedRect.united(drawHandle(p3_));
        changedRect = changedRect.united(drawDashedLine(p0_, cursor));
        break;
    case 3:
        changedRect = changedRect.united(drawHandle(p0_));
        changedRect = changedRect.united(drawHandle(p3_));
        changedRect = changedRect.united(drawHandle(p1_));
        changedRect = changedRect.united(drawDashedLine(p0_, p1_));
        changedRect = changedRect.united(drawDashedLine(p3_, cursor));
        changedRect = changedRect.united(drawCubicBezier(p0_, p1_, cursor, p3_));
        break;
    case 4:
        changedRect = changedRect.united(drawHandle(p0_));
        changedRect = changedRect.united(drawHandle(p1_));
        changedRect = changedRect.united(drawHandle(p2_));
        changedRect = changedRect.united(drawHandle(p3_));
        changedRect = changedRect.united(drawDashedLine(p0_, p1_));
        changedRect = changedRect.united(drawDashedLine(p3_, p2_));
        changedRect = changedRect.united(drawCubicBezier(p0_, p1_, p2_, p3_));
        break;
    default:
        break;
    }
    return changedRect;
}

QRect CurveTool::bezierBoundingRect(const QPoint &cursor) const
{
    QRect penRect = buffer_->pen()->rect(QPoint(0, 0));
    int margin = qMax(penRect.width(), penRect.height()) + 2;

    QPoint pts[4];
    int n = 0;
    switch (phase_) {
    case 1: pts[n++] = p0_; pts[n++] = cursor; break;
    case 2: pts[n++] = p0_; pts[n++] = p3_; pts[n++] = cursor; break;
    case 3: pts[n++] = p0_; pts[n++] = p3_; pts[n++] = p1_; pts[n++] = cursor; break;
    case 4: pts[n++] = p0_; pts[n++] = p1_; pts[n++] = p2_; pts[n++] = p3_; break;
    default: return QRect();
    }

    int minX = pts[0].x(), maxX = pts[0].x();
    int minY = pts[0].y(), maxY = pts[0].y();
    for (int i = 1; i < n; i++) {
        minX = qMin(minX, pts[i].x());
        minY = qMin(minY, pts[i].y());
        maxX = qMax(maxX, pts[i].x());
        maxY = qMax(maxY, pts[i].y());
    }

    return QRect(minX - margin, minY - margin,
                 maxX - minX + 2 * margin + 1,
                 maxY - minY + 2 * margin + 1);
}

int CurveTool::nearestHandle(const QPoint &point) const
{
    const QPoint handles[4] = {p0_, p1_, p2_, p3_};
    int nearest = -1;
    int nearestDist = 100;
    for (int i = 0; i < 4; i++) {
        QPoint d = handles[i] - point;
        int dist = d.x() * d.x() + d.y() * d.y();
        if (dist < nearestDist) {
            nearestDist = dist;
            nearest = i;
        }
    }
    return nearest;
}

// ── Tool registration ──────────────────────────────────────────────────────

void CurveTool::setCurveMode(CurveMode mode)
{
    curveMode_ = mode;
    button_->setIcon(QIcon(mode == Quadratic ? ":/curve.png" : ":/beziercurve.png"));
    button_->setToolTip(mode == Quadratic ? "Curve – Quadratic [Q]" : "Curve – Bézier [Q]");
}

void CurveTool::registerTool()
{
    Tool::registerTool();

    button_->setCheckable(true);
    setCurveMode(curveMode_);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void CurveTool::activate()
{
    resetState();
    if (buffer_->tool() == this) {
        setCurveMode(curveMode_ == Quadratic ? Bezier : Quadratic);
        button_->setChecked(true);
    }
    Tool::activate();
}

void CurveTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 4);
}
