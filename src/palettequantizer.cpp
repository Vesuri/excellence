#include "palettequantizer.h"

// hash function for QColor for use in QSet / QHash
QT_BEGIN_NAMESPACE
uint qHash(const QColor &c)
{
    return qHash(c.rgba());
}
QT_END_NAMESPACE

PaletteQuantizer::PaletteQuantizer()
{
}

void PaletteQuantizer::addColor(const QColor &color)
{
    QRgb argb = color.rgba();

    if (colorMap.contains(argb)) {
        colorMap[argb]++;
    } else {
        colorMap[argb] = 1;
    }
}

QList<QColor> PaletteQuantizer::getPalette(int colorCount)
{
    palette.clear();

    QList<QColor> colors;
    for (const QRgb color : colorMap.keys()) {
        colors.append(QColor(color));
    }
    qWarning("XX COLORMAP HAS %d", colors.count());

    // if there're less colors in the image then allowed, simply pass them all
    if (colorMap.count() > colorCount) {
        qWarning("XX WHICH IS TOO MANY");
        // solves the color quantization
        colors = solveRootLevel(colorCount, colors);

        // if there're still too much colors, just snap them from the top))
        if (colors.count() > colorCount) {
            QList<int> counts;
            for (int i = 0; i < colors.count(); i++) {
                counts.append(colorMap[colors[i].rgba()]);
            }

            for (int i = 0; i < colors.count(); i++) {
                qWarning("XX COLOR %d %x %d", i, colors[i].rgba(), counts[i]);
            }

            QList<QColor> result;
            int maxValue = INT_MAX;
            for (int i = 0; i < colorCount; i++) {
                int currentMax = 0;
                int maxIndex = 0;
                for (int j = 0; j < counts.count(); j++) {
                    if (counts[j] > currentMax && counts[j] < maxValue) {
                        currentMax = counts[j];
                        maxIndex = j;
                    }
                }

                result.append(colors[maxIndex]);
                maxValue = currentMax;
            }

            colors = result;
        }
    }

    // clears the hit cache
    cache.clear();

    // adds the selected colors to a final palette
    palette = colors;

    // returns our new palette
    return palette;
}

quint8 PaletteQuantizer::getPaletteIndex(const QColor &color)
{
    quint8 result;

    // checks whether color was already requested, in that case returns an index from a cache
    if (cache.contains(color)) {
        result = cache[color];
    } else {
        // otherwise finds the nearest color
        result = (quint8)getNearestColor(color, palette);
        cache[color] = result;
    }

    // returns a palette index
    return result;
}

int PaletteQuantizer::getColorCount()
{
    return colorMap.count();
}

void PaletteQuantizer::clear()
{
    // clears all the information
    cache.clear();
    colorMap.clear();
}

QList<QColor> PaletteQuantizer::solveRootLevel(int colorCount, const QList<QColor> &colors)
{
    QList<QColor> result = colors;

    // selects three palettes: 1) hue is unique, 2) saturation is unique, 3) brightness is unique
    QHash<qreal, QColor> hueColors;
    QHash<qreal, QColor> saturationColors;
    QHash<qreal, QColor> brightnessColors;
    for (const QColor &color : colors) {
        hueColors.insert(color.hsvHueF(), color);
        saturationColors.insert(color.hsvSaturationF(), color);
        brightnessColors.insert(color.valueF(), color);
    }

    // retrieves number of colors, ie. how many we've eliminated
    int hueColorCount = hueColors.count();
    int saturationColorCount = saturationColors.count();
    int brightnessColorCount = brightnessColors.count();
    qWarning("XX HUE COLORS %d", hueColorCount);
    qWarning("XX SATURATION COLORS %d", saturationColorCount);
    qWarning("XX BRIGHTNESS COLORS %d", brightnessColorCount);

    // selects the palette (from those 3) which has the most colors, because an image has some details in that category
    if (hueColorCount > saturationColorCount && hueColorCount > brightnessColorCount) {
        qWarning("XX HUE COLORS");
        result = solve2ndLevel(hueColors.values(), Saturation, Brightness, colorCount);
    } else if (saturationColorCount > hueColorCount && saturationColorCount > brightnessColorCount) {
        qWarning("XX SATURATION COLORS");
        result = solve2ndLevel(saturationColors.values(), Hue, Brightness, colorCount);
    } else {
        qWarning("XX CHOOSING BRIGHTNESS COLORS");
        result = solve2ndLevel(brightnessColors.values(), Hue, Saturation, colorCount);
    }

    return result;
}

