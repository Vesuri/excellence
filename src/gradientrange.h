#ifndef GRADIENTRANGE_H
#define GRADIENTRANGE_H

#include <QString>
#include <QVector>
#include <QRect>
#include <cstdint>
#include <utility>
#include "buffer.h"

struct GradientMarker {
    int slot = 0;
    int colorIndex = 0;
    bool abrupt = false;
};

class GradientRange
{
public:
    const QVector<GradientMarker> &markers() const { return markers_; }

    void addMarker(int slot, int colorIndex, bool abrupt = false);
    void removeMarker(int slot);

    int spread() const { return spread_; }
    void setSpread(int spread) { spread_ = qBound(0, spread, 254); }
    int colorCount() const;

    bool random() const { return random_; }
    void setRandom(bool v) { random_ = v; }
    bool hardEdges() const { return hardEdges_; }
    void setHardEdges(bool v) { hardEdges_ = v; }
    int ditherAmount() const { return ditherAmount_; }
    void setDitherAmount(int v) { ditherAmount_ = qBound(0, v, 100); }

    bool cycling() const { return cycling_; }
    void setCycling(bool v) { cycling_ = v; }
    int cycleSpeed() const { return cycleSpeed_; }
    void setCycleSpeed(int v) { cycleSpeed_ = qBound(0, v, 71); }

    void setDefault(const QImage &image);

    void flip();
    void clear();
    void undo();
    void restore();
    void setRestorePoint();

private:
    struct Snapshot {
        QVector<GradientMarker> markers;
        int spread = 0;
        bool random = false;
        bool hardEdges = false;
        int ditherAmount = 0;
    };
    void takeUndoSnapshot();
    Snapshot currentSnapshot() const;
    void applySnapshot(const Snapshot &s);

    QVector<GradientMarker> markers_;
    int spread_ = 0;
    bool random_ = false;
    bool hardEdges_ = false;
    int ditherAmount_ = 0;
    bool cycling_ = false;
    int cycleSpeed_ = 0;
    Snapshot undoSnapshot_;
    Snapshot restorePoint_;
};

static const int kGradientRangeCount = 8;
static const int kGradientSlotCount = 128;
extern GradientRange gradientRanges[kGradientRangeCount];
extern int activeGradientRange;

enum GradientFillMode {
    FillFlat = 0,
    FillHorizontal,
    FillVertical,
    FillLinear,
    FillRadial,
    FillSpherical,
    FillHighlight
};
extern GradientFillMode activeGradientFillMode;
extern bool drawModeActive;
extern bool conformFill;
extern bool centerFill;

// Applies random dither to a gradient slot position. The noise displacement is
// proportional to span and ditherAmt (0–100), bounded to [minSlot, maxSlot].
inline float applyRandomDither(float slotPos, float span, int ditherAmt,
                                int pixelX, int pixelY,
                                float minSlot, float maxSlot)
{
    uint32_t h = uint32_t(pixelX) * 2246822519u ^ uint32_t(pixelY) * 3266489917u;
    h = (h ^ (h >> 17)) * 0x45d9f3bu;
    h ^= h >> 16;
    float noise = (h & 0xFFu) / 256.0f - 0.5f;
    return qBound(minSlot, slotPos + noise * span * (ditherAmt / 100.0f), maxSlot);
}

inline bool gradientFillIsRadial(GradientFillMode mode)
{
    return mode == FillRadial || mode == FillSpherical || mode == FillHighlight;
}

// True when the active fill mode requires a rubber band after the shape is drawn
// so the user can pick the gradient direction (Linear) or center (Radial/etc.).
inline bool gradientNeedsRubberBand()
{
    return activeGradientFillMode == FillLinear
        || (gradientFillIsRadial(activeGradientFillMode) && !centerFill);
}

inline QString gradientFillModeName(GradientFillMode mode)
{
    switch (mode) {
    case FillFlat:       return "Color";
    case FillHorizontal: return "Horizontal";
    case FillVertical:   return "Vertical";
    case FillLinear:     return "Linear";
    case FillRadial:     return "Radial";
    case FillSpherical:  return "Spherical";
    case FillHighlight:  return "Highlight";
    }
    return {};
}

// Returns true when draw mode is on, a gradient fill mode is selected, and the
// active range has markers.
inline bool gradientFillActive()
{
    return drawModeActive
        && activeGradientFillMode != FillFlat
        && !gradientRanges[activeGradientRange].markers().isEmpty();
}

// Computes gradient from/to endpoints for a polygon or path fill.
// For HV modes the gradient spans the shape's bounding box; for other modes
// the caller's drag start/end points are used as-is.
inline std::pair<QPoint, QPoint> gradientEndpoints(
    const QRect &bbox, const QPoint &startFallback, const QPoint &endFallback)
{
    const bool hvMode = activeGradientFillMode == FillHorizontal
                     || activeGradientFillMode == FillVertical;
    QPoint from = hvMode ? bbox.topLeft()     : startFallback;
    if (centerFill && gradientFillIsRadial(activeGradientFillMode))
        from = bbox.center();
    const QPoint to = hvMode ? bbox.bottomRight() : endFallback;
    return {from, to};
}

#endif // GRADIENTRANGE_H
