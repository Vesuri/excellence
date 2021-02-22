#ifndef RAWPLUGIN_H
#define RAWPLUGIN_H

#include <QImageIOPlugin>
#include <QColor>
#include <QVariant>

class RawPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "raw.json")

public:
    RawPlugin(QObject *parent = nullptr);

    Capabilities capabilities(QIODevice *device, const QByteArray & format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray & format = QByteArray()) const override;
};

class RawHandler : public QImageIOHandler
{
public:
    RawHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;
    QVariant option(QImageIOHandler::ImageOption option) const override;
    void setOption(QImageIOHandler::ImageOption option, const QVariant &value) override;
    bool supportsOption(QImageIOHandler::ImageOption option) const override;

private:
    QVariant compressionRatio;
};

#endif // RAWPLUGIN_H
