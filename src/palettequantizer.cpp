#include <QtMath>
#include <QSet>
#include <algorithm>
#include "spatial_color_quant.h"
#include "palettequantizer.h"
#include "dither.h"
#include "colorutils.h"

namespace {

struct GridColor {
    int r, g, b;
};

int packKey(const GridColor &c, int levels)
{
    return (c.r * levels + c.g) * levels + c.b;
}

int roundToLevel(double value, int levels)
{
    return qBound(0, qRound(value * (levels - 1)), levels - 1);
}

// Nearest-neighbor search over the integer RGB grid, expanding outward in
// Chebyshev shells (Euclidean distance is never smaller than the Chebyshev
// distance, so once a candidate is found we only need to keep expanding
// while a closer one is still possible).
GridColor findNearestUnused(GridColor target, int levels, const QSet<int> &used)
{
    if (!used.contains(packKey(target, levels))) {
        return target;
    }

    double bestDist = -1.0;
    GridColor best = target;
    int maxRadius = 3 * (levels - 1);
    for (int radius = 1; radius <= maxRadius; radius++) {
        if (bestDist >= 0.0 && radius > bestDist) {
            break;
        }

        int rMin = qMax(0, target.r - radius), rMax = qMin(levels - 1, target.r + radius);
        int gMin = qMax(0, target.g - radius), gMax = qMin(levels - 1, target.g + radius);
        int bMin = qMax(0, target.b - radius), bMax = qMin(levels - 1, target.b + radius);

        for (int r = rMin; r <= rMax; r++) {
            for (int g = gMin; g <= gMax; g++) {
                for (int b = bMin; b <= bMax; b++) {
                    int chebyshev = qMax(qAbs(r - target.r), qMax(qAbs(g - target.g), qAbs(b - target.b)));
                    if (chebyshev != radius) {
                        continue;
                    }

                    GridColor candidate{r, g, b};
                    if (used.contains(packKey(candidate, levels))) {
                        continue;
                    }

                    double dr = r - target.r, dg = g - target.g, db = b - target.b;
                    double dist = qSqrt(dr * dr + dg * dg + db * db);
                    if (bestDist < 0.0 || dist < bestDist) {
                        bestDist = dist;
                        best = candidate;
                    }
                }
            }
        }
    }
    return best;
}

}

QImage PaletteQuantizer::quantize(const QImage &source, int num_colors, DitherMode mode, int outOf, PaletteSortMode sortMode)
{
    int levelsPerChannel = qMax(2, qRound(qPow((double)outOf, 1.0 / 3.0)));
    qint64 maxColors = qint64(levelsPerChannel) * levelsPerChannel * levelsPerChannel;
    if (num_colors > maxColors) {
        num_colors = static_cast<int>(maxColors);
    }

    int width = source.width();
    int height = source.height();
    array2d< vector_fixed<double, 3> > image(width, height);
    array2d< vector_fixed<double, 3> > filter1_weights(1, 1);
    array2d< vector_fixed<double, 3> > filter3_weights(3, 3);
    array2d< vector_fixed<double, 3> > filter5_weights(5, 5);
    array2d< int > quantized_image(width, height);
    vector< vector_fixed<double, 3> > palette;

    for (int k = 0; k < 3; k++) {
        filter1_weights(0, 0)(k) = 1.0;
    }

    for (int i = 0; i < num_colors; i++) {
        vector_fixed<double, 3> v;
        v(0) = ((double)rand()) / RAND_MAX;
        v(1) = ((double)rand()) / RAND_MAX;
        v(2) = ((double)rand()) / RAND_MAX;
        palette.push_back(v);
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QColor color = source.pixelColor(x, y);
            image(x, y)(0) = color.redF();
            image(x, y)(1) = color.greenF();
            image(x, y)(2) = color.blueF();
        }
    }

    array3d<double>* coarse_variables;
    double dithering_level = 0.09*log((double)image.get_width()*image.get_height()) - 0.04*log((double)palette.size()) + 0.001;
    int filter_size = 3;

    double stddev = dithering_level;
    double sum = 0.0;
    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            for(int k=0; k<3; k++) {
            sum += filter3_weights(i,j)(k) =
                exp(-sqrt((double)((i-1)*(i-1) + (j-1)*(j-1)))/(stddev*stddev));
            }
        }
    }
    sum /= 3;
    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            for(int k=0; k<3; k++) {
                filter3_weights(i,j)(k) /= sum;
            }
        }
    }
    sum = 0.0;
    for(int i=0; i<5; i++) {
        for(int j=0; j<5; j++) {
            for(int k=0; k<3; k++) {
                sum += filter5_weights(i,j)(k) =
                    exp(-sqrt((double)((i-2)*(i-2) + (j-2)*(j-2)))/(stddev*stddev));
            }
        }
    }
    sum /= 3;
    for(int i=0; i<5; i++) {
        for(int j=0; j<5; j++) {
            for(int k=0; k<3; k++) {
                filter5_weights(i,j)(k) /= sum;
            }
        }
    }

    array2d< vector_fixed<double, 3> >* filters[] =
        {NULL, &filter1_weights, NULL, &filter3_weights,
         NULL, &filter5_weights};
    spatial_color_quant(image, *filters[filter_size], quantized_image, palette, coarse_variables, 1.0, 0.001, 3, 1);
    //spatial_color_quant(image, filter3_weights, quantized_image, palette, coarse_variables, 0.05, 0.02);

    QSet<int> usedGridColors;
    QVector<GridColor> gridPalette(num_colors);
    for (int i = 0; i < num_colors; i++) {
        GridColor grid{
            roundToLevel(palette[i](0), levelsPerChannel),
            roundToLevel(palette[i](1), levelsPerChannel),
            roundToLevel(palette[i](2), levelsPerChannel)
        };
        grid = findNearestUnused(grid, levelsPerChannel, usedGridColors);
        usedGridColors.insert(packKey(grid, levelsPerChannel));
        gridPalette[i] = grid;
    }

    QVector<QRgb> paletteRgb(num_colors);
    for (int i = 0; i < num_colors; i++) {
        int r = qRound(gridPalette[i].r * 255.0 / (levelsPerChannel - 1));
        int g = qRound(gridPalette[i].g * 255.0 / (levelsPerChannel - 1));
        int b = qRound(gridPalette[i].b * 255.0 / (levelsPerChannel - 1));
        paletteRgb[i] = qRgba(r, g, b, 255);
    }

    if (sortMode != PaletteSortMode::None) {
        bool darkToLight = sortMode == PaletteSortMode::DarkToLight;
        std::sort(paletteRgb.begin(), paletteRgb.end(), [darkToLight](QRgb a, QRgb b) {
            return darkToLight ? luma(a) < luma(b) : luma(a) > luma(b);
        });
    }

    QImage out = ditherToPalette(source.convertToFormat(QImage::Format_RGB32), paletteRgb, mode);
    out.setDotsPerMeterX(source.dotsPerMeterX());
    out.setDotsPerMeterY(source.dotsPerMeterY());

    return out;
}
