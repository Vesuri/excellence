#ifndef BUFFER_H
#define BUFFER_H

#include <QObject>
#include <QColor>
#include <QRect>
#include <QImage>
#include <QList>

class Tool;
class UndoBuffer;
class Tool;
class Pen;

class Buffer : public QObject
{
    Q_OBJECT
public:
    explicit Buffer(int width = 320, int height = 256, int colors = 8, QObject *parent = nullptr);
    explicit Buffer(const QString &path, QObject *parent = nullptr);

    void setPath(const QString &path);
    QString path() const;
    QImage &image();
    void press(const QPoint &point, const Qt::MouseButton &button);
    void move(const QPoint &point);
    void release(const QPoint &point);
    void setTool(Tool *tool_);
    Tool *tool() const;
    void setPen(Pen *pen);
    Pen *pen() const;

public slots:
    void clear();
    void undo();

signals:
    void pathChanged(QString path);
    void modified(QRect area);
    void toolChanged(Tool *tool_);

private:
    void initialize(int width = 320, int height = 256, int colors = 8);

    QString path_;
    QImage image_;
    QRect modifiedArea;
    QImage preModificationImage;
    QList<UndoBuffer *> undoBuffers;
    Tool *tool_;
    Pen *pen_;
};

#endif // BUFFER_H
