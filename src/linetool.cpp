#include <algorithm>
#include <cmath>
#include <QImage>
#include <QRect>
#include <QVector>
#include <QGridLayout>
#include <QtMath>
#include "pen.h"
#include "buffer.h"
#include "undobuffer.h"
#include "algorithms.h"
#include "linetool.h"
#include "gradientrange.h"
#include "gradientrenderer.h"

LineTool LineTool::instance;

LineTool::LineTool(QObject *parent) : Tool(parent),
    mode_(Line), undoBuffer_(nullptr),
    active_(false), dragUndoBuffer_(nullptr)
{
}

void LineTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr) {
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }

    resetConnectedState();
    Tool::setBuffer(buffer);

    if (buffer_ != nullptr) {
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    }
}

void LineTool::resetConnectedState()
{
    active_ = false;
    vertices_.clear();
    delete dragUndoBuffer_;
    dragUndoBuffer_ = nullptr;
}

void LineTool::cancel()
{
    if (mode_ == Line) {
        if (!undoBuffer_) return;
        undoBuffer_->apply(buffer_);
        buffer_->notifyModified(buffer_->image().rect());
        delete undoBuffer_;
        undoBuffer_ = nullptr;
    } else {
        if (dragUndoBuffer_) {
            dragUndoBuffer_->apply(buffer_);
            buffer_->notifyModified(buffer_->image().rect());
        }
        resetConnectedState();
    }
}

QRect LineTool::doubleClick(const QPoint &point)
{
    Q_UNUSED(point)
    if (mode_ == Line || !active_)
        return QRect();

    delete dragUndoBuffer_;
    dragUndoBuffer_ = nullptr;

    if (mode_ == FilledPolygon) {
        active_ = false;
        if (lastPoint_ == firstPoint_) {
            vertices_.clear();
            return QRect();
        }
        QRect changedRect;
        Algorithms::line(lastPoint_, firstPoint_, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(paintPixel(p));
        });
        changedRect = changedRect.united(polygonFill());
        vertices_.clear();
        return changedRect;
    }

    // ConnectedLines
    active_ = false;
    vertices_.clear();
    return QRect();
}

// ── Single line ────────────────────────────────────────────────────────────

QRect LineTool::press(const QPoint &point, const Qt::KeyboardModifiers &)
{
    if (mode_ == Line) {
        startPoint_ = point;
        QRect rect = buffer_->pen()->rect(point);
        undoBuffer_ = new UndoBuffer(rect.topLeft(), buffer_->image().copy(rect), this);
        return drawPixel(point);
    }

    // Connected lines / filled polygon
    if (mouseButton_ == Qt::RightButton) {
        delete dragUndoBuffer_;
        dragUndoBuffer_ = nullptr;
        if (mode_ == FilledPolygon && active_) {
            active_ = false;

            if (lastPoint_ == firstPoint_) {
                vertices_.clear();
                return QRect();
            }

            QRect changedRect;
            Algorithms::line(lastPoint_, firstPoint_, [this, &changedRect](const QPoint &p) {
                changedRect = changedRect.united(paintPixel(p));
            });
            changedRect = changedRect.united(polygonFill());
            vertices_.clear();
            return changedRect;
        }
        active_ = false;
        vertices_.clear();
        return QRect();
    }

    // Left button
    if (!active_) {
        active_ = true;
        lastPoint_ = point;
        if (mode_ == FilledPolygon) {
            firstPoint_ = point;
            vertices_.clear();
            vertices_.append(point);
            return paintPixel(point);
        }
        return drawPixel(point);
    }

    return QRect();
}

QRect LineTool::move(const QPoint &point)
{
    if (mode_ == Line) {
        if (mouseButton_ == Qt::NoButton)
            return buffer_->pen()->paint(point, buffer_);

        if (!undoBuffer_)
            return QRect();

        currentPoint_ = point;
        undoBuffer_->apply(buffer_);
        delete undoBuffer_;

        QRect changedRect;
        Algorithms::line(startPoint_, point, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(buffer_->pen()->rect(p));
        });
        undoBuffer_ = new UndoBuffer(changedRect.topLeft(), buffer_->image().copy(changedRect), this);
        Algorithms::line(startPoint_, point, [this](const QPoint &p) { drawPixel(p); });
        return changedRect;
    }

    // Connected lines / filled polygon
    if (!active_)
        return buffer_->pen()->paint(point, buffer_);

    if (mouseButton_ != Qt::NoButton) {
        if (dragUndoBuffer_ != nullptr) {
            dragUndoBuffer_->apply(buffer_);
            delete dragUndoBuffer_;
            dragUndoBuffer_ = nullptr;
        }
        QRect area = lineBoundingRect(lastPoint_, point).intersected(buffer_->image().rect());
        if (!area.isEmpty()) {
            dragUndoBuffer_ = new UndoBuffer(area.topLeft(), buffer_->image().copy(area), this);
            Algorithms::line(lastPoint_, point, [this](const QPoint &p) {
                mode_ == ConnectedLines ? drawPixel(p) : paintPixel(p);
            });
        }
        return area;
    }

    // No button: hover preview
    QRect changedRect;
    Algorithms::line(lastPoint_, point, [this, &changedRect](const QPoint &p) {
        changedRect = changedRect.united(mode_ == ConnectedLines ? drawPixel(p) : paintPixel(p));
    });
    return changedRect;
}

