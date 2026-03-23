#ifndef GRADIENTRANGE_H
#define GRADIENTRANGE_H

#include <QVector>

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

    void flip();
    void clear();
    void undo();
    void restore();
    void setRestorePoint();

private:
    void takeUndoSnapshot();

    QVector<GradientMarker> markers_;
    QVector<GradientMarker> undoSnapshot_;
    QVector<GradientMarker> restorePoint_;
    int spread_ = 0;
};

static const int kGradientRangeCount = 8;
extern GradientRange gradientRanges[kGradientRangeCount];
extern int activeGradientRange;

#endif // GRADIENTRANGE_H
