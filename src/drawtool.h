#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <QPoint>
#include "tool.h"

class DrawTool : public Tool
{
    Q_OBJECT

public:
    enum DrawMode { Draw, ConnectedDraw, FilledShape };

    explicit DrawTool(QObject *parent = 0);

    void setDrawMode(const DrawMode &drawMode);
    virtual void setBuffer(Buffer *buffer);
    virtual QRect press(const QPoint &point);
    virtual QRect move(const QPoint &point);
    virtual QRect release(const QPoint &point);
    virtual void addButtonToGridLayout(QGridLayout *layout);

protected:
    virtual void registerTool();
    virtual void activate();

private:
    QRect draw(const QPoint &point);

    DrawMode drawMode;
    QPoint previousPoint;

    static DrawTool instance;
    static const char* icons[];
};

#endif // DRAWTOOL_H
