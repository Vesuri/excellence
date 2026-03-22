#include <QGridLayout>
#include <QtCore/qmath.h>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "beziercurvetool.h"

BezierCurveTool BezierCurveTool::instance;

BezierCurveTool::BezierCurveTool(QObject *parent) : Tool(parent),
    phase_(0), draggedHandle_(-1), undoBuffer_(nullptr)
{
}

void BezierCurveTool::resetState()
{
    phase_ = 0;
    draggedHandle_ = -1;
    delete undoBuffer_;
    undoBuffer_ = nullptr;
}

void BezierCurveTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    resetState();
    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect BezierCurveTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (mouseButton_ == Qt::RightButton) {
        if (phase_ == 4) {
            // Commit: Buffer already cleared moveUndoBuffer, draw permanent curve
            QPoint savedP0 = p0_, savedP1 = p1_, savedP2 = p2_, savedP3 = p3_;
            resetState();
            return drawCubicBezier(savedP0, savedP1, savedP2, savedP3);
        }
        resetState();
        return QRect();
    }

    // Left button — advance through placement phases
    switch (phase_) {
    case 0:
        p0_ = point;
        phase_ = 1;
        break;
    case 1:
        p3_ = point;
        phase_ = 2;
        break;
    case 2:
        p1_ = point;
        phase_ = 3;
        break;
    case 3:
        p2_ = point;
        phase_ = 4;
        break;
    case 4: {
        // Start dragging the nearest handle
        draggedHandle_ = nearestHandle(point);
        if (draggedHandle_ >= 0) {
            QRect rect = previewBoundingRect(point).intersected(buffer_->image().rect());
            undoBuffer_ = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
            return drawPreview(point);
        }
        break;
    }
    default:
        break;
    }
    return QRect();
}

QRect BezierCurveTool::move(const QPoint &point)
{
    if (mouseButton_ != Qt::NoButton) {
        if (phase_ == 4 && draggedHandle_ >= 0) {
            // Drag preview: clear previous, update handle, save new area, redraw
            undoBuffer_->apply(buffer_);
            delete undoBuffer_;
            undoBuffer_ = nullptr;

            switch (draggedHandle_) {
            case 0: p0_ = point; break;
            case 1: p1_ = point; break;
            case 2: p2_ = point; break;
            case 3: p3_ = point; break;
            }

            QRect rect = previewBoundingRect(point).intersected(buffer_->image().rect());
            undoBuffer_ = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
            return drawPreview(point);
        }
        return QRect();
    }

    if (phase_ > 0) {
        return drawPreview(point);
    }
    return QRect();
}

QRect BezierCurveTool::release(const QPoint &)
{
    if (mouseButton_ == Qt::LeftButton && phase_ == 4 && draggedHandle_ >= 0) {
        // Clear drag preview; next hover move will redraw it
        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;
        draggedHandle_ = -1;
    }
    return QRect();
}

QRect BezierCurveTool::hover(const QPoint &point)
{
    if (phase_ > 0) {
        return previewBoundingRect(point).intersected(buffer_->image().rect());
    }
    return QRect();
}

// ── Drawing helpers ────────────────────────────────────────────────────────

QRect BezierCurveTool::drawCubicBezier(const QPoint &p0, const QPoint &p1,
                                        const QPoint &p2, const QPoint &p3)
{
    int span    = qAbs(p3.x() - p0.x()) + qAbs(p3.y() - p0.y());
    int ctrl01  = qAbs(p1.x() - p0.x()) + qAbs(p1.y() - p0.y());
    int ctrl32  = qAbs(p2.x() - p3.x()) + qAbs(p2.y() - p3.y());
    int steps   = qMax(1, qMax(span, qMax(ctrl01, ctrl32)));

    QRect changedRect;
    QPoint prev = p0;
    for (int i = 1; i <= steps; i++) {
        qreal t = static_cast<qreal>(i) / steps;
        qreal u = 1.0 - t;
        qreal x = u*u*u * p0.x() + 3.0*u*u*t * p1.x() + 3.0*u*t*t * p2.x() + t*t*t * p3.x();
        qreal y = u*u*u * p0.y() + 3.0*u*u*t * p1.y() + 3.0*u*t*t * p2.y() + t*t*t * p3.y();
        QPoint cur(qRound(x), qRound(y));
        Algorithms::line(prev, cur, [this, &changedRect](const QPoint &p) {
            if (buffer_->image().rect().contains(p)) {
                changedRect = changedRect.united(buffer_->pen()->paint(p, buffer_));
            }
        });
        prev = cur;
    }
    return changedRect;
}

QRect BezierCurveTool::drawHandle(const QPoint &center)
{
    QRect changedRect;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            QPoint p(center.x() + dx, center.y() + dy);
            if (buffer_->image().rect().contains(p)) {
                buffer_->pen()->paint(p, buffer_);
                changedRect = changedRect.united(QRect(p, p));
            }
        }
    }
    return changedRect;
}

QRect BezierCurveTool::drawDashedLine(const QPoint &from, const QPoint &to)
{
    QRect changedRect;
    int i = 0;
    Algorithms::line(from, to, [this, &changedRect, &i](const QPoint &p) {
        if ((i & 1) == 0 && buffer_->image().rect().contains(p)) {
            buffer_->pen()->paint(p, buffer_);
            changedRect = changedRect.united(QRect(p, p));
        }
        i++;
    });
    return changedRect;
}

QRect BezierCurveTool::drawPreview(const QPoint &cursor)
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
        // Preview curve with cursor as P2
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

QRect BezierCurveTool::previewBoundingRect(const QPoint &cursor) const
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

int BezierCurveTool::nearestHandle(const QPoint &point) const
{
    const QPoint handles[4] = {p0_, p1_, p2_, p3_};
    int nearest = -1;
    int nearestDist = 100; // 10-pixel threshold
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

void BezierCurveTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/beziercurve.png"));
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void BezierCurveTool::activate()
{
    resetState();
    Tool::activate();
}

void BezierCurveTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 3, 5);
}
