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

    QSharedPointer<BitmapHeader> bitmapHeader;
    QSharedPointer<ColorMap> colorMap;
    QSharedPointer<CommodoreAmiga> commodoreAmiga;
    QImage image;
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

    BitmapHeader bitmapHeader(image);
    ColorMap colorMap(image);
    CommodoreAmiga commodoreAmiga(image);
    Body body(image);

    QByteArray ilbm("ILBM");
    ilbm.append(bitmapHeader.toByteArray());
    ilbm.append(colorMap.toByteArray());
    ilbm.append(commodoreAmiga.toByteArray());
    ilbm.append(body.toByteArray());
    Chunk form("FORM", ilbm);
    device()->write(form.toByteArray());

    return true;
}
