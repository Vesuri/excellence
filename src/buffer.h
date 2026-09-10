#ifndef BUFFER_H
#define BUFFER_H

#include <QObject>
#include <QColor>
#include <QRect>
#include <QImage>
#include <QList>
#include <QString>

class Tool;
class UndoBuffer;
class Pen;
class PenTip;
class Brush;

class Buffer : public QObject
{
    Q_OBJECT
public:
    enum PaintMode { Color, Smear, Smooth, Range, AverageSmear, Cycle, Random,
                     Tint, Colorize, Brighten, Darken, Mix, Negative,
                     Dither1, Dither2, Transparent, BrushMode, Stencil };
    enum StencilApplyMode { StencilReplace, StencilAdd, StencilSubtract };

    explicit Buffer(int width = 640, int height = 512, int colors = 32, QObject *parent = nullptr);
    explicit Buffer(const QString &path, QObject *parent = nullptr);
    Buffer(const QImage &image, const QString &path, QObject *parent = nullptr);

    void setPath(const QString &path);
    QString path() const;
    void setImage(const QImage &image);
    QImage &image();
    void press(const QPoint &point, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void move(const QPoint &point);
    void release(const QPoint &point);
    void doubleClick(const QPoint &point);
    void setTool(Tool *tool_);
    Tool *tool() const;
    void setPen(Pen *pen);
    Pen *pen() const;
    PenTip *penTip() const;
    Brush *brush() const;
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
    void setReplaceMode(bool on) { replaceMode_ = on; }
    bool replaceMode() const { return replaceMode_; }
    const QImage &brushStamp() const;
    int brushTransparentIndex() const;
    void setSmearDirection(const QPoint &dir);
    QPoint smearDirection() const;
    void setDrawModeAmount(int amount);
    int drawModeAmount() const;
    bool transparentMixHSV() const;
    void setTransparentMixHSV(bool hsv);
    const QImage &referenceImage() const;
    bool isDirty() const;
    void clearDirty();
    void setGridEnabled(bool enabled);
    bool gridEnabled() const;
    void setPixelGrid(bool enabled);
    bool pixelGrid() const;
    void setAutoFitZoom(bool enabled);
    bool autoFitZoom() const;
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

    void copyFrom(const Buffer *source);
    void mergeFrom(const Buffer *source, bool front);

    // Segment draw
    bool segmentActive() const { return segmentActive_; }
    void setSegmentActive(bool v);
    bool segmentByDistance() const { return segmentByDistance_; }
    void setSegmentByDistance(bool v);
    int segmentValue() const { return segmentValue_; }
    void setSegmentValue(int v);
    // Called by Pen implementations on each paint/erase point; returns true if stamp should fire.
    bool segmentCheck(const QPoint &point);

    // Fix Background
    bool fixBackgroundLocked() const { return !fixedBackground_.isNull(); }
    void setFixBackgroundLocked(bool locked);

    // Stencil
    bool hasStencil() const { return !stencilMask_.isNull(); }
    bool stencilEnabled() const { return stencilEnabled_; }
    void setStencilEnabled(bool enabled);
    bool isStencilProtected(const QPoint &p) const;
    const QImage &stencilMask() const { return stencilMask_; }
    void setStencilPixel(const QPoint &p, bool protect);
    void stencilFromForeground();
    void applyStencilColors(StencilApplyMode mode);
    void invertStencilMask();
    void deleteStencilMask();

    // Stencil selected-colors working set (Stencil Tool options, Colors section).
    // No undo history is kept for this set or for the mask itself, by design.
    bool stencilColorSelected(int colorIndex) const;
    void setStencilColorSelected(int colorIndex, bool selected);
    void clearStencilSelectedColors();
    void invertStencilSelectedColors();
    void setStencilSelectedColorsRestorePoint();
    void restoreStencilSelectedColors();

public slots:
    void clear();
    void clearWithColor(unsigned colorIndex);
    void resetToDefaultPalette();
    void undo();
    void redo();
    void mergeLastUndo();
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
    void autoFitZoomChanged(bool enabled);
    void mirrorChanged();
    void dirtyChanged(bool dirty);
    void paintModeChanged(PaintMode mode);
    void penChanged(Pen *pen);
    void penModified();
    void segmentChanged();
    void fixBackgroundChanged();
    void stencilChanged();
    void stencilColorsChanged();

private:
    void initialize(int width = 640, int height = 512, int colors = 32);
    QRect finalizeSegmentStroke();

    QString path_;
    QImage image_;
    QRect modifiedArea_;
    QImage preModificationImage_;
    QList<UndoBuffer *> undoBuffers_;
    QList<UndoBuffer *> redoStack_;
    UndoBuffer *moveUndoBuffer_;
    Tool *tool_ = nullptr;
    Pen *pen_;
    Pen *toolPen_;
    PenTip *penTip_;
    Brush *brush_;
    unsigned paintColor_;
    unsigned eraseColor_;
    PaintMode paintMode_;
    bool replaceMode_;
    QPoint smearDirection_;
    int cycleIndex_;
    int lastCycleColor_;
    bool cycleUsed_;
    int drawModeAmount_;
    bool transparentMixHSV_;
    bool gridEnabled_;
    bool pixelGrid_;
    bool autoFitZoom_ = true;
    int gridW_, gridH_;
    int gridOffsetX_, gridOffsetY_;
    bool mirrorX_, mirrorY_;
    int mirrorCenterX_, mirrorCenterY_;
    bool dirty_;
    QImage brushStamp_;
    int brushTransparentIndex_;
    bool segmentActive_ = false;
    bool segmentByDistance_ = true;
    int segmentValue_ = 10;
    float segmentAccum_ = 0.0f;
    QPoint segmentLastVisited_;
    QList<QPoint> segmentPath_;
    QImage fixedBackground_;
    QImage stencilMask_;
    bool stencilEnabled_ = false;
    QVector<bool> stencilSelectedColors_;
    QVector<bool> stencilSelectedColorsRestore_;
};

inline QString paintModeName(Buffer::PaintMode mode)
{
    switch (mode) {
    case Buffer::Color:       return "Color";
    case Buffer::Smear:       return "Smear";
    case Buffer::Smooth:      return "Smooth";
    case Buffer::Range:       return "Range";
    case Buffer::AverageSmear: return "Avg Smear";
    case Buffer::Cycle:       return "Cycle";
    case Buffer::Random:      return "Random";
    case Buffer::Tint:        return "Tint";
    case Buffer::Colorize:    return "Colorize";
    case Buffer::Brighten:    return "Brighten";
    case Buffer::Darken:      return "Darken";
    case Buffer::Mix:         return "Mix";
    case Buffer::Negative:    return "Negative";
    case Buffer::Dither1:     return "Dither 1";
    case Buffer::Dither2:     return "Dither 2";
    case Buffer::Transparent: return "Transparent";
    case Buffer::BrushMode:   return "Brush";
    case Buffer::Stencil:     return "Stencil";
    }
    return {};
}

#endif // BUFFER_H
