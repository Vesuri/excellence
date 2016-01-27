#ifndef PEN_H
#define PEN_H

#include <QObject>

class Pen : public QObject
{
    Q_OBJECT
public:
    explicit Pen(QObject *parent = 0);

    virtual QRect draw(const QPoint &point, QImage &image) = 0;
};

#endif // PEN_H
