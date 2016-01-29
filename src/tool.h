#ifndef TOOL_H
#define TOOL_H

#include <QObject>

class Tool : public QObject
{
    Q_OBJECT

public:
    enum Mode { Paint, Erase };

    explicit Tool(QObject *parent = 0);

    void setMode(const Mode &mode);

    virtual QRect press(const QPoint &point, QImage &image) = 0;
    virtual QRect move(const QPoint &point, QImage &image) = 0;
    virtual QRect release(const QPoint &point, QImage &image) = 0;

protected:
    Mode mode_;
};

#endif // TOOL_H
