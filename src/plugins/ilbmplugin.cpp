#include "ilbmplugin.h"

ILBMPlugin::ILBMPlugin(QObject *parent) : QImageIOPlugin(parent)
{
}

QImageIOPlugin::Capabilities ILBMPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    return ((device == 0 && (format == "iff" || format == "ilbm")) || device->peek(4) == "FORM") ? (QImageIOPlugin::CanRead | QImageIOPlugin::CanWrite) : QImageIOPlugin::Capabilities();
}

QImageIOHandler *ILBMPlugin::create(QIODevice *device, const QByteArray &format) const
{
    return 0;
}
