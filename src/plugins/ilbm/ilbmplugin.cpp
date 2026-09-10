#include <QImage>
#include <QScopedPointer>

#include "chunk.h"
#include "bitmapheader.h"
#include "colormap.h"
#include "commodoreamiga.h"
#include "body.h"
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

ILBMHandler::ILBMHandler() : QImageIOHandler(),
    compressionRatio_(-1)
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

    QScopedPointer<BitmapHeader> bitmapHeader;
    QScopedPointer<ColorMap> colorMap;
    QScopedPointer<CommodoreAmiga> commodoreAmiga(new CommodoreAmiga);
    QImage image;
    QByteArray asIsChunks;
    Chunk form(device()->readAll());
    if (form.id() == "FORM") {
        if (form.data(0, 4) == "ILBM") {
            for (unsigned offset = 4; offset < form.size();) {
                Chunk chunk(form.data(offset));

                if (chunk.id() == "BMHD") {
                    bitmapHeader.reset(new BitmapHeader(chunk));
                } else if (chunk.id() == "CMAP") {
                    colorMap.reset(new ColorMap(chunk));
                } else if (chunk.id() == "CAMG") {
                    commodoreAmiga.reset(new CommodoreAmiga(chunk));
                } else if (chunk.id() == "BODY") {
                    if (!bitmapHeader || !colorMap)
                        return false;
                    Body body(chunk);
                    image = body.toImage(*bitmapHeader, *colorMap, *commodoreAmiga);
                } else {
                    asIsChunks.append(chunk.toByteArray());
                }

                offset += ((chunk.size() + 1) & 0xfffffffe) + 8;
            }
        }
    }

    if (!commodoreAmiga.isNull()) {
        asIsChunks.append(commodoreAmiga->toByteArray());
    }

    if (!asIsChunks.isEmpty()) {
        image.setText("Unknown ILBM chunks", QString(asIsChunks.toBase64()));
    }

    *outputImage = image;
    return !image.isNull();
}

bool ILBMHandler::write(const QImage &image)
{
    if (device() == nullptr || !device()->isWritable()) {
        return false;
    }

    BitmapHeader::Compression compression(compressionRatio_.toInt() >= 0 ? static_cast<BitmapHeader::Compression>(compressionRatio_.toInt()) : BitmapHeader::CompressionByteRun1);
    BitmapHeader bitmapHeader(image, compression);
    ColorMap colorMap(image);
    Body body(image, compression);

    QByteArray ilbm("ILBM");
    ilbm.append(bitmapHeader.toByteArray());
    ilbm.append(colorMap.toByteArray());

    QString imageText = image.text("Unknown ILBM chunks");
    if (!imageText.isEmpty()) {
        QByteArray asIsChunks = QByteArray::fromBase64(imageText.toLocal8Bit());

        for (unsigned offset = 0; offset < static_cast<unsigned>(asIsChunks.size());) {
            Chunk chunk(asIsChunks.mid(static_cast<int>(offset)));

            ilbm.append(chunk.toByteArray());

            offset += ((chunk.size() + 1) & 0xfffffffe) + 8;
        }
    }

    ilbm.append(body.toByteArray());
    Chunk form("FORM", ilbm);
    const QByteArray data = form.toByteArray();
    return device()->write(data) == data.size();
}

QVariant ILBMHandler::option(QImageIOHandler::ImageOption option) const
{
    if (option == QImageIOHandler::CompressionRatio) {
        return compressionRatio_;
    } else {
        return QImageIOHandler::option(option);
    }
}

void ILBMHandler::setOption(QImageIOHandler::ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::CompressionRatio) {
        compressionRatio_ = value;
    } else {
        QImageIOHandler::setOption(option, value);
    }
}

bool ILBMHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
    if (option == QImageIOHandler::CompressionRatio) {
        return true;
    } else {
        return QImageIOHandler::supportsOption(option);
    }
}
