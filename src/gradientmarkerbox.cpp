#include <QDataStream>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMap>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include "buffer.h"
#include "gradientmarkerbox.h"
#include "gradientrenderer.h"

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
    int previewY = kMarkerRowHeight + 2;
    if (range_ && buffer_ && !range_->markers().isEmpty()) {
        const QImage &image = buffer_->image();
        for (int y = 0; y < kPreviewHeight; y++) {
            for (int x = 0; x < W; x++) {
                float t = float(x) / float(W);
                int idx = GradientRenderer::colorIndex(t, x, y, range_, image);
                p.fillRect(x, previewY + y, 1, 1, colorForIndex(idx));
            }
        }
    } else {
        p.fillRect(0, previewY, W, kPreviewHeight, QColor(30, 30, 30));
    }
}

void GradientMarkerBox::saveSlotState(int slot)
{
    if (dragSavedStates_.contains(slot)) return;
    const auto &markers = range_->markers();
    for (const auto &m : markers) {
        if (m.slot == slot) {
            dragSavedStates_[slot] = {true, m.colorIndex, m.abrupt};
            return;
        }
    }
    dragSavedStates_[slot] = {false, -1, false};
}

void GradientMarkerBox::mousePressEvent(QMouseEvent *event)
{
    if (!range_) return;
    int slot = slotAt(event->pos().x());
    if (event->button() == Qt::LeftButton) {
        int colorIndex = buffer_ ? static_cast<int>(buffer_->paintColor()) : 0;
        dragging_ = true;
        dragStartSlot_ = slot;
        dragBaseColor_ = colorIndex;
        dragSavedStates_.clear();
        saveSlotState(slot);
        range_->addMarker(slot, colorIndex);
        emit rangeChanged();
        update();
    } else if (event->button() == Qt::RightButton) {
        dragging_ = false;
        dragSavedStates_.clear();
        range_->removeMarker(slot);
        emit rangeChanged();
        update();
    }
}

void GradientMarkerBox::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragging_ || !range_) return;

    int currentSlot = slotAt(event->pos().x());
    int minSlot = qMin(dragStartSlot_, currentSlot);
    int maxSlot = qMax(dragStartSlot_, currentSlot);
    int paletteSize = buffer_ ? buffer_->image().colorCount() : 1;
    if (paletteSize < 1) return;

    // Save original state for any slot we're about to overwrite for the first time
    for (int s = minSlot; s <= maxSlot; s++)
        saveSlotState(s);

    // Set markers for all slots in the drag range
    for (int s = minSlot; s <= maxSlot; s++) {
        int colorIndex = qBound(0, dragBaseColor_ + qAbs(s - dragStartSlot_), paletteSize - 1);
        range_->addMarker(s, colorIndex);
    }

    // Restore slots that were previously set but are now outside the range
    for (auto it = dragSavedStates_.cbegin(); it != dragSavedStates_.cend(); ++it) {
        int s = it.key();
        if (s >= minSlot && s <= maxSlot) continue;
        if (it.value().hadMarker)
            range_->addMarker(s, it.value().colorIndex, it.value().abrupt);
        else
            range_->removeMarker(s);
    }

    emit rangeChanged();
    update();
}

void GradientMarkerBox::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
        dragSavedStates_.clear();
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
