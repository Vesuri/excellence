#include <QImage>

#include "chunk.h"
#include "bitmapheader.h"
#include "colormap.h"
#include "commodoreamiga.h"
#include "ilbmplugin.h"

ILBMPlugin::ILBMPlugin(QObject *parent) : QImageIOPlugin(parent)
{
}

QImageIOPlugin::Capabilities ILBMPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    bool isReadable = device == nullptr || device->isReadable();
    bool isWritable = device == nullptr || device->isWritable();
    bool isILBM = format == "iff" || format == "ilbm";

    if (device != nullptr && device->isReadable()) {
        QByteArray header = device->peek(12);
        isILBM = header.length() == 12 && header.startsWith("FORM") && header.endsWith("ILBM");
    }

    return isILBM ? ((isReadable ? QImageIOPlugin::CanRead : Capabilities()) | (isWritable ? QImageIOPlugin::CanWrite : Capabilities())) : Capabilities();
}

QImageIOHandler *ILBMPlugin::create(QIODevice */*device*/, const QByteArray &/*format*/) const
{
    return new ILBMHandler();
}

ILBMHandler::ILBMHandler() : QImageIOHandler()
{
}

bool ILBMHandler::canRead() const
{
    bool isILBM = false;
    if (device() != nullptr && device()->isReadable()) {
        QByteArray header = device()->peek(12);
        isILBM = header.length() == 12 && header.startsWith("FORM") && header.endsWith("ILBM");
    }
    return isILBM;
}

bool ILBMHandler::read(QImage *outputImage)
{
    if (device() == nullptr || !device()->isReadable()) {
        return false;
    }

    BitmapHeader bitmapHeader;
    ColorMap colorMap;
    CommodoreAmiga commodoreAmiga;
    QImage image;
    Chunk form(device()->readAll());
    if (form.id() == "FORM") {
        if (form.data(0, 4) == "ILBM") {
            for (unsigned offset = 4; offset < form.size();) {
                Chunk chunk(form.data(static_cast<int>(offset)));

                if (chunk.id() == "BMHD") {
                    bitmapHeader = BitmapHeader(chunk);
                } else if (chunk.id() == "CMAP") {
                    colorMap = ColorMap(chunk);
                } else if (chunk.id() == "CAMG") {
                    commodoreAmiga = CommodoreAmiga(chunk);
                } else if (chunk.id() == "BODY") {
                    // Create a image based on the BODY chunk: the bitmap header and colormap should be valid at this point
                    image = QImage(bitmapHeader.width(), bitmapHeader.height(), QImage::Format_Indexed8);

                    // Store the aspect ratio of the image
                    float aspectRatio = bitmapHeader.xAspect() / static_cast<float>(bitmapHeader.yAspect());
                    image.setDotsPerMeterY(static_cast<int>(image.dotsPerMeterY() * aspectRatio));

                    QVector<QRgb> colorTable = colorMap.toVector();
                    if (!commodoreAmiga.isNull() && (commodoreAmiga.modes() & CommodoreAmiga::ExtraHalfbrite)) {
                        // Make sure Extra Halfbrite images have at least 64 colors
                        while (colorTable.size() < 64) {
                            colorTable.append(0);
                        }

                        // Halve the intensity of the latter half of the palette for Extra Halfbrite images
                        for (int i = 0; i < colorTable.size() / 2; i++) {
                            colorTable[colorTable.size() / 2 + i] = (colorTable[i] & 0xff000000) | ((colorTable[i] & 0x00fefefe) >> 1);
                        }
                    }
                    image.setColorTable(colorTable);

                    // Data is padded to word boundaries
                    int bytesPerPlane = ((bitmapHeader.width() + 15) & 0xfff0) / 8;
                    int planesPerRow = (bitmapHeader.planes() + (bitmapHeader.masking() == BitmapHeader::MaskingHasMask ? 1 : 0));
                    int bytesPerRow = bytesPerPlane * planesPerRow;

                    unsigned char *planarRow = new unsigned char[bytesPerRow];
                    for (int y = 0, index = 0; y < image.height(); y++) {
                        unsigned char *planar = planarRow;
                        unsigned char *planarEnd = planar + bytesPerRow;

                        if (bitmapHeader.compression() == BitmapHeader::CompressionNone) {
                            // No compression: copy a row of planar data as is
                            while (planar < planarEnd) {
                                *planar++ = chunk.ubyte(index++);
                            }
                        } else {
                            // Runlength compression: convert encoded data until the row is full
                            while (planar < planarEnd) {
                                int count = chunk.byte(index++);
                                if (count >= 0) {
                                    // Count positive: copy the following count + 1 bytes as is
                                    for (int i = 0; i < count + 1; i++) {
                                        *planar++ = chunk.ubyte(index++);
                                    }
                                } else {
                                    // Count negative: repeat the following byte (-count + 1) times
                                    for (int i = 0; i < -count + 1; i++) {
                                        *planar++ = chunk.ubyte(index);
                                    }
                                    index++;
                                }
                            }
                        }

                        // Convert the planar data to chunky
                        unsigned char *chunky = image.scanLine(y);
                        for (int x = 0; x < image.width(); x++, chunky++) {
                            unsigned char mask = 0x80 >> (x & 7);
                            unsigned char shift = 7 - (x & 7);
                            *chunky = 0;
                            for (int plane = 0; plane < bitmapHeader.planes(); plane++) {
                                *chunky |= ((planarRow[plane * bytesPerPlane + x / 8] & mask) >> shift) << plane;
                            }
                        }
                    }
                    delete[] planarRow;
                } else {
                    image.setText(QString(chunk.id()), QString(chunk.data().toBase64()));
                }

                // A chunk's total physical size is ckSize rounded up to an even number plus the size of the header
                offset += ((chunk.size() + 1) & 0xfffffffe) + 8;
            }
        }
    }

    *outputImage = image;
    return !image.isNull();
}

bool ILBMHandler::write(const QImage &image)
{
    if (device() == nullptr || !device()->isWritable()) {
        return false;
    }

    BitmapHeader bitmapHeader;
    ColorMap colorMap;
    CommodoreAmiga commodoreAmiga;
    QByteArray ilbm("ILBM");
    ilbm.append(bitmapHeader.toByteArray());
    ilbm.append(colorMap.toByteArray());
    ilbm.append(commodoreAmiga.toByteArray());
    Chunk form("FORM", ilbm);
    device()->write(form.toByteArray());

    return true;
}
