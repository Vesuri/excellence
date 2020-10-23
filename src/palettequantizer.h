#ifndef PALETTEQUANTIZER_H
#define PALETTEQUANTIZER_H

#include <QColor>
#include <QHash>
#include <QRandomGenerator>

class PaletteQuantizer
{
public:
    PaletteQuantizer();
    void addColor(const QColor &color);
    QList<QColor> getPalette(int colorCount);
    quint8 getPaletteIndex(const QColor &color);
    int getColorCount();
    void clear();

private:
    enum Comparer { Hue, Saturation, Brightness };

    static QList<QColor> solveRootLevel(int colorCount, const QList<QColor> &colors);
    static QList<QColor> solve2ndLevel(const QList<QColor> &defaultColors, Comparer firstComparer, Comparer secondComparer, int colorCountLimit);
    static QList<QColor> solve3rdLevel(const QList<QColor> &defaultColors, Comparer secondComparer, int colorCountLimit);
    static int getNearestColor(const QColor &color, const QList<QColor> &palette);

    QList<QColor> palette;
    QHash<QColor, quint8> cache;
    QHash<QRgb, int> colorMap;
};

#endif // PALETTEQUANTIZER_H
