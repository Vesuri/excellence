#ifndef BUFFER_H
#define BUFFER_H

#include <QObject>
#include <QColor>
#include <QRect>
#include <QImage>
#include <QList>

class Tool;
template <class T> class QVector;
class UndoBuffer;
class Tool;
class Pen;

class Buffer : public QObject
{
    Q_OBJECT
public:
    explicit Buffer(int width, int height, int colors, QObject *parent = 0);

    QImage &image();
    QRgb color(unsigned index) const;
    unsigned colorCount() const;
    void press(const QPoint &point, const Qt::MouseButton &button);
    void move(const QPoint &point);
    void release(const QPoint &point);
    void setTool(Tool *tool);
    void setPen(Pen *pen);
    Pen *pen() const;

public slots:
    void clear();
    void undo();

signals:
    void modified(QRect area);
    void toolChanged(Tool *tool);

private:
    QImage image_;
    QVector<QRgb> palette_;
    QRect modifiedArea;
    QImage preModificationImage;
    QList<UndoBuffer *> undoBuffers;
    Tool *tool;
    Pen *pen_;
};

#endif // BUFFER_H
