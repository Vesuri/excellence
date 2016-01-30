#ifndef PEN_H
#define PEN_H

#include <QObject>

class Pen : public QObject
{
    Q_OBJECT
public:
    explicit Pen(QObject *parent = 0);

    virtual QRect paint(const QPoint &point, QImage &image) const = 0;
    virtual QRect erase(const QPoint &point, QImage &image) const = 0;
    virtual QRect rect(const QPoint &point) const = 0;
};

#endif // PEN_H
