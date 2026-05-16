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

    // Project ideal onto the c1→c2 axis to get the true linear blend needed.
    // blend = 0 means use all idx1, blend = 1 means use all idx2.
    int dr = qRed(table[idx2])   - qRed(table[idx1]);
    int dg = qGreen(table[idx2]) - qGreen(table[idx1]);
    int db = qBlue(table[idx2])  - qBlue(table[idx1]);
    int er = ideal.red()   - qRed(table[idx1]);
    int eg = ideal.green() - qGreen(table[idx1]);
    int eb = ideal.blue()  - qBlue(table[idx1]);
    float denom = float(dr*dr + dg*dg + db*db);
    float blend = denom > 0.0f ? qBound(0.0f, float(er*dr + eg*dg + eb*db) / denom, 1.0f) : 0.0f;
    return {idx1, idx2, blend};
}

QColor GradientMarkerBox::interpolatedColor(float slotPos, int pixelX, int pixelY) const
{
    if (!range_) return Qt::transparent;
    const auto &markers = range_->markers();
    if (markers.isEmpty()) return Qt::transparent;

    const bool hardEdges = range_->hardEdges();
    const bool isRandom  = range_->random();
    const int  ditherAmt = range_->ditherAmount();

    // Random mode: jitter the sample position along the gradient before lookup.
    // Each pixel independently samples from a randomly displaced slot position,
    // producing the "scattered pixel" appearance. The displacement scales with
    // the full gradient span so dither=100 can reach anywhere in the gradient.
    if (isRandom && ditherAmt > 0 && markers.size() >= 2) {
        uint32_t h = uint32_t(pixelX) * 2246822519u ^ uint32_t(pixelY) * 3266489917u;
        h = (h ^ (h >> 17)) * 0x45d9f3bu;
        h ^= h >> 16;
        float noise = (h & 0xFFu) / 256.0f - 0.5f;  // -0.5..+0.5
        float span  = markers.last().slot - markers.first().slot;
        slotPos = qBound(float(markers.first().slot),
                         slotPos + noise * span * (ditherAmt / 100.0f),
                         float(markers.last().slot));
    }

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

            if (isRandom) {
                // Jitter already applied — snap to nearest palette color
                return colorForIndex(ditherPair(ideal).idx1);
            }

            DitherPair dp = ditherPair(ideal);

            // clang-format off
            static const int bayer16[16][16] = {
                {  0,128, 32,160,  8,136, 40,168,  2,130, 34,162, 10,138, 42,170},
                {192, 64,224, 96,200, 72,232,104,194, 66,226, 98,202, 74,234,106},
                { 48,176, 16,144, 56,184, 24,152, 50,178, 18,146, 58,186, 26,154},
                {240,112,208, 80,248,120,216, 88,242,114,210, 82,250,122,218, 90},
                { 12,140, 44,172,  4,132, 36,164, 14,142, 46,174,  6,134, 38,166},
                {204, 76,236,108,196, 68,228,100,206, 78,238,110,198, 70,230,102},
                { 60,188, 28,156, 52,180, 20,148, 62,190, 30,158, 54,182, 22,150},
                {252,124,220, 92,244,116,212, 84,254,126,222, 94,246,118,214, 86},
                {  3,131, 35,163, 11,139, 43,171,  1,129, 33,161,  9,137, 41,169},
                {195, 67,227, 99,203, 75,235,107,193, 65,225, 97,201, 73,233,105},
                { 51,179, 19,147, 59,187, 27,155, 49,177, 17,145, 57,185, 25,153},
                {243,115,211, 83,251,123,219, 91,241,113,209, 81,249,121,217, 89},
                { 15,143, 47,175,  7,135, 39,167, 13,141, 45,173,  5,133, 37,165},
                {207, 79,239,111,199, 71,231,103,205, 77,237,109,197, 69,229,101},
                { 63,191, 31,159, 55,183, 23,151, 61,189, 29,157, 53,181, 21,149},
                {255,127,223, 95,247,119,215, 87,253,125,221, 93,245,117,213, 85}
            };
            // clang-format on

            // Ordered (Bayer 16x16) dithering — always full strength
            float threshold = (bayer16[pixelY % 16][pixelX % 16] + 0.5f) / 256.0f;
            return colorForIndex(dp.blend > threshold ? dp.idx2 : dp.idx1);
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
        float firstSlot = range_->markers().first().slot;
        float lastSlot  = range_->markers().last().slot;
        for (int y = 0; y < kPreviewHeight; y++) {
            for (int x = 0; x < W; x++) {
                float slotPos = firstSlot + float(x) / W * (lastSlot - firstSlot);
                p.fillRect(x, previewY + y, 1, 1, interpolatedColor(slotPos, x, y));
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
