#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <QObject>
#include <QPoint>

class DrawTool : public QObject
{
    Q_OBJECT

public:
    enum Mode { Draw, ConnectedDraw, FilledShape };

    explicit DrawTool(QObject *parent = 0);

    void setMode(const Mode &mode);
    QRect press(const QPoint &point, QImage &image);
    QRect move(const QPoint &point, QImage &image);
    QRect release(const QPoint &point, QImage &image);

private:
    Mode mode;
    QPoint previousPoint;
};

#endif // DRAWTOOL_H
