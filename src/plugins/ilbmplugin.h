#ifndef ILBMPLUGIN_H
#define ILBMPLUGIN_H

#include <QImageIOPlugin>

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

public:
    ILBMHandler();

    virtual bool canRead() const;
    virtual bool read(QImage *image);

private:
    class Chunk {
    public:
        Chunk(const QByteArray &chunk);

        QByteArray id() const;
        unsigned size() const;
        QByteArray data(int offset = 0, int length = -1) const;
        char byte(int offset) const;
        unsigned char ubyte(int offset) const;
        unsigned short uword(int offset) const;
        unsigned ulong(int offset) const;

    private:
        QByteArray chunk;
    };
};

#endif // ILBMPLUGIN_H
