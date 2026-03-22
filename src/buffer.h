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
    void setImage(const QImage &image);
    QImage &image();
    void press(const QPoint &point, const Qt::MouseButton &button, const Qt::KeyboardModifiers &modifiers);
    void move(const QPoint &point);
    void release(const QPoint &point);
    void setTool(Tool *tool_);
    Tool *tool() const;
    void setPen(Pen *pen);
    Pen *pen() const;
    void setToolPen(Pen *pen);
    Pen *toolPen() const;
    void setPaintColor(unsigned colorIndex);
    unsigned paintColor() const;
    void setEraseColor(unsigned colorIndex);
    unsigned eraseColor() const;
    void copyImageColor(unsigned fromIndex, unsigned toIndex);
    void swapImageColors(unsigned index1, unsigned index2);
    void copyPaletteColor(unsigned fromIndex, unsigned toIndex);
    void swapPaletteColors(unsigned index1, unsigned index2);
    void setColor(unsigned colorIndex, const QColor &color);

public slots:
    void clear();
    void clearWithColor(unsigned colorIndex);
    void undo();
    void notifyModified(const QRect &rect);

signals:
    void pathChanged(QString path);
    void modified(QRect area);
    void zoomed(QRect area);
    void toolChanged(Tool *tool);
    void paletteModified();
    void paintColorChanged(unsigned colorIndex, QColor color);
    void eraseColorChanged(unsigned colorIndex, QColor color);

private:
    void initialize(int width = 320, int height = 256, int colors = 8);

    QString path_;
    QImage image_;
    QRect modifiedArea;
    QImage preModificationImage;
    QList<UndoBuffer *> undoBuffers;
    UndoBuffer *moveUndoBuffer;
    Tool *tool_;
    Pen *pen_;
    Pen *toolPen_;
    unsigned paintColor_;
    unsigned eraseColor_;
};

#endif // BUFFER_H
