#ifndef GRADIENTRUBBERBAND_H
#define GRADIENTRUBBERBAND_H

#include <QString>
#include <QImage>
#include <QPoint>
#include <QRect>
#include <QSize>
#include "algorithms.h"

struct GradientRubberBand {
    bool   pending = false;
    QPoint from;

    void start(const QPoint &anchor) { pending = true; from = anchor; }
    void clear()                     { pending = false; }

    // Uses qMin/qMax instead of QRect(p1,p2).normalized() to avoid the Qt off-by-one
    // where normalized() skips swapping when x2==x1-1 or y2==y1-1, producing a
    // zero-width/height rect that saves nothing and leaves pixels unrestored.
    QRect hoverRect(const QPoint &cursor, const QRect &imageRect) const {
        if (!pending) return {};
        return QRect(qMin(from.x(), cursor.x()),
                     qMin(from.y(), cursor.y()),
                     qAbs(cursor.x() - from.x()) + 1,
                     qAbs(cursor.y() - from.y()) + 1).intersected(imageRect);
    }

    QRect draw(const QPoint &cursor, QImage &image) const {
        if (!pending) return {};
        const int mask = image.colorCount() - 1;
        QRect changed;
        Algorithms::line(from, cursor, [&](const QPoint &p) {
            if (image.rect().contains(p)) {
                image.setPixel(p, static_cast<uint>(image.pixelIndex(p) ^ mask));
                changed = changed.united(QRect(p, QSize(1, 1)));
            }
        });
        return changed;
    }

    QString status() const { return pending ? "click to set gradient angle" : QString(); }
};

#endif // GRADIENTRUBBERBAND_H
