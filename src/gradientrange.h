#ifndef GRADIENTRANGE_H
#define GRADIENTRANGE_H

#include <QString>
#include <QVector>
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

inline bool gradientFillIsRadial(GradientFillMode mode)
{
    return mode == FillRadial || mode == FillSpherical || mode == FillHighlight;
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

inline QString paintModeName(Buffer::PaintMode mode)
{
    switch (mode) {
    case Buffer::Color:       return "Color";
    case Buffer::Smear:        return "Smear";
    case Buffer::Smooth:       return "Smooth";
    case Buffer::Range:        return "Range";
    case Buffer::AverageSmear: return "Avg Smear";
    case Buffer::Cycle:        return "Cycle";
    case Buffer::Random:       return "Random";
    case Buffer::Tint:         return "Tint";
    case Buffer::Colorize:     return "Colorize";
    case Buffer::Brighten:     return "Brighten";
    case Buffer::Darken:       return "Darken";
    case Buffer::Mix:          return "Mix";
    case Buffer::Negative:     return "Negative";
    case Buffer::Dither1:      return "Dither 1";
    case Buffer::Dither2:      return "Dither 2";
    case Buffer::Transparent:  return "Transparent";
    case Buffer::BrushMode:    return "Brush";
    }
    return {};
}

inline QString effectiveDrawModeName(Buffer::PaintMode paintMode)
{
    if (!drawModeActive)
        return "Color";
    if (paintMode == Buffer::Color)
        return gradientFillModeName(activeGradientFillMode);
    return paintModeName(paintMode);
}

// Returns true when draw mode is on, a gradient fill mode is selected, and the
// active range has markers.
inline bool gradientFillActive()
{
    return drawModeActive
        && activeGradientFillMode != FillFlat
        && !gradientRanges[activeGradientRange].markers().isEmpty();
}

#endif // GRADIENTRANGE_H
