#ifndef BUFFER_H
#define BUFFER_H

#include <QObject>
#include <QColor>
#include <QRect>
#include <QImage>
#include <QList>

class Tool;
class UndoBuffer;
class Pen;
class PenTip;

class Buffer : public QObject
{
    Q_OBJECT
public:
    enum PaintMode { Normal, Replace, Smear, Smooth, Range, AverageSmear, Cycle, Random,
                     Tint, Colorize, Brighten, Darken, Mix, Negative,
                     Dither1, Dither2, Transparent, BrushMode };

    explicit Buffer(int width = 640, int height = 512, int colors = 32, QObject *parent = nullptr);
    explicit Buffer(const QString &path, QObject *parent = nullptr);

    void setPath(const QString &path);
    QString path() const;
    void setImage(const QImage &image);
    QImage &image();
    void press(const QPoint &point, const Qt::MouseButton &button, const Qt::KeyboardModifiers &modifiers);
    void move(const QPoint &point);
    void release(const QPoint &point);
    void doubleClick(const QPoint &point);
    void setTool(Tool *tool_);
    Tool *tool() const;
    void setPen(Pen *pen);
    Pen *pen() const;
    PenTip *penTip() const;
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
    void setPaintMode(PaintMode mode);
    PaintMode paintMode() const;
    const QImage &brushStamp() const;
    int brushTransparentIndex() const;
    void setSmearDirection(const QPoint &dir);
    QPoint smearDirection() const;
    void setDrawModeAmount(int amount);
    int drawModeAmount() const;
    bool isDirty() const;
    void clearDirty();
    void setGridEnabled(bool enabled);
    bool gridEnabled() const;
    void setPixelGrid(bool enabled);
    bool pixelGrid() const;
    void setGridSpacing(int w, int h);
    int gridW() const;
    int gridH() const;
    void setGridOffset(int x, int y);
    int gridOffsetX() const;
    int gridOffsetY() const;
    QPoint snapToGrid(const QPoint &p) const;
    void setMirrorX(bool enabled);
    bool mirrorX() const;
    void setMirrorY(bool enabled);
    bool mirrorY() const;
    void setMirrorCenter(int x, int y);
    int mirrorCenterX() const;
    int mirrorCenterY() const;
    QVector<int> gradientColors() const;
    int nextCycleColor(bool reverse = false);
    void resetCycle();

public slots:
    void clear();
    void clearWithColor(unsigned colorIndex);
    void resetToDefaultPalette();
    void undo();
    void redo();
    void undoAll();
    void redoAll();
    void clearUndoBuffer();
    void notifyModified(const QRect &rect);
    void clearHoverPreview();

signals:
    void pathChanged(QString path);
    void modified(QRect area);
    void zoomed(QRect area);
    void toolChanged(Tool *tool);
    void paletteModified();
    void paintColorChanged(unsigned colorIndex, QColor color);
    void eraseColorChanged(unsigned colorIndex, QColor color);
    void gridChanged();
    void pixelGridChanged(bool enabled);
    void mirrorChanged();
    void dirtyChanged(bool dirty);
    void paintModeChanged(PaintMode mode);
    void penChanged(Pen *pen);

private:
    void initialize(int width = 640, int height = 512, int colors = 32);

    QString path_;
    QImage image_;
    QRect modifiedArea;
    QImage preModificationImage;
    QList<UndoBuffer *> undoBuffers;
    QList<UndoBuffer *> redoStack;
    UndoBuffer *moveUndoBuffer;
    Tool *tool_ = nullptr;
    Pen *pen_;
    Pen *toolPen_;
    PenTip *penTip_;
    unsigned paintColor_;
    unsigned eraseColor_;
    PaintMode paintMode_;
    QPoint smearDirection_;
    int cycleIndex_;
    int drawModeAmount_;
    bool gridEnabled_;
    bool pixelGrid_;
    int gridW_, gridH_;
    int gridOffsetX_, gridOffsetY_;
    bool mirrorX_, mirrorY_;
    int mirrorCenterX_, mirrorCenterY_;
    bool dirty_;
    QImage brushStamp_;
    int brushTransparentIndex_;
};

#endif // BUFFER_H
