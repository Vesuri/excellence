#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <QPoint>
#include "tool.h"

class Pen;

class DrawTool : public Tool
{
    Q_OBJECT

public:
    enum DrawMode { Draw, ConnectedDraw, FilledShape };

    explicit DrawTool(QObject *parent = 0);

    void setDrawMode(const DrawMode &drawMode);
    void setPen(Pen *pen);
    virtual QRect press(const QPoint &point, QImage &image);
    virtual QRect move(const QPoint &point, QImage &image);
    virtual QRect release(const QPoint &point, QImage &image);

private:
    QRect draw(const QPoint &point, QImage &image);

    DrawMode drawMode;
    Pen *pen;
    QPoint previousPoint;
};

#endif // DRAWTOOL_H
