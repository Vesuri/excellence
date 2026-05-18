#include <climits>
#include <QImage>
#include <QMessageBox>
#include "brush.h"
#include "gradientrange.h"
#include "pentip.h"
#include "undobuffer.h"
#include "tool.h"
#include "buffer.h"
#include "defaultpalette.h"

Buffer::Buffer(int width, int height, int colors, QObject *parent) : QObject(parent),
    moveUndoBuffer(nullptr),
    pen_(nullptr),
    toolPen_(nullptr),
    penTip_(nullptr),
    brush_(nullptr),
    paintColor_(1),
    eraseColor_(0),
    paintMode_(Color),
    replaceMode_(false),
    smearDirection_(0, 0),
    cycleIndex_(0),
    lastCycleColor_(0),
    cycleUsed_(false),
    drawModeAmount_(50),
    transparentMixHSV_(false),
    gridEnabled_(false),
    pixelGrid_(false),
    gridW_(8), gridH_(8),
    gridOffsetX_(0), gridOffsetY_(0),
    mirrorX_(false), mirrorY_(false),
    mirrorCenterX_(0), mirrorCenterY_(0),
    dirty_(false),
    brushTransparentIndex_(-1)
{
    initialize(width, height, colors);
    mirrorCenterX_ = image_.width() / 2;
    mirrorCenterY_ = image_.height() / 2;
}

Buffer::Buffer(const QString &path, QObject *parent) : QObject(parent),
    path_(path),
    image_(),
    moveUndoBuffer(nullptr),
    pen_(nullptr),
    toolPen_(nullptr),
    penTip_(nullptr),
    brush_(nullptr),
    paintColor_(1),
    eraseColor_(0),
    paintMode_(Color),
    replaceMode_(false),
    smearDirection_(0, 0),
    cycleIndex_(0),
    lastCycleColor_(0),
    cycleUsed_(false),
    drawModeAmount_(50),
    transparentMixHSV_(false),
    gridEnabled_(false),
    pixelGrid_(false),
    gridW_(8), gridH_(8),
    gridOffsetX_(0), gridOffsetY_(0),
    mirrorX_(false), mirrorY_(false),
    mirrorCenterX_(0), mirrorCenterY_(0),
    brushTransparentIndex_(-1)
{
    if (!path.isEmpty()) {
        image_.load(path);
    }
    if (image_.isNull() || image_.format() != QImage::Format_Indexed8) {
        if (!path.isEmpty() && image_.format() != QImage::Format_Indexed8) {
            QMessageBox msgBox;
            msgBox.setText("Not an indexed palette image.");
            msgBox.exec();
        }
        initialize();
    }
    mirrorCenterX_ = image_.width() / 2;
    mirrorCenterY_ = image_.height() / 2;
}

void Buffer::initialize(int width, int height, int colors)
{
    image_ = QImage(width, height, QImage::Format_Indexed8);
    QVector<QRgb> colorTable;
    const QRgb *palette = defaultPaletteForColors(colors);
    for (int i = 0; i < colors; i++) {
        colorTable.append(palette[i]);
    }
    image_.setColorTable(colorTable);
    image_.fill(0);
}

void Buffer::setPath(const QString &path)
{
    path_ = path;

    emit pathChanged(path);
}

QString Buffer::path() const
{
    return path_;
}

void Buffer::setImage(const QImage &image)
{
    image_ = image;

    emit modified(image_.rect());
}

QImage &Buffer::image()
{
    return image_;
}

void Buffer::clear()
{
    clearWithColor(0);
}

void Buffer::clearWithColor(unsigned colorIndex)
{
    undoBuffers.append(new UndoBuffer(QPoint(), image_.copy()));
    qDeleteAll(redoStack);
    redoStack.clear();

    image_.fill(colorIndex);

    emit modified(image_.rect());
}

void Buffer::press(const QPoint &point, const Qt::MouseButton &button, const Qt::KeyboardModifiers &modifiers)
{
    tool_->setMouseButton(button);
    const QPoint p = snapToGrid(point);

    switch (tool_->type()) {
    case Tool::Zoom: {
        QRect zoomedArea = tool_->press(p, modifiers);

        emit zoomed(zoomedArea);
        break;
    }
    default:
        if (moveUndoBuffer) {
            moveUndoBuffer->apply(this);
            delete moveUndoBuffer;
            moveUndoBuffer = nullptr;
        }

        preModificationImage = image_.copy();

        // Reset segment state for new stroke; pre-charge so first stamp fires at press
        segmentAccum_ = static_cast<float>(segmentValue_);
        segmentLastVisited_ = p;
        segmentPath_.clear();

        modifiedArea = tool_->press(p, modifiers);

        emit modified(modifiedArea);
        break;
    }
}

