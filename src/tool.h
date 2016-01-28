#ifndef TOOL_H
#define TOOL_H

#include <QObject>

class Tool : public QObject
{
    Q_OBJECT

public:
    explicit Tool(QObject *parent = 0);

    virtual QRect press(const QPoint &point, QImage &image) = 0;
    virtual QRect move(const QPoint &point, QImage &image) = 0;
    virtual QRect release(const QPoint &point, QImage &image) = 0;
};

#endif // TOOL_H
