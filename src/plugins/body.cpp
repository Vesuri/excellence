#include <QByteArray>
#include <QImage>

#include "colormap.h"
#include "commodoreamiga.h"
#include "body.h"

Body::Body(const QImage &image, const BitmapHeader::Compression compression) : Chunk("BODY", QByteArray())
{
    // Data is padded to word boundaries
    unsigned bytesPerPlane = ((image.width() + 15) & 0xfff0) / 8;
    unsigned planesPerRow = 1;
    for (unsigned maxColorsWithPlanes = 2; maxColorsWithPlanes < static_cast<unsigned>(image.colorCount()); maxColorsWithPlanes *= 2, planesPerRow++);
    unsigned bytesPerRow = bytesPerPlane * planesPerRow;

    QByteArray data;

    unsigned char *planarRow = new unsigned char[bytesPerRow];
    for (unsigned y = 0; y < static_cast<unsigned>(image.height()); y++) {
        unsigned char *planar = planarRow;
        for (unsigned plane = 0; plane < planesPerRow; plane++) {
            // Convert the chunky data to planar
            const unsigned char *chunky = image.scanLine(static_cast<int>(y));
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
        }

        if (compression == BitmapHeader::CompressionNone) {
            data.append(reinterpret_cast<char *>(planarRow), static_cast<int>(bytesPerRow));
        } else if (compression == BitmapHeader::CompressionByteRun1) {
            enum RunLengthMode {
                RunLengthModeNotSet = 0,
                RunLengthModeCopy = 1,
                RunLengthModeRepeat = 2
            };

            RunLengthMode runLengthMode = RunLengthModeNotSet;
            unsigned runLengthStart = 0;
            for (unsigned x = 1, sameCount = 0; x < bytesPerRow; x++) {
                unsigned runLength = x - runLengthStart;
                sameCount = planarRow[x] == planarRow[x - 1] ? (sameCount + 1) : 0;

                switch (runLengthMode) {
                case RunLengthModeCopy:
                    // 3 or more similar bytes or maximum copy block size of 128
                    if (sameCount >= 2 || runLength == 128) {
                        // Only write copy block if it exists
                        if (runLength - sameCount != 0) {
                            // Copy count (positive): Bytes to be copied - 1
                            data.append(static_cast<char>(runLength - sameCount - 1));

                            // The bytes as they are
                            for (unsigned i = runLengthStart; i < x - sameCount; i++) {
                                data.append(static_cast<char>(planarRow[i]));
                            }

                            // Next block starts from the end of the copied bytes
                            runLengthStart = x - sameCount;
                        }

                        if (sameCount >= 2) {
                            // 3 or more similar bytes: switch to repeat mode
                            runLengthMode = RunLengthModeRepeat;
                        } else {
                            // Reset same byte count
                            sameCount = 0;
                        }
                    }
                    break;
                case RunLengthModeRepeat:
                    // A different byte or maximum repeat block size of 128
                    if (sameCount == 0 || runLength == 128) {
                        // Repeat count (negative): -(Bytes to be copied - 1)
                        data.append(-static_cast<char>(runLength - 1));

                        // The byte to be repeated
                        data.append(static_cast<char>(planarRow[runLengthStart]));

                        // Next block starts from the end of the repeated bytes
                        runLengthStart = x;

                        if (sameCount == 0) {
                            // A different byte: switch to copy mode
                            runLengthMode = RunLengthModeCopy;
                        } else {
                            // Reset same byte count
                            sameCount = 0;
                        }
                    }
                    break;
                default:
                    // Choose a mode if not yet set
                    runLengthMode = planarRow[x] == planarRow[runLengthStart] ? RunLengthModeRepeat : RunLengthModeCopy;
                    break;
                }
            }

            // Write any remaining bytes
            if (runLengthMode != RunLengthModeNotSet) {
                unsigned runLength = bytesPerRow - runLengthStart;

                if (runLengthMode == RunLengthModeCopy) {
                    data.append(static_cast<char>(runLength) - 1);
                    while (runLengthStart < bytesPerRow) {
                        data.append(static_cast<char>(planarRow[runLengthStart++]));
                    }
                } else {
                    data.append(-static_cast<char>(runLength - 1));
                    data.append(static_cast<char>(planarRow[runLengthStart]));
                }
            }
        }
    }
    delete[] planarRow;

    setData(data);
}

Body::Body(const Chunk &chunk) : Chunk(chunk)
{
}

QImage Body::toImage(const BitmapHeader &bitmapHeader, const ColorMap &colorMap, const CommodoreAmiga &commodoreAmiga) const
{
    QImage image(bitmapHeader.width(), bitmapHeader.height(), QImage::Format_Indexed8);

    // Calculate the aspect ratio of the image
    unsigned char xAspect = bitmapHeader.xAspect();
    unsigned char yAspect = bitmapHeader.yAspect();
    if (xAspect == 1 && yAspect == 1) {
        // Old DPaints claimed all images to be in 1:1 ratio - check if CAMG has more information
        if (commodoreAmiga.modes() & CommodoreAmiga::Hires) {
            yAspect = 2;
        }
        if (commodoreAmiga.modes() & CommodoreAmiga::Lace) {
            xAspect = 2;
        }
    }
    float aspectRatio = xAspect / static_cast<float>(yAspect);
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
                    for (int i = 0; i < count + 1 && planar < planarEnd; i++) {
                        *planar++ = ubyte(index++);
                    }
                } else {
                    // Count negative: repeat the following byte (-count + 1) times
                    for (int i = 0; i < -count + 1 && planar < planarEnd; i++) {
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
