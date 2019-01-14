#include <QByteArray>
#include <QImage>

#include "bitmapheader.h"

BitmapHeader::BitmapHeader(const QImage &image) : Chunk("BMHD", QByteArray(20, 0))
{
    unsigned char planes = 1;
    for (unsigned maxColorsWithPlanes = 2; maxColorsWithPlanes < static_cast<unsigned>(image.colorCount()); maxColorsWithPlanes *= 2, planes++);

    setUword(0, static_cast<unsigned short>(image.width()));
    setUword(2, static_cast<unsigned short>(image.height()));
    setUbyte(8, planes);
    setUbyte(14, 1);
    setUbyte(15, 1);
    setWord(16, static_cast<short>(image.width()));
    setWord(18, static_cast<short>(image.height()));
}

BitmapHeader::BitmapHeader(const Chunk &chunk) : Chunk(chunk)
{
}

unsigned short BitmapHeader::width() const
{
    return uword(0);
}

unsigned short BitmapHeader::height() const
{
    return uword(2);
}

short BitmapHeader::x() const
{
    return word(4);
}

short BitmapHeader::y() const
{
    return word(6);
}

unsigned char BitmapHeader::planes() const
{
    return ubyte(8);
}

BitmapHeader::Masking BitmapHeader::masking() const
{
    return static_cast<Masking>(ubyte(9));
}

BitmapHeader::Compression BitmapHeader::compression() const
{
    return static_cast<Compression>(ubyte(10));
}

unsigned short BitmapHeader::transparentColor() const
{
    return uword(12);
}

unsigned char BitmapHeader::xAspect() const
{
    return ubyte(14);
}

unsigned char BitmapHeader::yAspect() const
{
    return ubyte(15);
}

short BitmapHeader::pageWidth() const
{
    return word(16);
}

short BitmapHeader::pageHeight() const
{
    return word(18);
}
