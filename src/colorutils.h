#ifndef COLORUTILS_H
#define COLORUTILS_H

#include <QColor>
#include <QRgb>
#include <QVector>
#include <Qt>
#include <climits>

inline int luma(QRgb color)
{
    return (qRed(color) * 299 + qGreen(color) * 587 + qBlue(color) * 114) / 1000;
}

inline Qt::GlobalColor contrastColor(const QColor &color)
{
    return luma(color.rgb()) > 128 ? Qt::black : Qt::white;
}

inline int nearestColorIndex(QRgb color, const QVector<QRgb> &palette)
{
    int bestIdx = 0, bestDist = INT_MAX;
    for (int i = 0; i < palette.size(); i++) {
        int dr = qRed(color)   - qRed(palette[i]);
        int dg = qGreen(color) - qGreen(palette[i]);
        int db = qBlue(color)  - qBlue(palette[i]);
        int dist = dr*dr + dg*dg + db*db;
        if (dist < bestDist) { bestDist = dist; bestIdx = i; }
    }
    return bestIdx;
}

#endif // COLORUTILS_H
