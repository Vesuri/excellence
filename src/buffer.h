#ifndef BUFFER_H
#define BUFFER_H

#include <QObject>
#include <QColor>
#include <QRect>

template <class T> class QVector;

class Buffer : public QObject
{
    Q_OBJECT
public:
    explicit Buffer(int width, int height, int colors, QObject *parent = 0);

    QImage *image() const;
    QVector<QRgb> *palette() const;

private slots:
    void clear();

signals:
    void modified(QRect area);

private:
    QImage *image_;
    QVector<QRgb> *palette_;
};

#endif // BUFFER_H