void Buffer::move(const QPoint &point)
{
    const QPoint p = snapToGrid(point);

    if (moveUndoBuffer) {
        moveUndoBuffer->apply(this);
        delete moveUndoBuffer;
        moveUndoBuffer = nullptr;
    }

    if (tool_->mouseButton() == Qt::NoButton) {
        QRect rect = tool_->hover(p);
        if (!rect.isNull()) {
            // Expand save rect to cover all mirror positions
            if (mirrorX_ || mirrorY_) {
                int dx = 2 * (mirrorCenterX_ - p.x());
                int dy = 2 * (mirrorCenterY_ - p.y());
                if (mirrorX_)                  rect = rect.united(rect.translated(dx, 0));
                if (mirrorY_)                  rect = rect.united(rect.translated(0, dy));
                if (mirrorX_ && mirrorY_)      rect = rect.united(rect.translated(dx, dy));
            }
            rect = rect.intersected(image_.rect());
            moveUndoBuffer = new UndoBuffer(rect.topLeft(), image().copy(rect));
        }
    }

    // For # of Points segment mode: clear stale path so only this move's draw is recorded,
    // then replay N stamps after the tool draws (gives live preview during drag).
    if (segmentActive_ && !segmentByDistance_ && tool_->mouseButton() != Qt::NoButton)
        segmentPath_.clear();

    QRect area = tool_->move(p);

    if (segmentActive_ && !segmentByDistance_ && tool_->mouseButton() != Qt::NoButton && !segmentPath_.isEmpty())
        area = area.united(finalizeSegmentStroke());

    switch (tool_->type()) {
    case Tool::Zoom:
        emit zoomed(area);
        break;
    default:
        modifiedArea = modifiedArea.united(area);

        emit modified(area);
        break;
    }
}

void Buffer::release(const QPoint &point)
{
    // Clear any path recorded during ghost/preview draws so only the final
    // tool draw (inside tool_->release) is captured for # of Points mode.
    segmentPath_.clear();

    QRect area = tool_->release(snapToGrid(point));

    switch (tool_->type()) {
    case Tool::Zoom:
        emit zoomed(area);
        break;
    default:
        modifiedArea = modifiedArea.united(area);

        // # of Points segment finalization: stamp N evenly-spaced points along recorded path
        if (segmentActive_ && !segmentByDistance_ && !segmentPath_.isEmpty()) {
            QRect segArea = finalizeSegmentStroke();
            modifiedArea = modifiedArea.united(segArea);
            if (!segArea.isNull())
                emit modified(segArea);
        }
        segmentPath_.clear();

        emit modified(area);

        if (!modifiedArea.isNull()) {
            undoBuffers.append(new UndoBuffer(modifiedArea.topLeft(), preModificationImage.copy(modifiedArea)));
            qDeleteAll(redoStack);
            redoStack.clear();
            if (!dirty_) { dirty_ = true; emit dirtyChanged(true); }
            if (paintMode_ == Cycle && cycleUsed_)
                setPaintColor(static_cast<unsigned>(lastCycleColor_));
        }

        modifiedArea = QRect();
        preModificationImage = QImage();
        break;
    }

    tool_->setMouseButton(Qt::NoButton);
}

void Buffer::doubleClick(const QPoint &point)
{
    if (!tool_) return;
    // Save pre-modification state so the subsequent release() can create a valid undo entry.
    if (preModificationImage.isNull())
        preModificationImage = image_.copy();
    QRect area = tool_->doubleClick(snapToGrid(point));
    modifiedArea = modifiedArea.united(area);
    emit modified(area);
}

void Buffer::notifyModified(const QRect &rect)
{
    modifiedArea = modifiedArea.united(rect);
    if (!dirty_) {
        dirty_ = true;
        emit dirtyChanged(true);
    }
    emit modified(rect);
}

bool Buffer::isDirty() const { return dirty_; }

void Buffer::clearDirty()
{
    if (dirty_) {
        dirty_ = false;
        emit dirtyChanged(false);
    }
}

void Buffer::undo()
{
    if (!undoBuffers.isEmpty()) {
        if (moveUndoBuffer) {
            moveUndoBuffer->apply(this);
            delete moveUndoBuffer;
            moveUndoBuffer = nullptr;
        }
        UndoBuffer *undoBuffer = undoBuffers.takeLast();
        redoStack.append(new UndoBuffer(undoBuffer->pos(), image_.copy(undoBuffer->rect())));
        undoBuffer->apply(this);
        emit modified(undoBuffer->rect());
        delete undoBuffer;
    }
}

