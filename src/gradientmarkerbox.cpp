#include <QDataStream>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include "buffer.h"
#include "gradientmarkerbox.h"

GradientMarkerBox::GradientMarkerBox(QWidget *parent) : QWidget(parent)
{
    setAcceptDrops(true);
    setFixedHeight(kMarkerRowHeight + 2 + kPreviewHeight);
}

void GradientMarkerBox::setRange(GradientRange *range)
{
    range_ = range;
    update();
}

void GradientMarkerBox::setBuffer(Buffer *buffer)
{
    buffer_ = buffer;
    update();
}

void GradientMarkerBox::setShowPreview(bool show)
{
    showPreview_ = show;
    setFixedHeight(kMarkerRowHeight + (show ? 2 + kPreviewHeight : 0));
    update();
}

QSize GradientMarkerBox::sizeHint() const
{
    return QSize(512, kMarkerRowHeight + 2 + kPreviewHeight);
}

int GradientMarkerBox::slotAt(int x) const
{
    return qBound(0, x * kSlotCount / qMax(width(), 1), kSlotCount - 1);
}

int GradientMarkerBox::slotX(int slot) const
{
    return slot * width() / kSlotCount;
}

QColor GradientMarkerBox::colorForIndex(int colorIndex) const
{
    if (!buffer_ || colorIndex < 0 || colorIndex >= buffer_->image().colorCount())
        return Qt::white;
    return QColor(buffer_->image().color(colorIndex));
}

QColor GradientMarkerBox::interpolatedColor(float slotPos, int pixelX) const
{
    if (!range_) return Qt::transparent;
    const auto &markers = range_->markers();
    if (markers.isEmpty()) return Qt::transparent;

    const bool hardEdges = range_->hardEdges();
    const bool random    = range_->random();
    const float dither   = range_->ditherAmount() / 100.0f;

    if (slotPos <= markers.first().slot)
        return colorForIndex(markers.first().colorIndex);
    if (slotPos >= markers.last().slot)
        return colorForIndex(markers.last().colorIndex);

    for (int i = 0; i < markers.size() - 1; i++) {
        if (slotPos >= markers[i].slot && slotPos < markers[i + 1].slot) {
            if (markers[i].abrupt || hardEdges)
                return colorForIndex(markers[i].colorIndex);

            float t = (slotPos - markers[i].slot) / float(markers[i + 1].slot - markers[i].slot);

            if (random && dither > 0.0f) {
                // Ordered dither: use a repeating 8-level threshold pattern
                // so the preview is deterministic and visually clear.
                static const int pattern[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
                float threshold = (pattern[pixelX % 8] + 0.5f) / 8.0f;
                // Scale t by dither so at dither=0 we get smooth, at 1.0 fully dithered
                float dt = (t - 0.5f) / dither + 0.5f;
                dt = qBound(0.0f, dt, 1.0f);
                return colorForIndex(dt >= threshold ? markers[i + 1].colorIndex : markers[i].colorIndex);
            }

            QColor c1 = colorForIndex(markers[i].colorIndex);
            QColor c2 = colorForIndex(markers[i + 1].colorIndex);
            return QColor(
                qRound(c1.red()   + t * (c2.red()   - c1.red())),
                qRound(c1.green() + t * (c2.green() - c1.green())),
                qRound(c1.blue()  + t * (c2.blue()  - c1.blue()))
            );
        }
    }
    return colorForIndex(markers.last().colorIndex);
}

void GradientMarkerBox::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    const int W = width();

    // Marker row background
    p.fillRect(0, 0, W, kMarkerRowHeight, QColor(30, 30, 30));

    // Dot indicators for empty slots
    p.setPen(QColor(80, 80, 80));
    for (int s = 0; s < kSlotCount; s++) {
        int x = slotX(s) + (slotX(s + 1) - slotX(s)) / 2;
        p.drawPoint(x, kMarkerRowHeight / 2);
    }

    // Draw markers
    if (range_) {
        for (const auto &m : range_->markers()) {
            int x1 = slotX(m.slot);
            int x2 = slotX(m.slot + 1);
            p.fillRect(x1, 0, x2 - x1, kMarkerRowHeight, colorForIndex(m.colorIndex));
            if (m.abrupt) {
                p.setPen(Qt::white);
                p.drawLine(x2 - 1, 0, x2 - 1, kMarkerRowHeight - 1);
            }
        }
    }

    // Preview bar
    if (showPreview_) {
        int previewY = kMarkerRowHeight + 2;
        if (range_ && !range_->markers().isEmpty()) {
            for (int x = 0; x < W; x++) {
                float slotPos = float(x) / W * (kSlotCount - 1);
                p.fillRect(x, previewY, 1, kPreviewHeight, interpolatedColor(slotPos, x));
            }
        } else {
            p.fillRect(0, previewY, W, kPreviewHeight, QColor(30, 30, 30));
        }
    }
}

void GradientMarkerBox::mousePressEvent(QMouseEvent *event)
{
    if (!range_) return;
    int slot = slotAt(event->pos().x());
    if (event->button() == Qt::LeftButton) {
        int colorIndex = buffer_ ? static_cast<int>(buffer_->paintColor()) : 0;
        range_->addMarker(slot, colorIndex);
        emit rangeChanged();
        update();
    } else if (event->button() == Qt::RightButton) {
        range_->removeMarker(slot);
        emit rangeChanged();
        update();
    }
}

void GradientMarkerBox::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!range_ || event->button() != Qt::LeftButton) return;
    int slot = slotAt(event->pos().x());
    // Toggle abrupt flag on the marker at this slot, or add an abrupt marker
    const auto &markers = range_->markers();
    for (int i = 0; i < markers.size(); i++) {
        if (markers[i].slot == slot) {
            int colorIndex = markers[i].colorIndex;
            bool abrupt = !markers[i].abrupt;
            range_->removeMarker(slot);
            range_->addMarker(slot, colorIndex, abrupt);
            emit rangeChanged();
            update();
            return;
        }
    }
    // No marker here — add an abrupt one
    int colorIndex = buffer_ ? static_cast<int>(buffer_->paintColor()) : 0;
    range_->addMarker(slot, colorIndex, true);
    emit rangeChanged();
    update();
}

void GradientMarkerBox::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-palette-index"))
        event->acceptProposedAction();
}

void GradientMarkerBox::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-palette-index"))
        event->acceptProposedAction();
}

void GradientMarkerBox::dropEvent(QDropEvent *event)
{
    if (!range_ || !event->mimeData()->hasFormat("application/x-palette-index"))
        return;
    QByteArray data = event->mimeData()->data("application/x-palette-index");
    QDataStream stream(&data, QIODevice::ReadOnly);
    unsigned colorIndex;
    stream >> colorIndex;
    int slot = slotAt(event->position().toPoint().x());
    range_->addMarker(slot, static_cast<int>(colorIndex));
    event->acceptProposedAction();
    emit rangeChanged();
    update();
}
