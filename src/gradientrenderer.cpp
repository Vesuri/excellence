#include "gradientrenderer.h"
#include <QColor>
#include <climits>
#include <cmath>

namespace GradientRenderer {

static QColor colorAt(int idx, const QImage &image)
{
    if (idx < 0 || idx >= image.colorCount()) return Qt::white;
    return QColor(image.color(idx));
}

struct DitherPair { int idx1; int idx2; float blend; };

static DitherPair ditherPair(const QColor &ideal, const QImage &image)
{
    const int n = image.colorCount();
    if (n < 2) return {0, 0, 0.0f};

    int idx1 = 0, idx2 = 0;
    int dist1 = INT_MAX, dist2 = INT_MAX;

    for (int i = 0; i < n; i++) {
        QColor c(image.color(i));
        int dr = ideal.red()   - c.red();
        int dg = ideal.green() - c.green();
        int db = ideal.blue()  - c.blue();
        int dist = dr*dr + dg*dg + db*db;
        if (dist < dist1) {
            idx2 = idx1; dist2 = dist1;
            idx1 = i;    dist1 = dist;
        } else if (dist < dist2) {
            idx2 = i; dist2 = dist;
        }
    }

    QColor c1(image.color(idx1));
    QColor c2(image.color(idx2));
    int dr = c2.red()   - c1.red();
    int dg = c2.green() - c1.green();
    int db = c2.blue()  - c1.blue();
    int er = ideal.red()   - c1.red();
    int eg = ideal.green() - c1.green();
    int eb = ideal.blue()  - c1.blue();
    float denom = float(dr*dr + dg*dg + db*db);
    float blend = denom > 0.0f ? qBound(0.0f, float(er*dr + eg*dg + eb*db) / denom, 1.0f) : 0.0f;
    return {idx1, idx2, blend};
}

int colorIndex(float t, int pixelX, int pixelY,
               const GradientRange *range, const QImage &image)
{
    if (!range || image.colorCount() == 0) return 0;
    const auto &markers = range->markers();
    if (markers.isEmpty()) return 0;
    if (markers.size() == 1) return markers.first().colorIndex;

    const bool hardEdges = range->hardEdges();
    const bool isRandom  = range->random();
    const int  ditherAmt = range->ditherAmount();

    float span    = float(markers.last().slot - markers.first().slot);
    float slotPos = markers.first().slot + t * span;

    if (isRandom && ditherAmt > 0) {
        uint32_t h = uint32_t(pixelX) * 2246822519u ^ uint32_t(pixelY) * 3266489917u;
        h = (h ^ (h >> 17)) * 0x45d9f3bu;
        h ^= h >> 16;
        float noise = (h & 0xFFu) / 256.0f - 0.5f;
        slotPos = qBound(float(markers.first().slot),
                         slotPos + noise * span * (ditherAmt / 100.0f),
                         float(markers.last().slot));
    }

    if (slotPos <= markers.first().slot)
        return markers.first().colorIndex;
    if (slotPos >= markers.last().slot)
        return markers.last().colorIndex;

    for (int i = 0; i < markers.size() - 1; i++) {
        if (slotPos >= markers[i].slot && slotPos < markers[i + 1].slot) {
            if (markers[i].abrupt || hardEdges)
                return markers[i].colorIndex;

            float seg_t = (slotPos - markers[i].slot) / float(markers[i + 1].slot - markers[i].slot);
            QColor c1 = colorAt(markers[i].colorIndex, image);
            QColor c2 = colorAt(markers[i + 1].colorIndex, image);
            QColor ideal(
                qRound(c1.red()   + seg_t * (c2.red()   - c1.red())),
                qRound(c1.green() + seg_t * (c2.green() - c1.green())),
                qRound(c1.blue()  + seg_t * (c2.blue()  - c1.blue()))
            );

            if (isRandom)
                return ditherPair(ideal, image).idx1;

            DitherPair dp = ditherPair(ideal, image);

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

            float threshold = (bayer16[pixelY % 16][pixelX % 16] + 0.5f) / 256.0f;
            return dp.blend > threshold ? dp.idx2 : dp.idx1;
        }
    }
    return markers.last().colorIndex;
}

float computeT(int px, int py, int imageW, int imageH,
               GradientFillMode mode, const QPoint &from, const QPoint &to)
{
    switch (mode) {
    case FillHorizontal:
        return float(px) / qMax(imageW - 1, 1);
    case FillVertical:
        return float(py) / qMax(imageH - 1, 1);
    case FillLinear: {
        float dx = float(to.x() - from.x());
        float dy = float(to.y() - from.y());
        float lenSq = dx * dx + dy * dy;
        if (lenSq < 1.0f) return 0.0f;
        float rx = float(px - from.x());
        float ry = float(py - from.y());
        return qBound(0.0f, (rx * dx + ry * dy) / lenSq, 1.0f);
    }
    case FillRadial: {
        float dx = float(to.x() - from.x());
        float dy = float(to.y() - from.y());
        float radius = sqrtf(dx * dx + dy * dy);
        if (radius < 1.0f) return 0.0f;
        float rx = float(px - from.x());
        float ry = float(py - from.y());
        return qBound(0.0f, sqrtf(rx * rx + ry * ry) / radius, 1.0f);
    }
    case FillSpherical:
    case FillHighlight: {
        float dx = float(to.x() - from.x());
        float dy = float(to.y() - from.y());
        float radius = sqrtf(dx * dx + dy * dy);
        if (radius < 1.0f) return 0.0f;
        float rx = float(px - from.x());
        float ry = float(py - from.y());
        float raw = qBound(0.0f, sqrtf(rx * rx + ry * ry) / radius, 1.0f);
        return sinf(raw * float(M_PI) / 2.0f);
    }
    default:
        return 0.0f;
    }
}

} // namespace GradientRenderer
