#include "spatial_color_quant.h"
#include "palettequantizer.h"

QImage PaletteQuantizer::quantize(const QImage &source, int num_colors)
{
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

    QImage out(width, height, QImage::Format_Indexed8);
    out.setDotsPerMeterX(source.dotsPerMeterX());
    out.setDotsPerMeterY(source.dotsPerMeterY());
    for (int i = 0; i < num_colors; i++) {
        QColor color;
        color.setRedF(palette[i](0));
        color.setGreenF(palette[i](1));
        color.setBlueF(palette[i](2));
        color.setAlphaF(1.0);
        out.setColor(i, color.rgba());
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            out.setPixel(x, y, quantized_image(x,y));
        }
    }

    return out;
}
