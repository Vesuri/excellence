#include "gradientrange.h"
#include <algorithm>

GradientRange gradientRanges[kGradientRangeCount];
int activeGradientRange = 0;

int GradientRange::colorCount() const
{
    int n = markers_.size();
    if (n == 0) return 0;
    return n + (n - 1) * spread_;
}

void GradientRange::addMarker(int slot, int colorIndex, bool abrupt)
{
    removeMarker(slot);
    markers_.append({slot, colorIndex, abrupt});
    std::sort(markers_.begin(), markers_.end(), [](const GradientMarker &a, const GradientMarker &b) {
        return a.slot < b.slot;
    });
}

void GradientRange::removeMarker(int slot)
{
    for (int i = 0; i < markers_.size(); i++) {
        if (markers_[i].slot == slot) {
            markers_.removeAt(i);
            return;
        }
    }
}

void GradientRange::flip()
{
    if (markers_.isEmpty())
        return;
    takeUndoSnapshot();
    // Reverse color order while keeping slot positions
    int n = markers_.size();
    for (int i = 0; i < n / 2; i++) {
        std::swap(markers_[i].colorIndex, markers_[n - 1 - i].colorIndex);
        std::swap(markers_[i].abrupt, markers_[n - 1 - i].abrupt);
    }
}

void GradientRange::clear()
{
    if (markers_.isEmpty())
        return;
    takeUndoSnapshot();
    markers_.clear();
}

void GradientRange::undo()
{
    Snapshot current = currentSnapshot();
    applySnapshot(undoSnapshot_);
    undoSnapshot_ = current;
}

void GradientRange::setRestorePoint()
{
    restorePoint_ = currentSnapshot();
}

void GradientRange::restore()
{
    takeUndoSnapshot();
    applySnapshot(restorePoint_);
}

void GradientRange::takeUndoSnapshot()
{
    undoSnapshot_ = currentSnapshot();
}

GradientRange::Snapshot GradientRange::currentSnapshot() const
{
    return { markers_, spread_, random_, hardEdges_, ditherAmount_ };
}

void GradientRange::applySnapshot(const Snapshot &s)
{
    markers_      = s.markers;
    spread_       = s.spread;
    random_       = s.random;
    hardEdges_    = s.hardEdges;
    ditherAmount_ = s.ditherAmount;
}
