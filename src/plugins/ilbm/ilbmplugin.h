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

    Capabilities capabilities(QIODevice *device, const QByteArray & format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray & format = QByteArray()) const override;
};

class ILBMHandler : public QImageIOHandler
{
public:
    ILBMHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;
    QVariant option(QImageIOHandler::ImageOption option) const override;
    void setOption(QImageIOHandler::ImageOption option, const QVariant &value) override;
    bool supportsOption(QImageIOHandler::ImageOption option) const override;

private:
    QVariant compressionRatio;
};

#endif // ILBMPLUGIN_H