void Buffer::mergeLastUndo()
{
    if (undoBuffers.isEmpty()) return;
    UndoBuffer *last = undoBuffers.takeLast();
    // Patch preModificationImage with the original pixels stored in this entry
    // (cheaper than re-copying the entire image), then restore the canvas.
    const QImage &orig = last->image();
    const QPoint pos = last->pos();
    for (int y = 0; y < orig.height(); y++)
        for (int x = 0; x < orig.width(); x++)
            preModificationImage.setPixel(pos.x() + x, pos.y() + y,
                                          static_cast<uint>(orig.pixelIndex(x, y)));
    last->apply(this);
    delete last;
}

void Buffer::redo()
{
    if (!redoStack.isEmpty()) {
        if (moveUndoBuffer) {
            moveUndoBuffer->apply(this);
            delete moveUndoBuffer;
            moveUndoBuffer = nullptr;
        }
        UndoBuffer *redoBuffer = redoStack.takeLast();
        undoBuffers.append(new UndoBuffer(redoBuffer->pos(), image_.copy(redoBuffer->rect())));
        redoBuffer->apply(this);
        emit modified(redoBuffer->rect());
        delete redoBuffer;
    }
}

void Buffer::undoAll()
{
    while (!undoBuffers.isEmpty())
        undo();
}

void Buffer::redoAll()
{
    while (!redoStack.isEmpty())
        redo();
}

void Buffer::clearUndoBuffer()
{
    qDeleteAll(undoBuffers);
    undoBuffers.clear();
    qDeleteAll(redoStack);
    redoStack.clear();
}

void Buffer::setPen(Pen *pen)
{
    if (Brush *old = qobject_cast<Brush *>(pen_))
        disconnect(old, &Brush::imageChanged, this, &Buffer::penModified);
    pen_ = pen;
    if (PenTip *t = qobject_cast<PenTip *>(pen))
        penTip_ = t;
    else if (Brush *b = qobject_cast<Brush *>(pen)) {
        brush_ = b;
        brushStamp_ = b->image();
        brushTransparentIndex_ = b->transparentIndex();
        connect(b, &Brush::imageChanged, this, &Buffer::penModified);
    }
    emit penChanged(pen);
}

Pen *Buffer::pen() const { return pen_; }

PenTip *Buffer::penTip() const { return penTip_; }
Brush *Buffer::brush() const { return brush_; }

const QImage &Buffer::brushStamp() const
{
    return brushStamp_;
}

int Buffer::brushTransparentIndex() const
{
    return brushTransparentIndex_;
}

void Buffer::setToolPen(Pen *pen)
{
    toolPen_ = pen;
}

Pen *Buffer::toolPen() const
{
    return toolPen_;
}

void Buffer::setTool(Tool *tool)
{
    if (tool_ != tool) {
        clearHoverPreview();
        tool_ = tool;
        tool_->setMouseButton(Qt::NoButton);
    }
    emit toolChanged(tool);
}

void Buffer::clearHoverPreview()
{
    if (moveUndoBuffer) {
        moveUndoBuffer->apply(this);
        delete moveUndoBuffer;
        moveUndoBuffer = nullptr;
        emit modified(image_.rect());
    }
}

Tool *Buffer::tool() const
{
    return tool_;
}

void Buffer::setPaintColor(unsigned colorIndex)
{
    paintColor_ = colorIndex;

    emit paintColorChanged(paintColor_, image().color(static_cast<int>(paintColor_)));
}

unsigned Buffer::paintColor() const
{
    return paintColor_;
}

void Buffer::setEraseColor(unsigned colorIndex)
{
    eraseColor_ = colorIndex;

    emit eraseColorChanged(eraseColor_, image().color(static_cast<int>(eraseColor_)));
}

unsigned Buffer::eraseColor() const
{
    return eraseColor_;
}

void Buffer::copyImageColor(unsigned fromIndex, unsigned toIndex)
{
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            if (image_.pixelIndex(x, y) == static_cast<int>(toIndex)) {
                image_.setPixel(x, y, fromIndex);
            }
        }
    }
    emit modified(image_.rect());
}

void Buffer::swapImageColors(unsigned index1, unsigned index2)
{
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            if (image_.pixelIndex(x, y) == static_cast<int>(index1)) {
                image_.setPixel(x, y, index2);
            } else if (image_.pixelIndex(x, y) == static_cast<int>(index2)) {
                image_.setPixel(x, y, index1);
            }
        }
    }
    emit modified(image_.rect());
}

