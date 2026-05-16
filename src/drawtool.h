#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <QList>
#include <QPoint>
#include "tool.h"
#include "gradientrubberband.h"

class DrawTool : public Tool
{
    Q_OBJECT

public:
    enum DrawMode { Dotted, ConnectedDraw, FilledShape };

    explicit DrawTool(QObject *parent = nullptr);

    void setDrawMode(const DrawMode &drawMode);
    void setBuffer(Buffer *buffer) override;
    bool hasFill() const override { return drawMode == FilledShape; }
    QRect press(const QPoint &point, const Qt::KeyboardModifiers &modifiers) override;
    QRect move(const QPoint &point) override;
    QRect release(const QPoint &point) override;
    QRect hover(const QPoint &point) override;
    void cancel() override;
    QString status() const override;
    void addButtonToGridLayout(QGridLayout *layout) override;

protected:
    void registerTool() override;
    void activate() override;

private:
    QRect draw(const QPoint &point);
    QRect polygonFill(int fillColor, const QPoint &to);
    QRect applyPolygonGradient(const QList<QPoint> &path, const QPoint &gradFrom, const QPoint &gradTo);

    DrawMode drawMode;
    QPoint startingPoint;
    QPoint previousPoint;
    QPoint lastStampedPoint;
    QRect drawnBounds_;
    QList<QPoint> pathPoints_;
    GradientRubberBand rubberBand_;
    QList<QPoint> pendingPathPoints_;

    static DrawTool instance;
    static const char* icons[];
};

#endif // DRAWTOOL_H
