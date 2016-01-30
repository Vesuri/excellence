#ifndef UNDOBUFFER_H
#define UNDOBUFFER_H

#include <QObject>
#include <QPoint>
#include <QImage>

class QImage;

class UndoBuffer : public QObject
{
    Q_OBJECT
public:
    explicit UndoBuffer(const QPoint &pos, const QImage &image, QObject *parent = 0);

    QPoint pos() const;
    QImage image() const;
    void apply(QImage &image) const;

private:
    QPoint pos_;
    QImage image_;
};

#endif // UNDOBUFFER_H
