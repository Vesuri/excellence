#ifndef ILBMPLUGIN_H
#define ILBMPLUGIN_H

#include <QImageIOPlugin>
#include <QColor>
#include <QVariant>

class ILBMPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "ilbm.json")

public:
    ILBMPlugin(QObject *parent = nullptr);

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
    virtual QVariant option(QImageIOHandler::ImageOption option) const;
    virtual void setOption(QImageIOHandler::ImageOption option, const QVariant &value);
    virtual bool supportsOption(QImageIOHandler::ImageOption option) const;

private:
    QVariant compressionRatio;
};

#endif // ILBMPLUGIN_H