QList<QColor> PaletteQuantizer::solve2ndLevel(const QList<QColor> &defaultColors, Comparer firstComparer, Comparer secondComparer, int colorCountLimit)
{
    QList<QColor> result = defaultColors;

    if (result.count() > colorCountLimit) {
        QHash<qreal, QColor> firstColors;
        QHash<qreal, QColor> secondColors;
        for (const QColor &color : result) {
            qWarning("XX COLOR: %x", color.rgba());
            switch (firstComparer) {
            case Hue:
                firstColors.insert(color.hsvHueF(), color);
                break;
            case Saturation:
                firstColors.insert(color.hsvSaturationF(), color);
                break;
            case Brightness:
                firstColors.insert(color.valueF(), color);
                break;
            default:
                break;
            }

            switch (secondComparer) {
            case Hue:
                secondColors.insert(color.hsvHueF(), color);
                break;
            case Saturation:
                secondColors.insert(color.hsvSaturationF(), color);
                break;
            case Brightness:
                secondColors.insert(color.valueF(), color);
                break;
            default:
                break;
            }
        }

        qWarning("XX FIRST COLORS %d", firstColors.count());
        qWarning("XX SECOND COLORS %d", secondColors.count());
        if (firstColors.count() > secondColors.count()) {
            qWarning("XX CHOOSING FIRST COLORS");
            result = solve3rdLevel(firstColors.values(), secondComparer, colorCountLimit);
        } else {
            qWarning("XX CHOOSING SECOND COLORS");
            result = solve3rdLevel(secondColors.values(), firstComparer, colorCountLimit);
        }
    }

    return result;
}

QList<QColor> PaletteQuantizer::solve3rdLevel(const QList<QColor> &defaultColors, Comparer secondComparer, int colorCountLimit)
{
    QList<QColor> result = defaultColors;

    if (result.count() > colorCountLimit) {
        qWarning("XX STILL TOO MANY %d", result.count());
        QHash<qreal, QColor> colors;
        for (const QColor &color : result) {
            switch (secondComparer) {
            case Hue:
                colors.insert(color.hsvHueF(), color);
                break;
            case Saturation:
                colors.insert(color.hsvSaturationF(), color);
                break;
            case Brightness:
                colors.insert(color.valueF(), color);
                break;
            default:
                break;
            }
        }

        qWarning("XX %d UNIQUE", colors.count());
        if (colors.count() >= colorCountLimit) {
            result = colors.values();
        }
    }

    return result;
}

int PaletteQuantizer::getNearestColor(const QColor &color, const QList<QColor> &palette)
{
    int bestIndex = 0;
    int bestFactor = INT_MAX;

    for (int index = 0; index < palette.count(); index++) {
        QColor targetColor = palette[index];

        int deltaA = color.alpha() - targetColor.alpha();
        int deltaR = color.red() - targetColor.red();
        int deltaG = color.green() - targetColor.green();
        int deltaB = color.blue() - targetColor.blue();

        int factorA = deltaA * deltaA;
        int factorR = deltaR * deltaR;
        int factorG = deltaG * deltaG;
        int factorB = deltaB * deltaB;

        int factor = factorA + factorR + factorG + factorB;

        if (factor == 0) return index;

        if (factor < bestFactor) {
            bestFactor = factor;
            bestIndex = index;
        }
    }

    return bestIndex;
}
