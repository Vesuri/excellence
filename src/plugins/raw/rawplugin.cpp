#include <QImage>
#include <QSharedPointer>

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
    compressionRatio(-1)
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

    bool interleave = true;
    bool wordAlign = true;
    QString palettePlacement = "end";
    int paletteDepth = 24;

    int width = (image.width() + (wordAlign ? 15 : 7)) & (wordAlign ? 0xfff0 : 0xfff8);
    int height = image.height();
    int bitplanes = 0;
    for (int colors = image.colorCount() - 1; colors > 0; colors >>= 1) {
        bitplanes++;
    }

    int lineWidth = width / 8;
    int bitplaneDelta = interleave ? lineWidth : (lineWidth * height);
    int lineDelta = interleave ? (lineWidth * bitplanes) : lineWidth;

    int dataSize = lineWidth * height * bitplanes;
    int paletteSize = image.colorCount() * paletteDepth / 6;
    int dataOffset = palettePlacement == "start" ? paletteSize : 0;
    int paletteOffset = palettePlacement == "end" ? dataSize : 0;

    if (!palettePlacement.isEmpty()) {
        dataSize += paletteSize;
    }

    unsigned char *data = new unsigned char[dataSize];
    memset(data, 0, static_cast<size_t>(dataSize));

    if (!palettePlacement.isEmpty()) {
        for (int c = 0; c < image.colorCount(); c++) {
            QRgb rgba = image.color(c);
            if (paletteDepth == 12) {
                data[paletteOffset + 2 * c] = static_cast<unsigned char>(qRed(rgba) >> 4);
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
    if (option == QImageIOHandler::CompressionRatio) {
        return compressionRatio;
    } else {
        return QImageIOHandler::option(option);
    }
}

void RawHandler::setOption(QImageIOHandler::ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::CompressionRatio) {
        compressionRatio = value;
    } else {
        QImageIOHandler::setOption(option, value);
    }
}

bool RawHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
    if (option == QImageIOHandler::CompressionRatio) {
        return true;
    } else {
        return QImageIOHandler::supportsOption(option);
    }
}
