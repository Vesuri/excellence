#ifndef UNDOBUFFER_H
#define UNDOBUFFER_H

#include <QObject>
#include <QPoint>
#include <QImage>

class Buffer;

class UndoBuffer : public QObject
{
    Q_OBJECT
public:
    explicit UndoBuffer(const QPoint &pos, const QImage &image, QObject *parent = nullptr);

    QPoint pos() const;
    QImage image() const;
    QRect rect() const;
    void apply(Buffer *buffer) const;

private:
    QPoint pos_;
    QImage image_;
};

#endif // UNDOBUFFER_H
