#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <QPoint>
#include "tool.h"

class DrawTool : public Tool
{
    Q_OBJECT

public:
    enum DrawMode { Draw, ConnectedDraw, FilledShape };

    explicit DrawTool(QObject *parent = nullptr);

    void setDrawMode(const DrawMode &drawMode);
    void setBuffer(Buffer *buffer) override;
    QRect press(const QPoint &point) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;

private:
    QRect draw(const QPoint &point);

    DrawMode drawMode;
    QPoint startingPoint;
    QPoint previousPoint;

    static DrawTool instance;
    static const char* icons[];
};

#endif // DRAWTOOL_H
