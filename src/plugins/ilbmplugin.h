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

private:
    class Chunk {
    public:
        Chunk();
        Chunk(const QByteArray &chunk);
        Chunk(const Chunk &chunk);

        QByteArray id() const;
        unsigned size() const;
        QByteArray data(int offset = 0, int length = -1) const;
        char byte(int offset) const;
        unsigned char ubyte(int offset) const;
        short word(int offset) const;
        unsigned short uword(int offset) const;
        unsigned ulong(int offset) const;

    private:
        QByteArray chunk;
    };

    class BitmapHeader : public Chunk {
    public:
        enum Compression {
            CompressionNone = 0,
            CompressionByteRun1 = 1
        };

        enum Masking {
            MaskingNone = 0,
            MaskingHasMask = 1,
            MaskingHasTransparentColor = 2,
            MaskingLasso = 3
        };

        BitmapHeader();
        BitmapHeader(const Chunk &chunk);

        unsigned short width() const;
        unsigned short height() const;
        short x() const;
        short y() const;
        unsigned char planes() const;
        Masking masking() const;
        Compression compression() const;
        unsigned short transparentColor() const;
        unsigned char xAspect() const;
        unsigned char yAspect() const;
        short pageWidth() const;
        short pageHeight() const;
    };

    class ColorMap : public Chunk {
    public:
        ColorMap();
        ColorMap(const Chunk &chunk);

        unsigned count() const;
        QRgb at(unsigned index) const;
        QVector<QRgb> toVector() const;
    };

    class CommodoreAmiga : public Chunk {
    public:
        enum Modes {
            Lace = 0x0004,
            SuperHires = 0x0020,
            ExtraHalfbrite = 0x0080,
            HAM = 0x0800,
            Hires = 0x8000
        };

        CommodoreAmiga();
        CommodoreAmiga(const Chunk &chunk);

        Modes modes() const;
    };
};

#endif // ILBMPLUGIN_H
