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

GradientMarkerBox::DitherPair GradientMarkerBox::ditherPair(const QColor &ideal) const
{
    if (!buffer_ || buffer_->image().colorCount() < 2)
        return {0, 0, 0.0f};

    const QVector<QRgb> &table = buffer_->image().colorTable();
    int idx1 = 0, idx2 = 0;
    int dist1 = INT_MAX, dist2 = INT_MAX;

    for (int i = 0; i < table.size(); i++) {
        int dr = ideal.red()   - qRed(table[i]);
        int dg = ideal.green() - qGreen(table[i]);
        int db = ideal.blue()  - qBlue(table[i]);
        int dist = dr*dr + dg*dg + db*db;
        if (dist < dist1) {
            idx2 = idx1; dist2 = dist1;
            idx1 = i;    dist1 = dist;
        } else if (dist < dist2) {
            idx2 = i; dist2 = dist;
        }
    }

    // blend: 0 = all idx1, 1 = all idx2
    float total = float(dist1 + dist2);
    float blend = total > 0.0f ? float(dist1) / total : 0.0f;
    return {idx1, idx2, blend};
}

QColor GradientMarkerBox::interpolatedColor(float slotPos, int pixelX) const
{
    if (!range_) return Qt::transparent;
    const auto &markers = range_->markers();
    if (markers.isEmpty()) return Qt::transparent;

    const bool hardEdges = range_->hardEdges();
    const bool isRandom  = range_->random();
    const int  ditherAmt = range_->ditherAmount();

    if (slotPos <= markers.first().slot)
        return colorForIndex(markers.first().colorIndex);
    if (slotPos >= markers.last().slot)
        return colorForIndex(markers.last().colorIndex);

    for (int i = 0; i < markers.size() - 1; i++) {
        if (slotPos >= markers[i].slot && slotPos < markers[i + 1].slot) {
            if (markers[i].abrupt || hardEdges)
                return colorForIndex(markers[i].colorIndex);

            float t = (slotPos - markers[i].slot) / float(markers[i + 1].slot - markers[i].slot);

            QColor c1 = colorForIndex(markers[i].colorIndex);
            QColor c2 = colorForIndex(markers[i + 1].colorIndex);
            QColor ideal(
                qRound(c1.red()   + t * (c2.red()   - c1.red())),
                qRound(c1.green() + t * (c2.green() - c1.green())),
                qRound(c1.blue()  + t * (c2.blue()  - c1.blue()))
            );

            DitherPair dp = ditherPair(ideal);

            if (isRandom) {
                if (ditherAmt == 0)
                    return colorForIndex(dp.idx1);
                // Narrow the dither zone at lower amounts
                float scaled = (dp.blend - 0.5f) * (100.0f / ditherAmt) + 0.5f;
                scaled = qBound(0.0f, scaled, 1.0f);
                float threshold = ((pixelX * 1103515245 + 12345) & 0xFF) / 255.0f;
                return colorForIndex(scaled > threshold ? dp.idx2 : dp.idx1);
            } else {
                // Ordered (Bayer) dithering — always full strength
                static const int pattern[8] = {0, 4, 2, 6, 1, 5, 3, 7};
                float threshold = (pattern[pixelX % 8] + 0.5f) / 8.0f;
                return colorForIndex(dp.blend > threshold ? dp.idx2 : dp.idx1);
            }
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
