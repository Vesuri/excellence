#ifndef LINETOOL_H
#define LINETOOL_H

#include <QPoint>
#include "tool.h"

class Pen;
class UndoBuffer;

class LineTool : public Tool
{
    Q_OBJECT

public:
    explicit LineTool(QObject *parent = 0);

    void setPen(Pen *pen);
    virtual QRect press(const QPoint &point, QImage &image);
    virtual QRect move(const QPoint &point, QImage &image);
    virtual QRect release(const QPoint &point, QImage &image);

private:
    QRect changes(const QPoint &point);
    QRect draw(const QPoint &point, QImage &image);

    Pen *pen;
    QPoint startPoint;
    UndoBuffer *undoBuffer;
};

#endif // LINETOOL_H
