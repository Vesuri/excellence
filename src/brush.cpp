#include <QRect>
#include <QImage>
#include <climits>
#include <cmath>
#include "buffer.h"
#include "brush.h"

Brush::Brush(const QImage &image, int transparentIndex, QObject *parent) : Pen(parent),
    image_(image),
    transparentIndex_(transparentIndex),
    handleOffset_(image.width() / 2, image.height() / 2)
{
}

QPoint Brush::handleOffset() const
{
    return handleOffset_;
}

void Brush::setHandleOffset(const QPoint &offset)
{
    handleOffset_ = offset;
}

static void brushStampAt(const QImage &brushImage, int transparentIndex, Buffer *buffer,
                         const QPoint &origin, bool isErase)
{
    Buffer::PaintMode mode = buffer->paintMode();
    QRect imageRect = buffer->image().rect();
    unsigned paintC = buffer->paintColor();
    unsigned eraseC = buffer->eraseColor();
    bool effectiveErase = isErase;
    paintC = Pen::resolveDrawColor(buffer, mode, effectiveErase, paintC);
    // mode is now Normal if it was Cycle/Random; effectiveErase is false in that case.

    for (int y = 0; y < brushImage.height(); y++) {
        for (int x = 0; x < brushImage.width(); x++) {
            int idx = brushImage.pixelIndex(x, y);
            bool isTransparent = (idx == transparentIndex);
            // Replace fills the full bounding rect; all other modes skip transparent pixels
            if (isTransparent && mode != Buffer::Replace)
                continue;
            QPoint p(origin.x() + x, origin.y() + y);
            if (!imageRect.contains(p))
                continue;

            if (isErase) {
                // Erase: BrushMode treated as Normal (use eraseColor, not brush pixel)
                Buffer::PaintMode eraseMode = (mode == Buffer::BrushMode) ? Buffer::Normal : mode;
                Pen::applyPixelMode(p, buffer, eraseMode, effectiveErase, paintC, eraseC);
            } else if (mode == Buffer::BrushMode) {
                // Stamp the brush's own pixel colors
                buffer->image().setPixel(p, static_cast<uint>(idx));
            } else {
                Pen::applyPixelMode(p, buffer, mode, effectiveErase, paintC, eraseC);
            }
        }
    }
}

QRect Brush::paint(const QPoint &point, Buffer *buffer) const
{
    if (!buffer->segmentCheck(point)) return QRect();
    QRect imageRect = buffer->image().rect();
    QPoint origin = point - handleOffset_;
    brushStampAt(image_, transparentIndex_, buffer, origin, false);
    QRect changed = image_.rect().translated(origin);

    int cx = buffer->mirrorCenterX(), cy = buffer->mirrorCenterY();
    if (buffer->mirrorX()) {
        QPoint mxOrigin = QPoint(2 * cx - point.x(), point.y()) - handleOffset_;
        brushStampAt(image_, transparentIndex_, buffer, mxOrigin, false);
        changed = changed.united(image_.rect().translated(mxOrigin));
    }
    if (buffer->mirrorY()) {
        QPoint myOrigin = QPoint(point.x(), 2 * cy - point.y()) - handleOffset_;
        brushStampAt(image_, transparentIndex_, buffer, myOrigin, false);
        changed = changed.united(image_.rect().translated(myOrigin));
    }
    if (buffer->mirrorX() && buffer->mirrorY()) {
        QPoint mxyOrigin = QPoint(2 * cx - point.x(), 2 * cy - point.y()) - handleOffset_;
        brushStampAt(image_, transparentIndex_, buffer, mxyOrigin, false);
        changed = changed.united(image_.rect().translated(mxyOrigin));
    }
    return changed.intersected(imageRect);
}

QRect Brush::erase(const QPoint &point, Buffer *buffer) const
{
    if (!buffer->segmentCheck(point)) return QRect();
    QRect imageRect = buffer->image().rect();
    QPoint origin = point - handleOffset_;
    brushStampAt(image_, transparentIndex_, buffer, origin, true);
    QRect changed = image_.rect().translated(origin);

    int cx = buffer->mirrorCenterX(), cy = buffer->mirrorCenterY();
    if (buffer->mirrorX()) {
        QPoint mxOrigin = QPoint(2 * cx - point.x(), point.y()) - handleOffset_;
        brushStampAt(image_, transparentIndex_, buffer, mxOrigin, true);
        changed = changed.united(image_.rect().translated(mxOrigin));
    }
    if (buffer->mirrorY()) {
        QPoint myOrigin = QPoint(point.x(), 2 * cy - point.y()) - handleOffset_;
        brushStampAt(image_, transparentIndex_, buffer, myOrigin, true);
        changed = changed.united(image_.rect().translated(myOrigin));
    }
    if (buffer->mirrorX() && buffer->mirrorY()) {
        QPoint mxyOrigin = QPoint(2 * cx - point.x(), 2 * cy - point.y()) - handleOffset_;
        brushStampAt(image_, transparentIndex_, buffer, mxyOrigin, true);
        changed = changed.united(image_.rect().translated(mxyOrigin));
    }
    return changed.intersected(imageRect);
}