QRect LineTool::release(const QPoint &point)
{
    if (mode_ == Line) {
        if (!undoBuffer_)
            return QRect();

        undoBuffer_->apply(buffer_);
        delete undoBuffer_;
        undoBuffer_ = nullptr;

        QRect changedRect;
        Algorithms::line(startPoint_, point, [this, &changedRect](const QPoint &p) {
            changedRect = changedRect.united(drawPixel(p));
        });
        return changedRect;
    }

    // Connected lines / filled polygon
    if (dragUndoBuffer_ != nullptr) {
        dragUndoBuffer_->apply(buffer_);
        delete dragUndoBuffer_;
        dragUndoBuffer_ = nullptr;
    }

    if (!active_ || mouseButton_ != Qt::LeftButton)
        return QRect();

    QRect changedRect;
    Algorithms::line(lastPoint_, point, [this, &changedRect](const QPoint &p) {
        changedRect = changedRect.united(mode_ == ConnectedLines ? drawPixel(p) : paintPixel(p));
    });
    lastPoint_ = point;
    if (mode_ == FilledPolygon)
        vertices_.append(point);
    return changedRect;
}

QRect LineTool::hover(const QPoint &point)
{
    if (mode_ == Line || !active_)
        return buffer_->pen()->rect(point);
    return lineBoundingRect(lastPoint_, point);
}

QString LineTool::status() const
{
    if (mode_ != Line || mouseButton_ == Qt::NoButton || !undoBuffer_)
        return QString();
    int dx = currentPoint_.x() - startPoint_.x();
    int dy = currentPoint_.y() - startPoint_.y();
    double angle = qRadiansToDegrees(std::atan2(static_cast<double>(dy), static_cast<double>(dx)));
    if (angle < 0) angle += 360.0;
    double length = std::sqrt(static_cast<double>(dx * dx + dy * dy));
    return QString("%1° %2px").arg(angle, 0, 'f', 1).arg(qRound(length));
}

// ── Helpers ────────────────────────────────────────────────────────────────

QRect LineTool::drawPixel(const QPoint &point)
{
    if (mouseButton_ == Qt::RightButton)
        return buffer_->pen()->erase(point, buffer_);
    return buffer_->pen()->paint(point, buffer_);
}

QRect LineTool::paintPixel(const QPoint &point)
{
    return buffer_->pen()->paint(point, buffer_);
}

QRect LineTool::lineBoundingRect(const QPoint &from, const QPoint &to) const
{
    QRect penRect = buffer_->pen()->rect(QPoint(0, 0));
    int penW = penRect.width();
    int penH = penRect.height();
    return QRect(from, to).normalized().adjusted(-penW, -penH, penW, penH);
}

QRect LineTool::polygonFill()
{
    if (vertices_.size() < 3)
        return QRect();

    QImage &image = buffer_->image();
    const QRect imageRect = image.rect();
    const int fillColor = static_cast<int>(buffer_->paintColor());
    const int n = vertices_.size();

    int minY = imageRect.bottom(), maxY = imageRect.top();
    for (const QPoint &v : vertices_) {
        minY = qMin(minY, v.y());
        maxY = qMax(maxY, v.y());
    }
    minY = qMax(minY, imageRect.top());
    maxY = qMin(maxY, imageRect.bottom());

    const bool useGradient = gradientFillActive();
    const GradientRange *range = useGradient ? &gradientRanges[activeGradientRange] : nullptr;

    QRect changedRect;
    for (int y = minY; y <= maxY; y++) {
        QVector<int> xs;
        for (int i = 0; i < n; i++) {
            const QPoint &p1 = vertices_[i];
            const QPoint &p2 = vertices_[(i + 1) % n];
            if ((p1.y() <= y && p2.y() > y) || (p2.y() <= y && p1.y() > y)) {
                int x = p1.x() + (y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y());
                xs.append(x);
            }
        }
        std::sort(xs.begin(), xs.end());
        for (int i = 0; i + 1 < xs.size(); i += 2) {
            int x1 = qMax(xs[i], imageRect.left());
            int x2 = qMin(xs[i + 1], imageRect.right());
            for (int x = x1; x <= x2; x++) {
                if (useGradient) {
                    float t = GradientRenderer::computeT(x, y, image.width(), image.height(),
                                                          activeGradientFillMode, firstPoint_, lastPoint_);
                    int ci = GradientRenderer::colorIndex(t, x, y, range, image);
                    image.setPixel(x, y, static_cast<uint>(ci));
                } else {
                    image.setPixel(x, y, static_cast<uint>(fillColor));
                }
            }
            if (x1 <= x2)
                changedRect = changedRect.united(QRect(x1, y, x2 - x1 + 1, 1));
        }
    }
    return changedRect;
}

// ── Tool registration / activation ────────────────────────────────────────

void LineTool::updateButton()
{
    static const struct { const char *icon; const char *tip; } kModes[] = {
        { ":/line.png",           "Line [W]" },
        { ":/connectedlines.png", "Connected Lines [W]" },
        { ":/filledlines.png",    "Connected Lines \xe2\x80\x93 Filled Polygon [W]" },
    };
    button_->setIcon(QIcon(kModes[mode_].icon));
    button_->setToolTip(kModes[mode_].tip);
}

void LineTool::registerTool()
{
    Tool::registerTool();

    button_->setCheckable(true);
    updateButton();

    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void LineTool::activate()
{
    resetConnectedState();
    delete undoBuffer_;
    undoBuffer_ = nullptr;

    if (buffer_->tool() == this) {
        mode_ = static_cast<Mode>((mode_ + 1) % 3);
        updateButton();
        button_->setChecked(true);
    }
    Tool::activate();
}

void LineTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 0, 3);
}
