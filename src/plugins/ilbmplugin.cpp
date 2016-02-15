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

bool ILBMHandler::read(QImage *image)
{
    if (device() == 0) {
        return false;
    }

    int planes = 0;
    Masking masking = MaskingNone;
    Compression compression = CompressionNone;
    QImage tempImage;
    QVector<QRgb> colorTable;
    unsigned commodoreAmiga;
    Chunk form(device()->readAll());
    if (form.id() == "FORM") {
        if (form.data(0, 4) == "ILBM") {
            for (unsigned offset = 4; offset < form.size();) {
                Chunk chunk(form.data(offset));

                if (chunk.id() == "BMHD") {
                    tempImage = QImage(chunk.uword(0), chunk.uword(2), QImage::Format_Indexed8);
                    planes = chunk.ubyte(8);
                    masking = static_cast<Masking>(chunk.ubyte(9));
                    compression = static_cast<Compression>(chunk.ubyte(10));
                } else if (chunk.id() == "CMAP") {
                    for (unsigned i = 0; i < chunk.size() / 3; i++) {
                        colorTable.append(0xff000000 | (chunk.ubyte(i * 3) << 16) | (chunk.ubyte(i * 3 + 1) << 8) | chunk.ubyte(i * 3 + 2));
                    }
                    tempImage.setColorTable(colorTable);
                } else if (chunk.id() == "BODY") {
                    int bytesPerPlane = ((tempImage.width() + 15) & 0xfff0) / 8;
                    int planesPerRow = (planes + (masking != MaskingNone ? 1 : 0));
                    int bytesPerRow = bytesPerPlane * planesPerRow;

                    qWarning("XX IMAGE WIDTH %d", tempImage.width());
                    qWarning("XX IMAGE HEIGHT %d", tempImage.height());
                    qWarning("XX PLANES %d", planes);
                    qWarning("XX MASK %d", masking);
                    qWarning("XX BYTES PER PLANE %d", bytesPerPlane);
                    qWarning("XX PLANES PER ROW %d", planesPerRow);
                    qWarning("XX BYTES PER ROW %d", bytesPerRow);

                    unsigned char row[bytesPerRow];
                    for (int y = 0, rowOffset = 0; y < tempImage.height(); y++) {
                        if (compression == CompressionNone) {
                            for (int inputOffset = 0; inputOffset < bytesPerRow; inputOffset++) {
                                row[inputOffset] = chunk.byte(rowOffset + inputOffset);
                            }
                            rowOffset += bytesPerRow;
                        } else {
                            for (int inputOffset = 0, outputOffset = 0; outputOffset < bytesPerRow;) {
                                int count = chunk.byte(rowOffset + inputOffset++);
                                if (count > 0) {
                                    for (int i = 0; i < count + 1; i++) {
                                        row[outputOffset++] = chunk.byte(rowOffset + inputOffset++);
                                    }
                                } else {
                                    for (int i = 0; i < -count + 1; i++) {
                                        row[outputOffset++] = chunk.byte(rowOffset + inputOffset);
                                    }
                                    inputOffset++;
                                }

                                if (outputOffset >= bytesPerRow) {
                                    rowOffset += inputOffset;
                                }
                            }
                        }

                        for (int x = 0; x < tempImage.width(); x++) {
                            unsigned char mask = 0x80 >> (x & 7);
                            unsigned char shift = 7 - (x & 7);
                            unsigned char pixel = 0;
                            for (int plane = 0; plane < planes; plane++) {
                                pixel |= ((row[plane * bytesPerPlane + x / 8] & mask) >> shift) << plane;
                            }
                            tempImage.setPixel(x, y, pixel);
                        }
                    }
                } else if (chunk.id() == "CAMG") {
                    commodoreAmiga = chunk.ulong(0);
                }

                offset += chunk.size() + 8;
            }
        }
    }

    // TODO only do this if commodoreAmiga has EHB set
    if (colorTable.count() > 32) {
        for (int i = 0; i < 32; i++) {
            QRgb color = colorTable[i];
            colorTable[i + 32] = (color & 0xff000000) | ((color & 0x00fefefe) >> 1);
        }
    }
    tempImage.setColorTable(colorTable);
    *image = tempImage;
    return !tempImage.isNull();
}

ILBMHandler::Chunk::Chunk(const QByteArray &chunk) :
    chunk(chunk)
{
    qWarning() << "GOT CHUNK" << id() << "OF SIZE" << size();
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

unsigned short ILBMHandler::Chunk::uword(int offset) const
{
    return ((unsigned char)chunk.at(8 + offset) << 8) | (unsigned char)chunk.at(9 + offset);
}

unsigned ILBMHandler::Chunk::ulong(int offset) const
{
    return ((unsigned char)chunk.at(8 + offset) << 24) | ((unsigned char)chunk.at(9 + offset) << 16) | ((unsigned char)chunk.at(10 + offset) << 8) | (unsigned char)chunk.at(11 + offset);
}
