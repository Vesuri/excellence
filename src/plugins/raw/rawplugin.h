#ifndef RAWPLUGIN_H
#define RAWPLUGIN_H

#include <QImageIOPlugin>
#include <QVariant>

// Custom ImageOption values above Qt's built-in range (last Qt value: ImageTransformation = 18)
namespace RawOption {
    enum Option {
        Interleave       = 19, // bool — interleave bitplanes line-by-line (default: true)
        WordAlign        = 20, // bool — pad row width to 16-bit boundary (default: true)
        PaletteDepth     = 21, // int  — bits per palette entry: 12 or 24 (default: 24)
        PalettePlacement = 22, // int  — RawOption::Placement value (default: PlacementEnd)
    };
    enum Placement {
        PlacementNone  = 0,
        PlacementStart = 1,
        PlacementEnd   = 2,
    };
}

class RawPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "raw.json")

public:
    RawPlugin(QObject *parent = nullptr);

    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
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
    bool interleave_;
    bool wordAlign_;
    int paletteDepth_;
    RawOption::Placement palettePlacement_;
};

#endif // RAWPLUGIN_H
