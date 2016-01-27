#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include "tool.h"
#include <QPoint>

class DrawTool : public Tool
{
    Q_OBJECT

public:
    enum Mode { Draw, ConnectedDraw, FilledShape };

    explicit DrawTool(QObject *parent = 0);

    void setMode(const Mode &mode);
    void setColor(int color);
    virtual QRect press(const QPoint &point, QImage &image);
    virtual QRect move(const QPoint &point, QImage &image);
    virtual QRect release(const QPoint &point, QImage &image);

private:
    Mode mode;
    int color;
    QPoint previousPoint;
};

#endif // DRAWTOOL_H
