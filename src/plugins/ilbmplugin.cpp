#include <QImage>
#include <QDebug>
#include "ilbmplugin.h"

ILBMPlugin::ILBMPlugin(QObject *parent) : QImageIOPlugin(parent)
{
}

QImageIOPlugin::Capabilities ILBMPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    bool isILBM = false;

    if (device == 0) {
        isILBM = format == "iff" || format == "ilbm";
    } else {
        QByteArray header = device->peek(12);
        isILBM = header.length() == 12 && header.startsWith("FORM") && header.endsWith("ILBM");
    }

    return isILBM ? (QImageIOPlugin::CanRead | QImageIOPlugin::CanWrite) : QImageIOPlugin::Capabilities();
}

QImageIOHandler *ILBMPlugin::create(QIODevice *device, const QByteArray &format) const
{
    return new ILBMHandler();
}

ILBMHandler::ILBMHandler() : QImageIOHandler()
{
}

bool ILBMHandler::canRead() const
{
    bool isILBM = false;
    if (device() != 0) {
        QByteArray header = device()->peek(12);
        isILBM = header.length() == 12 && header.startsWith("FORM") && header.endsWith("ILBM");
    }
    return isILBM;
}

bool ILBMHandler::read(QImage *outputImage)
{
    if (device() == 0) {
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
                Chunk chunk(form.data(offset));

                if (chunk.id() == "BMHD") {
                    bitmapHeader = BitmapHeader(chunk);
                } else if (chunk.id() == "CMAP") {
                    colorMap = ColorMap(chunk);
                } else if (chunk.id() == "BODY") {
                    image = QImage(bitmapHeader.width(), bitmapHeader.height(), QImage::Format_Indexed8);
                    image.setColorTable(colorMap.toVector());

                    int bytesPerPlane = ((bitmapHeader.width() + 15) & 0xfff0) / 8;
                    int planesPerRow = (bitmapHeader.planes() + (bitmapHeader.masking() != BitmapHeader::MaskingNone ? 1 : 0));
                    int bytesPerRow = bytesPerPlane * planesPerRow;

                    unsigned char planarRow[bytesPerRow];
                    for (int y = 0, index = 0; y < image.height(); y++) {
                        unsigned char *planar = planarRow;
                        unsigned char *planarEnd = planar + bytesPerRow;

                        if (bitmapHeader.compression() == BitmapHeader::CompressionNone) {
                            // No compression: copy a row of planar data as is
                            while (planar < planarEnd) {
                                *planar++ = chunk.byte(index++);
                            }
                        } else {
                            // Runlength compression: convert encoded data until the row is full
                            while (planar < planarEnd) {
                                int count = chunk.byte(index++);
                                if (count >= 0) {
                                    // Count positive: copy the following count + 1 bytes as is
                                    for (int i = 0; i < count + 1; i++) {
                                        *planar++ = chunk.byte(index++);
                                    }
                                } else {
                                    // Count negative: repeat the following byte (-count + 1) times
                                    for (int i = 0; i < -count + 1; i++) {
                                        *planar++ = chunk.byte(index);
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
                } else if (chunk.id() == "CAMG") {
                    commodoreAmiga = CommodoreAmiga(chunk);
                }

                offset += chunk.size() + 8;
            }
        }
    }

    if (commodoreAmiga.modes() & CommodoreAmiga::ExtraHalfbrite) {
        for (int i = 0; i < image.colorCount() / 2; i++) {
            QRgb color = image.color(i);
            image.setColor(image.colorCount() / 2 + i, (color & 0xff000000) | ((color & 0x00fefefe) >> 1));
        }
    }
    *outputImage = image;
    return !image.isNull();
}

ILBMHandler::Chunk::Chunk()
{
    chunk.append("NONE");
    chunk.append((char)0);
    chunk.append((char)0);
    chunk.append((char)0);
    chunk.append((char)0);
}

ILBMHandler::Chunk::Chunk(const QByteArray &chunk) :
    chunk(chunk)
{
}

ILBMHandler::Chunk::Chunk(const Chunk &chunk) :
    chunk(chunk.chunk)
{
}

QByteArray ILBMHandler::Chunk::id() const
{
    return chunk.left(4);
}

unsigned ILBMHandler::Chunk::size() const
{
    return ulong(-4);
}

QByteArray ILBMHandler::Chunk::data(int offset, int length) const
{
    return chunk.mid(8 + offset, length >= 0 ? length : size());
}

char ILBMHandler::Chunk::byte(int offset) const
{
    return chunk.at(8 + offset);
}

unsigned char ILBMHandler::Chunk::ubyte(int offset) const
{
    return (unsigned char)chunk.at(8 + offset);
}

short ILBMHandler::Chunk::word(int offset) const
{
    return ((char)chunk.at(8 + offset) << 8) | (char)chunk.at(9 + offset);
}

unsigned short ILBMHandler::Chunk::uword(int offset) const
{
    return ((unsigned char)chunk.at(8 + offset) << 8) | (unsigned char)chunk.at(9 + offset);
}

unsigned ILBMHandler::Chunk::ulong(int offset) const
{
    return ((unsigned char)chunk.at(8 + offset) << 24) | ((unsigned char)chunk.at(9 + offset) << 16) | ((unsigned char)chunk.at(10 + offset) << 8) | (unsigned char)chunk.at(11 + offset);
}

ILBMHandler::BitmapHeader::BitmapHeader()
{
}

ILBMHandler::BitmapHeader::BitmapHeader(const Chunk &chunk) : Chunk(chunk)
{
}

unsigned short ILBMHandler::BitmapHeader::width() const
{
    return uword(0);
}

unsigned short ILBMHandler::BitmapHeader::height() const
{
    return uword(2);
}

short ILBMHandler::BitmapHeader::x() const
{
    return word(4);
}

short ILBMHandler::BitmapHeader::y() const
{
    return word(6);
}

unsigned char ILBMHandler::BitmapHeader::planes() const
{
    return ubyte(8);
}

ILBMHandler::BitmapHeader::Masking ILBMHandler::BitmapHeader::masking() const
{
    return static_cast<Masking>(ubyte(9));
}

ILBMHandler::BitmapHeader::Compression ILBMHandler::BitmapHeader::compression() const
{
    return static_cast<Compression>(ubyte(10));
}

unsigned short ILBMHandler::BitmapHeader::transparentColor() const
{
    return uword(12);
}

unsigned char ILBMHandler::BitmapHeader::xAspect() const
{
    return ubyte(14);
}

unsigned char ILBMHandler::BitmapHeader::yAspect() const
{
    return ubyte(15);
}

short ILBMHandler::BitmapHeader::pageWidth() const
{
    return word(16);
}

short ILBMHandler::BitmapHeader::pageHeight() const
{
    return word(18);
}

ILBMHandler::ColorMap::ColorMap()
{
}

ILBMHandler::ColorMap::ColorMap(const Chunk &chunk) : Chunk(chunk)
{
}

unsigned ILBMHandler::ColorMap::count() const
{
    return size() / 3;
}

QRgb ILBMHandler::ColorMap::at(unsigned index) const
{
    return 0xff000000 | (ubyte(index * 3) << 16) | (ubyte(index * 3 + 1) << 8) | ubyte(index * 3 + 2);
}

QVector<QRgb> ILBMHandler::ColorMap::toVector() const
{
    QVector<QRgb> vector;
    for (unsigned i = 0; i < count(); i++) {
        vector.append(at(i));
    }
    return vector;
}

ILBMHandler::CommodoreAmiga::CommodoreAmiga()
{
}

ILBMHandler::CommodoreAmiga::CommodoreAmiga(const Chunk &chunk) : Chunk(chunk)
{
}

ILBMHandler::CommodoreAmiga::Modes ILBMHandler::CommodoreAmiga::modes() const
{
    return static_cast<Modes>(ulong(0));
}