QRect Brush::rect(const QPoint &point) const
{
    return image_.rect().translated(point - handleOffset_);
}

const QImage &Brush::image() const
{
    return image_;
}

int Brush::transparentIndex() const
{
    return transparentIndex_;
}

void Brush::setTransparentIndex(int index)
{
    transparentIndex_ = index;
}

void Brush::remap(const QVector<QRgb> &palette)
{
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            QRgb color = image_.color(image_.pixelIndex(x, y));
            int bestIdx = 0, bestDist = INT_MAX;
            for (int i = 0; i < palette.size(); i++) {
                int dr = qRed(color)   - qRed(palette[i]);
                int dg = qGreen(color) - qGreen(palette[i]);
                int db = qBlue(color)  - qBlue(palette[i]);
                int dist = dr*dr + dg*dg + db*db;
                if (dist < bestDist) { bestDist = dist; bestIdx = i; }
            }
            image_.setPixel(x, y, static_cast<uint>(bestIdx));
        }
    }
}

void Brush::replaceColor(int fromIndex, int toIndex)
{
    for (int y = 0; y < image_.height(); y++)
        for (int x = 0; x < image_.width(); x++)
            if (image_.pixelIndex(x, y) == fromIndex)
                image_.setPixel(x, y, static_cast<uint>(toIndex));
}

// ── Private helper ─────────────────────────────────────────────────────────

QImage Brush::reindex(const QImage &src) const
{
    QImage result(src.width(), src.height(), QImage::Format_Indexed8);
    result.setColorTable(image_.colorTable());
    const QVector<QRgb> ct = image_.colorTable();
    for (int y = 0; y < src.height(); y++) {
        for (int x = 0; x < src.width(); x++) {
            QRgb color = src.pixel(x, y);
            int bestIdx = 0, bestDist = INT_MAX;
            for (int i = 0; i < ct.size(); i++) {
                int dr = qRed(color)   - qRed(ct[i]);
                int dg = qGreen(color) - qGreen(ct[i]);
                int db = qBlue(color)  - qBlue(ct[i]);
                int dist = dr*dr + dg*dg + db*db;
                if (dist < bestDist) { bestDist = dist; bestIdx = i; }
            }
            result.setPixel(x, y, static_cast<uint>(bestIdx));
        }
    }
    return result;
}

// ── Transforms ─────────────────────────────────────────────────────────────

void Brush::flipHorizontal() { image_ = image_.mirrored(true, false); }
void Brush::flipVertical()   { image_ = image_.mirrored(false, true); }

void Brush::rotate90CW()
{
    int w = image_.width(), h = image_.height();
    QImage result(h, w, QImage::Format_Indexed8);
    result.setColorTable(image_.colorTable());
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            result.setPixel(h - 1 - y, x, static_cast<uint>(image_.pixelIndex(x, y)));
    image_ = result;
    handleOffset_ = QPoint(image_.width() / 2, image_.height() / 2);
}

void Brush::rotate90CCW()
{
    int w = image_.width(), h = image_.height();
    QImage result(h, w, QImage::Format_Indexed8);
    result.setColorTable(image_.colorTable());
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            result.setPixel(y, w - 1 - x, static_cast<uint>(image_.pixelIndex(x, y)));
    image_ = result;
    handleOffset_ = QPoint(image_.width() / 2, image_.height() / 2);
}

void Brush::scale(int width, int height)
{
    if (width < 1)  width  = 1;
    if (height < 1) height = 1;
    QImage rgb = image_.convertToFormat(QImage::Format_ARGB32)
                       .scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    image_ = reindex(rgb);
    handleOffset_ = QPoint(image_.width() / 2, image_.height() / 2);
}

void Brush::doubleSize() { scale(image_.width() * 2, image_.height() * 2); }
void Brush::halveSize()  { scale(qMax(1, image_.width() / 2), qMax(1, image_.height() / 2)); }

void Brush::shearX(double factor)
{
    int w = image_.width(), h = image_.height();
    int maxShift = (int)std::ceil(qAbs(h * factor));
    int newW = w + maxShift;
    int bg = qMax(0, transparentIndex_);
    QImage result(newW, h, QImage::Format_Indexed8);
    result.setColorTable(image_.colorTable());
    result.fill(static_cast<uint>(bg));
    for (int y = 0; y < h; y++) {
        int shift = (int)(y * factor);
        int xOff = (factor >= 0) ? shift : shift + maxShift;
        for (int x = 0; x < w; x++) {
            int nx = x + xOff;
            if (nx >= 0 && nx < newW)
                result.setPixel(nx, y, static_cast<uint>(image_.pixelIndex(x, y)));
        }
    }
    image_ = result;
    handleOffset_ = QPoint(image_.width() / 2, image_.height() / 2);
}

