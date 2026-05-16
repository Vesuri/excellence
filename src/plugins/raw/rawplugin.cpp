#include <QImage>

#include "rawplugin.h"

RawPlugin::RawPlugin(QObject *parent) : QImageIOPlugin(parent)
{
}

QImageIOPlugin::Capabilities RawPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    bool isWritable = device == nullptr || device->isWritable();
    bool isRaw = format == "raw";

    return isRaw ? (isWritable ? QImageIOPlugin::CanWrite : Capabilities()) : Capabilities();
}

QImageIOHandler *RawPlugin::create(QIODevice */*device*/, const QByteArray &/*format*/) const
{
    return new RawHandler();
}

RawHandler::RawHandler() : QImageIOHandler(),
    interleave_(true),
    wordAlign_(true),
    paletteDepth_(24),
    palettePlacement_(RawOption::PlacementEnd)
{
}

bool RawHandler::canRead() const
{
    return false;
}

bool RawHandler::read(QImage *)
{
    return false;
}

bool RawHandler::write(const QImage &image)
{
    if (device() == nullptr || !device()->isWritable()) {
        return false;
    }

    int width = (image.width() + (wordAlign_ ? 15 : 7)) & (wordAlign_ ? 0xfff0 : 0xfff8);
    int height = image.height();
    int bitplanes = 0;
    for (int colors = image.colorCount() - 1; colors > 0; colors >>= 1) {
        bitplanes++;
    }

    int lineWidth = width / 8;
    int bitplaneDelta = interleave_ ? lineWidth : (lineWidth * height);
    int lineDelta = interleave_ ? (lineWidth * bitplanes) : lineWidth;

    int dataSize = lineWidth * height * bitplanes;
    int paletteSize = image.colorCount() * paletteDepth_ / 6;
    int dataOffset = palettePlacement_ == RawOption::PlacementStart ? paletteSize : 0;
    int paletteOffset = palettePlacement_ == RawOption::PlacementEnd ? dataSize : 0;

    if (palettePlacement_ != RawOption::PlacementNone) {
        dataSize += paletteSize;
    }

    unsigned char *data = new unsigned char[dataSize];
    memset(data, 0, static_cast<size_t>(dataSize));

    if (palettePlacement_ != RawOption::PlacementNone) {
        for (int c = 0; c < image.colorCount(); c++) {
            QRgb rgba = image.color(c);
            if (paletteDepth_ == 12) {
                data[paletteOffset + 2 * c]     = static_cast<unsigned char>(qRed(rgba) >> 4);
                data[paletteOffset + 2 * c + 1] = static_cast<unsigned char>(qGreen(rgba) & 0xf0) | static_cast<unsigned char>(qBlue(rgba) >> 4);
            } else {
                data[paletteOffset + 4 * c + 1] = static_cast<unsigned char>(qRed(rgba));
                data[paletteOffset + 4 * c + 2] = static_cast<unsigned char>(qGreen(rgba));
                data[paletteOffset + 4 * c + 3] = static_cast<unsigned char>(qBlue(rgba));
            }
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < image.width(); x++) {
            unsigned char bit = 0x80 >> (x & 7);
            int pixel = image.pixelIndex(x, y);

            if (pixel > 0) {
                for (int bitplane = 0; bitplane < bitplanes; bitplane++) {
                    data[dataOffset + bitplane * bitplaneDelta + y * lineDelta + x / 8] |= (pixel & (1 << bitplane)) ? bit : 0;
                }
            }
        }
    }

    qint64 remainingBytes = dataSize;
    for (char *buffer = reinterpret_cast<char *>(data); remainingBytes > 0;) {
        qint64 result = device()->write(buffer, remainingBytes);
        if (result < 0) {
            break;
        }
        remainingBytes -= result;
        buffer += result;
    }
    delete [] data;

    return true;
}

QVariant RawHandler::option(QImageIOHandler::ImageOption option) const
{
    switch (static_cast<int>(option)) {
    case RawOption::Interleave:       return interleave_;
    case RawOption::WordAlign:        return wordAlign_;
    case RawOption::PaletteDepth:     return paletteDepth_;
    case RawOption::PalettePlacement: return static_cast<int>(palettePlacement_);
    default:                          return QImageIOHandler::option(option);
    }
}

void RawHandler::setOption(QImageIOHandler::ImageOption option, const QVariant &value)
{
    switch (static_cast<int>(option)) {
    case RawOption::Interleave:       interleave_ = value.toBool();                                         break;
    case RawOption::WordAlign:        wordAlign_ = value.toBool();                                          break;
    case RawOption::PaletteDepth:     paletteDepth_ = value.toInt();                                        break;
    case RawOption::PalettePlacement: palettePlacement_ = static_cast<RawOption::Placement>(value.toInt()); break;
    default:                          QImageIOHandler::setOption(option, value);                            break;
    }
}

bool RawHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
    switch (static_cast<int>(option)) {
    case RawOption::Interleave:
    case RawOption::WordAlign:
    case RawOption::PaletteDepth:
    case RawOption::PalettePlacement:
        return true;
    default:
        return QImageIOHandler::supportsOption(option);
    }
}
