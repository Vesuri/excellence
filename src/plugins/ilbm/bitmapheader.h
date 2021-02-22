#ifndef BITMAPHEADER_H
#define BITMAPHEADER_H

#include "chunk.h"

class QImage;

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

    BitmapHeader(const QImage &image, const Compression compression = CompressionNone);
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

#endif // BITMAPHEADER_H
