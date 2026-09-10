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
    virtual QRect paintAsColor(const QPoint &point, Buffer *buffer) const = 0;
    virtual QRect rect(const QPoint &point) const = 0;

    // Resolve stamp-level modes before per-pixel dispatch.
    static unsigned resolveDrawColor(Buffer *buffer, Buffer::PaintMode &mode,
                                     bool &isErase, unsigned paintColor);

    static void applyPixelMode(const QPoint &p, Buffer *buffer,
                               Buffer::PaintMode mode, bool isErase,
                               unsigned paintColor, unsigned eraseColor);
};

#endif // PEN_H
