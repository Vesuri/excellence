#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <QObject>

class DrawTool : public QObject
{
    Q_OBJECT
public:
    explicit DrawTool(QObject *parent = 0);

    QRect press(const QPoint &point, QImage &image);
    QRect move(const QPoint &point, QImage &image);
    QRect release(const QPoint &point, QImage &image);
};

#endif // DRAWTOOL_H
