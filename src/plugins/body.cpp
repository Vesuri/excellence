#include <QByteArray>
#include <QImage>

#include "bitmapheader.h"
#include "colormap.h"
#include "commodoreamiga.h"
#include "body.h"

Body::Body(const QImage &image) : Chunk("BODY", QByteArray())
{
    // Data is padded to word boundaries
    unsigned bytesPerPlane = ((image.width() + 15) & 0xfff0) / 8;
    unsigned planesPerRow = 1;
    for (unsigned maxColorsWithPlanes = 2; maxColorsWithPlanes < static_cast<unsigned>(image.colorCount()); maxColorsWithPlanes *= 2, planesPerRow++);
    unsigned bytesPerRow = bytesPerPlane * planesPerRow;

    setSize(bytesPerRow * static_cast<unsigned>(image.height()));

    unsigned char *planarRow = new unsigned char[bytesPerPlane];
    for (unsigned y = 0, index = 0; y < static_cast<unsigned>(image.height()); y++) {
        for (unsigned plane = 0; plane < planesPerRow; plane++) {
            // Convert the chunky data to planar
            const unsigned char *chunky = image.scanLine(static_cast<int>(y));
            unsigned char *planar = planarRow;
            for (unsigned x = 0; x < static_cast<unsigned>(image.width()); x++, chunky++) {
                if ((x & 7) == 0) {
                    *planar = 0;
                }

                unsigned mask = 0x01 << plane;
                unsigned shift = 7 - (x & 7);
                *planar |= ((*chunky & mask) >> plane) << shift;
                if ((x & 7) == 7) {
                    planar++;
                }
            }

            for (unsigned x = 0; x < bytesPerPlane; x++) {
                setUbyte(index++, planarRow[x]);
            }
        }
    }
    delete[] planarRow;
}

Body::Body(const Chunk &chunk) : Chunk(chunk)
{
}

QImage Body::toImage(const BitmapHeader &bitmapHeader, const ColorMap &colorMap, const CommodoreAmiga &commodoreAmiga) const
{
    QImage image(bitmapHeader.width(), bitmapHeader.height(), QImage::Format_Indexed8);

    // Store the aspect ratio of the image
    float aspectRatio = bitmapHeader.xAspect() / static_cast<float>(bitmapHeader.yAspect());
    image.setDotsPerMeterY(static_cast<int>(image.dotsPerMeterY() * aspectRatio));

    QVector<QRgb> colorTable = colorMap.toVector();
    if (!commodoreAmiga.isNull() && (commodoreAmiga.modes() & CommodoreAmiga::ExtraHalfbrite)) {
        // Make sure Extra Halfbrite images have at least 64 colors
        while (colorTable.size() < 64) {
            colorTable.append(0);
        }

        // Halve the intensity of the latter half of the palette for Extra Halfbrite images
        for (int i = 0; i < colorTable.size() / 2; i++) {
            colorTable[colorTable.size() / 2 + i] = (colorTable[i] & 0xff000000) | ((colorTable[i] & 0x00fefefe) >> 1);
        }
    }
    image.setColorTable(colorTable);

    // Data is padded to word boundaries
    unsigned bytesPerPlane = ((bitmapHeader.width() + 15) & 0xfff0) / 8;
    unsigned planesPerRow = (bitmapHeader.planes() + (bitmapHeader.masking() == BitmapHeader::MaskingHasMask ? 1 : 0));
    unsigned bytesPerRow = bytesPerPlane * planesPerRow;

    unsigned char *planarRow = new unsigned char[bytesPerRow];
    for (unsigned y = 0, index = 0; y < static_cast<unsigned>(image.height()); y++) {
        unsigned char *planar = planarRow;
        unsigned char *planarEnd = planar + bytesPerRow;

        if (bitmapHeader.compression() == BitmapHeader::CompressionNone) {
            // No compression: copy a row of planar data as is
            while (planar < planarEnd) {
                *planar++ = ubyte(index++);
            }
        } else {
            // Runlength compression: convert encoded data until the row is full
            while (planar < planarEnd) {
                int count = byte(index++);
                if (count >= 0) {
                    // Count positive: copy the following count + 1 bytes as is
                    for (int i = 0; i < count + 1; i++) {
                        *planar++ = ubyte(index++);
                    }
                } else {
                    // Count negative: repeat the following byte (-count + 1) times
                    for (int i = 0; i < -count + 1; i++) {
                        *planar++ = ubyte(index);
                    }
                    index++;
                }
            }
        }

        // Convert the planar data to chunky
        unsigned char *chunky = image.scanLine(static_cast<int>(y));
        for (unsigned x = 0; x < static_cast<unsigned>(image.width()); x++, chunky++) {
            unsigned char mask = 0x80 >> (x & 7);
            unsigned shift = 7 - (x & 7);
            *chunky = 0;
            for (unsigned plane = 0; plane < bitmapHeader.planes(); plane++) {
                *chunky |= ((planarRow[plane * bytesPerPlane + x / 8] & mask) >> shift) << plane;
            }
        }
    }
    delete[] planarRow;

    return image;
}
