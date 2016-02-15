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

    int width = 0;
    int height = 0;
    int depth = 0;
    int compression = 0;
    QVector<QRgb> colorTable;
    Chunk form(device()->readAll());
    if (form.id() == "FORM") {
        if (form.data(0, 4) == "ILBM") {
            for (unsigned offset = 4; offset < form.size();) {
                Chunk chunk(form.data(offset));

                if (chunk.id() == "BMHD") {
                    width = chunk.uword(0);
                    height = chunk.uword(2);
                    depth = chunk.ubyte(8);
                    compression = chunk.ubyte(10);
                } else if (chunk.id() == "CMAP") {
                    for (unsigned i = 0; i < chunk.size() / 3; i++) {
                        colorTable.append(0xff000000 | (chunk.ubyte(i * 3) << 16) | (chunk.ubyte(i * 3 + 1) << 8) | chunk.ubyte(i * 3 + 2));
                    }
                } else if (chunk.id() == "BODY") {

                }
                offset += chunk.size() + 8;
            }
        }
    }

    bool isValid = width > 0 && height > 0 && depth > 0;
    if (isValid) {
        QImage tempImage(width, height, QImage::Format_Indexed8);
        tempImage.setColorTable(colorTable);
        *image = tempImage;
    }
    return isValid;
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