void Buffer::copyPaletteColor(unsigned fromIndex, unsigned toIndex)
{
    QRgb color = image_.color(fromIndex);
    image_.setColor(toIndex, color);
    emit paletteModified();
    emit modified(image_.rect());
}

void Buffer::swapPaletteColors(unsigned index1, unsigned index2)
{
    QRgb color1 = image_.color(index1);
    QRgb color2 = image_.color(index2);
    image_.setColor(index1, color2);
    image_.setColor(index2, color1);
    emit paletteModified();
    emit modified(image_.rect());
}

void Buffer::setColor(unsigned colorIndex, const QColor &color)
{
    image_.setColor(colorIndex, color.rgb());
    emit paletteModified();
    emit modified(image_.rect());
}

void Buffer::setPaintMode(PaintMode mode) { paintMode_ = mode; emit paintModeChanged(mode); }
Buffer::PaintMode Buffer::paintMode() const { return paintMode_; }
void Buffer::setSmearDirection(const QPoint &dir) { smearDirection_ = dir; }
QPoint Buffer::smearDirection() const { return smearDirection_; }

QVector<int> Buffer::gradientColors() const
{
    QVector<int> range;
    int from = static_cast<int>(eraseColor_);
    int to   = static_cast<int>(paintColor_);
    int total = image_.colorCount();
    if (from <= to) {
        for (int i = from; i <= to && i < total; i++) range.append(i);
    } else {
        for (int i = from; i >= to && i >= 0; i--) range.append(i);
    }
    return range;
}

int Buffer::nextCycleColor(bool reverse)
{
    const auto &markers = gradientRanges[activeGradientRange].markers();
    if (markers.isEmpty()) return static_cast<int>(paintColor_);
    int sz = markers.size();
    int idx = ((cycleIndex_ % sz) + sz) % sz;
    lastCycleColor_ = markers[idx].colorIndex;
    cycleUsed_ = true;
    cycleIndex_ += reverse ? -1 : 1;
    return lastCycleColor_;
}

void Buffer::resetCycle()
{
    cycleUsed_ = false;
    const auto &markers = gradientRanges[activeGradientRange].markers();
    if (markers.isEmpty()) { cycleIndex_ = 0; return; }

    int paintColorIdx = static_cast<int>(paintColor_);
    for (int i = 0; i < markers.size(); i++) {
        if (markers[i].colorIndex == paintColorIdx) { cycleIndex_ = i; return; }
    }

    const QVector<QRgb> &table = image_.colorTable();
    if (paintColorIdx < 0 || paintColorIdx >= table.size()) { cycleIndex_ = 0; return; }
    QRgb fgRgb = table[paintColorIdx];
    int bestIdx = 0, bestDist = INT_MAX;
    for (int i = 0; i < markers.size(); i++) {
        int ci = markers[i].colorIndex;
        if (ci < 0 || ci >= table.size()) continue;
        int dr = qRed(fgRgb)   - qRed(table[ci]);
        int dg = qGreen(fgRgb) - qGreen(table[ci]);
        int db = qBlue(fgRgb)  - qBlue(table[ci]);
        int dist = dr*dr + dg*dg + db*db;
        if (dist < bestDist) { bestDist = dist; bestIdx = i; }
    }
    cycleIndex_ = bestIdx;
}

void Buffer::setDrawModeAmount(int amount) { drawModeAmount_ = qBound(0, amount, 100); }
int Buffer::drawModeAmount() const { return drawModeAmount_; }
bool Buffer::transparentMixHSV() const { return transparentMixHSV_; }
void Buffer::setTransparentMixHSV(bool hsv) { transparentMixHSV_ = hsv; }
const QImage &Buffer::referenceImage() const
{
    return preModificationImage.isNull() ? image_ : preModificationImage;
}

void Buffer::setSegmentActive(bool v) { segmentActive_ = v; emit segmentChanged(); }
void Buffer::setSegmentByDistance(bool v) { segmentByDistance_ = v; emit segmentChanged(); }
void Buffer::setSegmentValue(int v) { segmentValue_ = qMax(1, v); emit segmentChanged(); }

bool Buffer::segmentCheck(const QPoint &point)
{
    if (!segmentActive_) return true;
    if (!tool_ || tool_->mouseButton() == Qt::NoButton) return true; // hover: always stamp

    if (segmentByDistance_) {
        QPoint delta = point - segmentLastVisited_;
        float dist = sqrt(static_cast<float>(delta.x() * delta.x() + delta.y() * delta.y()));
        segmentAccum_ += dist;
        segmentLastVisited_ = point;
        if (segmentAccum_ < static_cast<float>(segmentValue_)) return false;
        segmentAccum_ -= static_cast<float>(segmentValue_);
        return true;
    } else {
        segmentPath_.append(point);
        return false;
    }
}