void Brush::shearY(double factor)
{
    int w = image_.width(), h = image_.height();
    int maxShift = (int)std::ceil(qAbs(w * factor));
    int newH = h + maxShift;
    int bg = qMax(0, transparentIndex_);
    QImage result(w, newH, QImage::Format_Indexed8);
    result.setColorTable(image_.colorTable());
    result.fill(static_cast<uint>(bg));
    for (int x = 0; x < w; x++) {
        int shift = (int)(x * factor);
        int yOff = (factor >= 0) ? shift : shift + maxShift;
        for (int y = 0; y < h; y++) {
            int ny = y + yOff;
            if (ny >= 0 && ny < newH)
                result.setPixel(x, ny, static_cast<uint>(image_.pixelIndex(x, y)));
        }
    }
    image_ = result;
    handleOffset_ = QPoint(image_.width() / 2, image_.height() / 2);
}

void Brush::bendX(double amount)
{
    int w = image_.width(), h = image_.height();
    int maxShift = (int)std::ceil(qAbs(amount));
    int newW = w + 2 * maxShift;
    int bg = qMax(0, transparentIndex_);
    QImage result(newW, h, QImage::Format_Indexed8);
    result.setColorTable(image_.colorTable());
    result.fill(static_cast<uint>(bg));
    for (int y = 0; y < h; y++) {
        double t = h > 1 ? (double)y / (h - 1) : 0.0;
        int shift = (int)std::round(amount * std::sin(t * M_PI));
        for (int x = 0; x < w; x++) {
            int nx = x + maxShift + shift;
            if (nx >= 0 && nx < newW)
                result.setPixel(nx, y, static_cast<uint>(image_.pixelIndex(x, y)));
        }
    }
    image_ = result;
    handleOffset_ = QPoint(image_.width() / 2, image_.height() / 2);
}

void Brush::bendY(double amount)
{
    int w = image_.width(), h = image_.height();
    int maxShift = (int)std::ceil(qAbs(amount));
    int newH = h + 2 * maxShift;
    int bg = qMax(0, transparentIndex_);
    QImage result(w, newH, QImage::Format_Indexed8);
    result.setColorTable(image_.colorTable());
    result.fill(static_cast<uint>(bg));
    for (int x = 0; x < w; x++) {
        double t = w > 1 ? (double)x / (w - 1) : 0.0;
        int shift = (int)std::round(amount * std::sin(t * M_PI));
        for (int y = 0; y < h; y++) {
            int ny = y + maxShift + shift;
            if (ny >= 0 && ny < newH)
                result.setPixel(x, ny, static_cast<uint>(image_.pixelIndex(x, y)));
        }
    }
    image_ = result;
    handleOffset_ = QPoint(image_.width() / 2, image_.height() / 2);
}

void Brush::outline(int colorIndex)
{
    int bg = qMax(0, transparentIndex_);
    int w = image_.width(), h = image_.height();
    QImage result = image_.copy();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (image_.pixelIndex(x, y) == bg) {
                if ((x > 0   && image_.pixelIndex(x-1, y) != bg) ||
                    (x < w-1 && image_.pixelIndex(x+1, y) != bg) ||
                    (y > 0   && image_.pixelIndex(x, y-1) != bg) ||
                    (y < h-1 && image_.pixelIndex(x, y+1) != bg))
                    result.setPixel(x, y, static_cast<uint>(colorIndex));
            }
        }
    }
    image_ = result;
}

void Brush::trim()
{
    int bg = qMax(0, transparentIndex_);
    int w = image_.width(), h = image_.height();
    QImage result = image_.copy();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (image_.pixelIndex(x, y) != bg) {
                if ((x == 0  || image_.pixelIndex(x-1, y) == bg) ||
                    (x == w-1 || image_.pixelIndex(x+1, y) == bg) ||
                    (y == 0  || image_.pixelIndex(x, y-1) == bg) ||
                    (y == h-1 || image_.pixelIndex(x, y+1) == bg))
                    result.setPixel(x, y, static_cast<uint>(bg));
            }
        }
    }
    image_ = result;
}

void Brush::tileCut()
{
    int w = image_.width(), h = image_.height();
    if (w > 1 && h > 1)
        image_ = image_.copy(0, 0, w - 1, h - 1);
}

void Brush::storeOriginal()   { originalImage_ = image_; }
void Brush::restoreOriginal() { if (!originalImage_.isNull()) { image_ = originalImage_; handleOffset_ = QPoint(image_.width() / 2, image_.height() / 2); } }
bool Brush::hasOriginal() const { return !originalImage_.isNull(); }

void Brush::detectBackground()
{
    if (image_.width() < 1 || image_.height() < 1)
        return;
    int tl = image_.pixelIndex(0, 0);
    int tr = image_.pixelIndex(image_.width() - 1, 0);
    int bl = image_.pixelIndex(0, image_.height() - 1);
    int br = image_.pixelIndex(image_.width() - 1, image_.height() - 1);
    if (tl == tr && tl == bl && tl == br)
        transparentIndex_ = tl;
}
