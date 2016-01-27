#include <QImage>
#include <QPoint>
#include "algorithms.h"

void Algorithms::drawLine(QImage &image, const QPoint &from, const QPoint &to)
{
    int CurrentX, CurrentY;
    int Xinc, Yinc;
    int Dx, Dy;
    int TwoDx, TwoDy;
    int TwoDxAccumulatedError;
    int TwoDyAccumulatedError;

    Dx = to.x() - from.x();
    Dy = to.y() - from.y();
    TwoDx = Dx + Dx;
    TwoDy = Dy + Dy;
    CurrentX = from.x();
    CurrentY = from.y();
    Xinc = 1;
    Yinc = 1;

    if (Dx < 0) {
        Xinc = -1;
        Dx = -Dx;
        TwoDx = -TwoDx;
    }
    if (Dy < 0) {
        Yinc = -1;
        Dy = -Dy;
        TwoDy = -TwoDy;
    }
    image.setPixel(from, 1);

    if (Dx != 0 || Dy != 0) {
        if (Dy <= Dx) {
            TwoDxAccumulatedError = 0;
            while (CurrentX != to.x()) {
                CurrentX += Xinc;
                TwoDxAccumulatedError += TwoDy;
                if (TwoDxAccumulatedError > Dx) {
                    CurrentY += Yinc;
                    TwoDxAccumulatedError -= TwoDx;
                }
                image.setPixel(CurrentX, CurrentY, 1);
            }
        } else {
            TwoDyAccumulatedError = 0;
            while (CurrentY != to.y()) {
                CurrentY += Yinc;
                TwoDyAccumulatedError += TwoDx;
                if (TwoDyAccumulatedError > Dy) {
                    CurrentX += Xinc;
                    TwoDyAccumulatedError -= TwoDy;
                }
                image.setPixel(CurrentX, CurrentY, 1);
            }
        }
    }
}