QRect Buffer::finalizeSegmentStroke()
{
    if (segmentPath_.isEmpty() || segmentValue_ < 1) return QRect();

    // Compute cumulative distances along recorded path
    QVector<float> cumLen(segmentPath_.size(), 0.0f);
    for (int i = 1; i < segmentPath_.size(); i++) {
        QPoint d = segmentPath_[i] - segmentPath_[i - 1];
        cumLen[i] = cumLen[i - 1] + sqrt(static_cast<float>(d.x() * d.x() + d.y() * d.y()));
    }
    float totalLen = cumLen.last();

    int N = segmentValue_;
    bool closed = segmentPath_.size() > 1 && segmentPath_.first() == segmentPath_.last();
    QRect result;
    // Temporarily disable segment so replay doesn't recurse
    segmentActive_ = false;
    for (int i = 1; i <= N; i++) {
        // Closed path (rectangle, ellipse): N stamps evenly around the loop.
        // Open path (line): first stamp at start, last at end; N=1 → midpoint.
        float target = closed ? totalLen * (i - 1) / N
                               : (N == 1 ? totalLen * 0.5f : totalLen * (i - 1) / (N - 1));
        // Binary search / linear scan for segment containing target
        int j = 1;
        while (j < segmentPath_.size() - 1 && cumLen[j] < target) j++;
        float segLen = cumLen[j] - cumLen[j - 1];
        float t = segLen > 0.0f ? (target - cumLen[j - 1]) / segLen : 0.0f;
        t = qBound(0.0f, t, 1.0f);
        QPoint d = segmentPath_[j] - segmentPath_[j - 1];
        QPoint stampPoint = segmentPath_[j - 1] + QPoint(qRound(d.x() * t), qRound(d.y() * t));
        result = result.united(pen_->paint(stampPoint, this));
    }
    segmentActive_ = true;
    return result;
}

void Buffer::setGridEnabled(bool enabled)
{
    gridEnabled_ = enabled;
    emit gridChanged();
}
bool Buffer::gridEnabled() const { return gridEnabled_; }

void Buffer::setPixelGrid(bool enabled)
{
    pixelGrid_ = enabled;
    emit pixelGridChanged(enabled);
}
bool Buffer::pixelGrid() const { return pixelGrid_; }

void Buffer::setGridSpacing(int w, int h)
{
    gridW_ = qMax(1, w);
    gridH_ = qMax(1, h);
    emit gridChanged();
}
int Buffer::gridW() const { return gridW_; }
int Buffer::gridH() const { return gridH_; }

void Buffer::setGridOffset(int x, int y)
{
    gridOffsetX_ = x;
    gridOffsetY_ = y;
    emit gridChanged();
}
int Buffer::gridOffsetX() const { return gridOffsetX_; }
int Buffer::gridOffsetY() const { return gridOffsetY_; }

QPoint Buffer::snapToGrid(const QPoint &p) const
{
    if (!gridEnabled_ || gridW_ <= 0 || gridH_ <= 0)
        return p;
    int x = qRound(static_cast<double>(p.x() - gridOffsetX_) / gridW_) * gridW_ + gridOffsetX_;
    int y = qRound(static_cast<double>(p.y() - gridOffsetY_) / gridH_) * gridH_ + gridOffsetY_;
    return QPoint(x, y);
}

void Buffer::setMirrorX(bool enabled) { mirrorX_ = enabled; emit mirrorChanged(); }
bool Buffer::mirrorX() const { return mirrorX_; }
void Buffer::setMirrorY(bool enabled) { mirrorY_ = enabled; emit mirrorChanged(); }
bool Buffer::mirrorY() const { return mirrorY_; }
void Buffer::setMirrorCenter(int x, int y) { mirrorCenterX_ = x; mirrorCenterY_ = y; emit mirrorChanged(); }
int Buffer::mirrorCenterX() const { return mirrorCenterX_; }
int Buffer::mirrorCenterY() const { return mirrorCenterY_; }

void Buffer::resetToDefaultPalette()
{
    int count = image_.colorCount();
    const QRgb *palette = defaultPaletteForColors(count);
    for (int i = 0; i < count; i++) {
        image_.setColor(i, palette[i]);
    }
    emit paletteModified();
    emit modified(image_.rect());
}
