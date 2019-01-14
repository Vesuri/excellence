#ifndef ILBMPLUGIN_H
#define ILBMPLUGIN_H

#include <QImageIOPlugin>
#include <QColor>

class ILBMPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "ilbm.json")

public:
    ILBMPlugin(QObject *parent = 0);

    virtual Capabilities capabilities(QIODevice *device, const QByteArray & format) const;
    virtual QImageIOHandler *create(QIODevice *device, const QByteArray & format = QByteArray()) const;
};

class ILBMHandler : public QImageIOHandler
{
public:
    ILBMHandler();

    virtual bool canRead() const;
    virtual bool read(QImage *image);
    virtual bool write(const QImage &image);
};

#endif // ILBMPLUGIN_H
