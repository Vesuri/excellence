#include <QImage>
#include <QSharedPointer>

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
    compressionRatio(0)
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

    QSharedPointer<BitmapHeader> bitmapHeader;
    QSharedPointer<ColorMap> colorMap;
    QSharedPointer<CommodoreAmiga> commodoreAmiga(new CommodoreAmiga());
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
                    Body body = Body(chunk);

                    // The bitmap header and colormap must be valid at this point
                    image = body.toImage(*bitmapHeader, *colorMap, *commodoreAmiga);
                } else {
                    asIsChunks.append(chunk.toByteArray());
                }

                // A chunk's total physical size is ckSize rounded up to an even number plus the size of the header
                offset += ((chunk.size() + 1) & 0xfffffffe) + 8;
            }
        }
    }

    if (!commodoreAmiga.isNull()) {
        asIsChunks.append(commodoreAmiga->toByteArray());
    }

    if (asIsChunks.size() > 0) {
        image.setText(QString(), QString(asIsChunks.toBase64()));
    }

    *outputImage = image;
    return !image.isNull();
}

bool ILBMHandler::write(const QImage &image)
{
    if (device() == nullptr || !device()->isWritable()) {
        return false;
    }

    BitmapHeader::Compression compression(static_cast<BitmapHeader::Compression>(compressionRatio.toInt()));
    BitmapHeader bitmapHeader(image, compression);
    ColorMap colorMap(image);
    Body body(image, compression);

    QByteArray ilbm("ILBM");
    ilbm.append(bitmapHeader.toByteArray());
    ilbm.append(colorMap.toByteArray());

    QString imageText = image.text();
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
    device()->write(form.toByteArray());

    return true;
}

QVariant ILBMHandler::option(QImageIOHandler::ImageOption option) const
{
    if (option == QImageIOHandler::CompressionRatio) {
        return compressionRatio;
    } else {
        return QImageIOHandler::option(option);
    }
}

void ILBMHandler::setOption(QImageIOHandler::ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::CompressionRatio) {
        compressionRatio = value;
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
