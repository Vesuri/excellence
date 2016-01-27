#include <QImage>
#include <QPoint>
#include "algorithms.h"

void Algorithms::drawLine(QImage &image, const QPoint &from, const QPoint &to)
{
    image.setPixel(from, 1);

    int dx = to.x() - from.x();
    int dy = to.y() - from.y();
    if (dx != 0 || dy != 0) {
        int xAdd = dx >= 0 ? 1 : -1;
        int yAdd = dy >= 0 ? 1 : -1;
        dx = dx >= 0 ? dx : -dx;
        dy = dy >= 0 ? dy : -dy;
        int twoDx = dx + dx;
        int twoDy = dy + dy;

        int x = from.x();
        int y = from.y();
        int error = 0;
        if (dy <= dx) {
            while (x != to.x()) {
                x += xAdd;
                error += twoDy;
                if (error > dx) {
                    y += yAdd;
                    error -= twoDx;
                }
                image.setPixel(x, y, 1);
            }
        } else {
            while (y != to.y()) {
                y += yAdd;
                error += twoDx;
                if (error > dy) {
                    x += xAdd;
                    error -= twoDy;
                }
                image.setPixel(x, y, 1);
            }
        }
    }
}
