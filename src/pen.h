#ifndef PEN_H
#define PEN_H

#include <QObject>
#include "buffer.h"

class Pen : public QObject
{
    Q_OBJECT
public:
    explicit Pen(QObject *parent = nullptr);

    virtual QRect paint(const QPoint &point, Buffer *buffer) const = 0;
    virtual QRect erase(const QPoint &point, Buffer *buffer) const = 0;
    virtual QRect rect(const QPoint &point) const = 0;

    // Shared per-pixel paint mode dispatch used by PenTip and Brush.
    // Handles all modes except BrushMode (caller must handle, e.g. PenTip tiles
    // the brush stamp) and Cycle/Random (caller must resolve the color first and
    // pass Buffer::Normal as the mode with the resolved color as paintColor).
    // Resolves Cycle/Random to a concrete color for the current stamp.
    // If mode is Cycle or Random: sets mode to Normal, sets isErase to false
    // (the returned color already incorporates erase semantics), and returns
    // the resolved color. For all other modes: returns paintColor unchanged.
    // Callers can pass the return value directly as paintColor to applyPixelMode.
    static unsigned resolveDrawColor(Buffer *buffer, Buffer::PaintMode &mode,
                                     bool &isErase, unsigned paintColor);

    static void applyPixelMode(const QPoint &p, Buffer *buffer,
                               Buffer::PaintMode mode, bool isErase,
                               unsigned paintColor, unsigned eraseColor);
};

#endif // PEN_H
