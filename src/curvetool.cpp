#include <QGridLayout>
#include <QtCore/qmath.h>
#include "pen.h"
#include "buffer.h"
#include "algorithms.h"
#include "curvetool.h"

CurveTool CurveTool::instance;

CurveTool::CurveTool(QObject *parent) : Tool(parent),
    phase_(0),
    undoBuffer_(nullptr)
{
}

void CurveTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    phase_ = 0;
    delete undoBuffer_;
    undoBuffer_ = nullptr;
    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

QRect CurveTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (mouseButton_ == Qt::RightButton) {
        phase_ = 0;
        return QRect();
    }

    if (phase_ == 0) {
        // Phase 1: record start point, save area and stamp a dot
        phase_ = 1;
        p0_ = point;
        QRect dotRect = buffer_->pen()->rect(point);
        undoBuffer_ = new UndoBuffer(dotRect.topLeft(), buffer_->image().copy(dotRect), this);
        return draw(point);
    }

    if (phase_ == 1) {
        // User clicked P2 without dragging (button-up hover path);
        // Buffer already cleared moveUndoBuffer, so just discard our drag undo.
        delete undoBuffer_;
        undoBuffer_ = nullptr;
        p2_ = point;
        return QRect();
    }

    // phase_ == 2: commit on release
    return QRect();
}

QRect CurveTool::move(const QPoint &point)
{
    if (mouseButton_ != Qt::NoButton) {
        if (phase_ == 1) {
            // Drag preview: clear previous, save new area, draw straight line
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

    if (phase_ == 1) {
        // Button-up hover: straight line preview via Buffer's moveUndoBuffer mechanism
        QRect changedRect;
        Algorithms::line(p0_, point, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(draw(p));
        });
        return changedRect;
    }

    if (phase_ == 2) {
        // Curve preview: control midpoint is cursor
        return drawCurve(p0_, p2_, point);
    }

    return QRect();
}

QRect CurveTool::release(const QPoint &point)
{
    if (mouseButton_ != Qt::LeftButton) {
        return QRect();
    }

    if (phase_ == 1) {
        // Commit straight line (clear drag preview first), advance to phase 2
        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;
        p2_ = point;
        phase_ = 2;
        QRect changedRect;
        Algorithms::line(p0_, p2_, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(draw(p));
        });
        return changedRect;
    }

    if (phase_ == 2) {
        // Commit final curve
        phase_ = 0;
        return drawCurve(p0_, p2_, point);
    }

    return QRect();
}

QRect CurveTool::hover(const QPoint &point)
{
    if (phase_ == 1) {
        return curveBoundingRect(p0_, point, point).intersected(buffer_->image().rect());
    }
    if (phase_ == 2) {
        return curveBoundingRect(p0_, p2_, point).intersected(buffer_->image().rect());
    }
    return QRect();
}

QRect CurveTool::draw(const QPoint &point)
{
    return buffer_->pen()->paint(point, buffer_);
}

QRect CurveTool::drawCurve(const QPoint &p0, const QPoint &p2, const QPoint &controlMid)
{
    // Quadratic Bezier: P1 = 2*M - 0.5*(P0+P2)
    // where M is the visible midpoint (controlMid)
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
            if (buffer_->image().rect().contains(p)) {
                changedRect = changedRect.united(draw(p));
            }
        });
        prev = cur;
    }
    return changedRect;
}

QRect CurveTool::curveBoundingRect(const QPoint &p0, const QPoint &p2, const QPoint &controlMid) const
{
    QRect penRect = buffer_->pen()->rect(QPoint(0, 0));
    int penW = penRect.width();
    int penH = penRect.height();
    // Curve lies within convex hull of P0, P1, P2 where P1 is the actual
    // Bezier control point (not the visible midpoint M = controlMid)
    int p1x = qRound(2.0 * controlMid.x() - 0.5 * (p0.x() + p2.x()));
    int p1y = qRound(2.0 * controlMid.y() - 0.5 * (p0.y() + p2.y()));
    int minX = qMin(p0.x(), qMin(p1x, p2.x()));
    int minY = qMin(p0.y(), qMin(p1y, p2.y()));
    int maxX = qMax(p0.x(), qMax(p1x, p2.x()));
    int maxY = qMax(p0.y(), qMax(p1y, p2.y()));
    return QRect(minX - penW, minY - penH, maxX - minX + 2 * penW + 1, maxY - minY + 2 * penH + 1);
}

void CurveTool::registerTool()
{
    Tool::registerTool();

    button_->setIcon(QIcon(":/curve.png"));
    button_->setCheckable(true);

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void CurveTool::activate()
{
    phase_ = 0;
    delete undoBuffer_;
    undoBuffer_ = nullptr;
    Tool::activate();
}

void CurveTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 2, 5);
}
