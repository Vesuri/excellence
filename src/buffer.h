#ifndef BUFFER_H
#define BUFFER_H

#include <QObject>
#include <QColor>
#include <QRect>

class Tool;
template <class T> class QVector;

class Buffer : public QObject
{
    Q_OBJECT
public:
    explicit Buffer(int width, int height, int colors, QObject *parent = 0);

    QImage *image() const;
    QVector<QRgb> *palette() const;
    void press(const QPoint &point, Tool *tool);
    void move(const QPoint &point, Tool *tool);
    void release(const QPoint &point, Tool *tool);

private slots:
    void clear();

signals:
    void modified(QRect area);

private:
    QImage *image_;
    QVector<QRgb> *palette_;
    QRect modifiedArea;
};

#endif // BUFFER_H
