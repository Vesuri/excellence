#include <cmath>
#include <limits>
#include <QColor>
#include "dither.h"
#include "colorutils.h"

namespace {

struct Oklab {
    double L, a, b;
};

double srgbToLinear(int c)
{
    double v = c / 255.0;
    return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
}

int linearToSrgb8(double v)
{
    v = qBound(0.0, v, 1.0);
    double s = v <= 0.0031308 ? v * 12.92 : 1.055 * std::pow(v, 1.0 / 2.4) - 0.055;
    return qBound(0, qRound(s * 255.0), 255);
}

Oklab rgbToOklab(QRgb c)
{
    double r = srgbToLinear(qRed(c));
    double g = srgbToLinear(qGreen(c));
    double b = srgbToLinear(qBlue(c));

    double l = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b;
    double m = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b;
    double s = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b;

    double l_ = std::cbrt(l);
    double m_ = std::cbrt(m);
    double s_ = std::cbrt(s);

    return {
        0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
        1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
        0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_
    };
}

double oklabDistance(const Oklab &x, const Oklab &y)
{
    double dl = x.L - y.L;
    double da = x.a - y.a;
    double db = x.b - y.b;
    return std::sqrt(dl * dl + da * da + db * db);
}

// Linear-light average of two sRGB colors, re-encoded back to sRGB. This is
// what a 50% checkerboard of the two colors actually emits, as opposed to
// the (much darker-looking) sRGB average.
QRgb linearLightAverage(QRgb a, QRgb b)
{
    double r = (srgbToLinear(qRed(a))   + srgbToLinear(qRed(b)))   / 2.0;
    double g = (srgbToLinear(qGreen(a)) + srgbToLinear(qGreen(b))) / 2.0;
    double bl = (srgbToLinear(qBlue(a)) + srgbToLinear(qBlue(b)))  / 2.0;
    return qRgb(linearToSrgb8(r), linearToSrgb8(g), linearToSrgb8(bl));
}

struct VirtualPaletteEntry {
    Oklab color;
    int indexA;
    int indexB;
};

const double kPatternGate = 0.12;

QVector<VirtualPaletteEntry> buildVirtualPalette(const QVector<QRgb> &palette)
{
    QVector<Oklab> labs;
    labs.reserve(palette.size());
    for (QRgb c : palette)
        labs.append(rgbToOklab(c));

    QVector<VirtualPaletteEntry> entries;
    entries.reserve(palette.size());
    for (int i = 0; i < palette.size(); i++)
        entries.append({labs[i], i, i});

    for (int i = 0; i < palette.size(); i++) {
        for (int j = i + 1; j < palette.size(); j++) {
            if (oklabDistance(labs[i], labs[j]) <= kPatternGate) {
                Oklab mix = rgbToOklab(linearLightAverage(palette[i], palette[j]));
                entries.append({mix, i, j});
            }
        }
    }
    return entries;
}

QImage noDither(const QImage &source, const QVector<QRgb> &palette)
{
    QImage indexed(source.size(), QImage::Format_Indexed8);
    indexed.setColorTable(palette);
    for (int y = 0; y < source.height(); y++)
        for (int x = 0; x < source.width(); x++)
            indexed.setPixel(x, y, nearestColorIndex(source.pixel(x, y), palette));
    return indexed;
}

QImage floydSteinbergDither(const QImage &source, const QVector<QRgb> &palette)
{
    const int w = source.width();
    const int h = source.height();

    QImage indexed(source.size(), QImage::Format_Indexed8);
    indexed.setColorTable(palette);

    struct Error { double r = 0, g = 0, b = 0; };
    QVector<Error> error(w * h);

    auto addError = [&](int x, int y, double r, double g, double b) {
        if (x < 0 || x >= w || y < 0 || y >= h)
            return;
        Error &e = error[y * w + x];
        e.r += r; e.g += g; e.b += b;
    };

    for (int y = 0; y < h; y++) {
        bool reverse = (y & 1) != 0;
        int dir = reverse ? -1 : 1;
        for (int i = 0; i < w; i++) {
            int x = reverse ? (w - 1 - i) : i;
            QRgb src = source.pixel(x, y);
            const Error &e = error[y * w + x];
            double r = qBound(0.0, qRed(src)   + e.r, 255.0);
            double g = qBound(0.0, qGreen(src) + e.g, 255.0);
            double b = qBound(0.0, qBlue(src)  + e.b, 255.0);

            int idx = nearestColorIndex(qRgb(qRound(r), qRound(g), qRound(b)), palette);
            indexed.setPixel(x, y, idx);

            QRgb pal = palette[idx];
            double er = r - qRed(pal);
            double eg = g - qGreen(pal);
            double eb = b - qBlue(pal);

            addError(x + dir, y,     er * 7.0 / 16.0, eg * 7.0 / 16.0, eb * 7.0 / 16.0);
            addError(x - dir, y + 1, er * 3.0 / 16.0, eg * 3.0 / 16.0, eb * 3.0 / 16.0);
            addError(x,       y + 1, er * 5.0 / 16.0, eg * 5.0 / 16.0, eb * 5.0 / 16.0);
            addError(x + dir, y + 1, er * 1.0 / 16.0, eg * 1.0 / 16.0, eb * 1.0 / 16.0);
        }
    }

    return indexed;
}

QImage patternDither(const QImage &source, const QVector<QRgb> &palette)
{
    QVector<VirtualPaletteEntry> virtualPalette = buildVirtualPalette(palette);

    QImage indexed(source.size(), QImage::Format_Indexed8);
    indexed.setColorTable(palette);

    for (int y = 0; y < source.height(); y++) {
        for (int x = 0; x < source.width(); x++) {
            Oklab pixel = rgbToOklab(source.pixel(x, y));

            int best = 0;
            double bestDist = std::numeric_limits<double>::max();
            for (int k = 0; k < virtualPalette.size(); k++) {
                double d = oklabDistance(pixel, virtualPalette[k].color);
                if (d < bestDist) { bestDist = d; best = k; }
            }

            const VirtualPaletteEntry &entry = virtualPalette[best];
            int chosen = entry.indexA == entry.indexB
                ? entry.indexA
                : (((x + y) & 1) ? entry.indexA : entry.indexB);
            indexed.setPixel(x, y, chosen);
        }
    }

    return indexed;
}

} // namespace

QImage ditherToPalette(const QImage &source, const QVector<QRgb> &palette, DitherMode mode)
{
    switch (mode) {
    case DitherMode::FloydSteinberg:
        return floydSteinbergDither(source, palette);
    case DitherMode::Pattern:
        return patternDither(source, palette);
    case DitherMode::None:
    default:
        return noDither(source, palette);
    }
}
