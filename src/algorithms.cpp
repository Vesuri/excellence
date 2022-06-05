#include <QImage>
#include <QPoint>
#include "algorithms.h"

void Algorithms::line(const QPoint &from, const QPoint &to, std::function<void(const QPoint &)> point)
{
    point(from);

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
                point(QPoint(x, y));
            }
        } else {
            while (y != to.y()) {
                y += yAdd;
                error += twoDx;
                if (error > dy) {
                    x += xAdd;
                    error -= twoDy;
                }
                point(QPoint(x, y));
            }
        }
    }
}

void Algorithms::rectangle(const QPoint &topLeft, const QPoint &bottomRight, std::function<void(const QPoint &)> point)
{
    QPoint topRight(bottomRight.x(), topLeft.y());
    QPoint bottomLeft(topLeft.x(), bottomRight.y());
    line(topLeft, topRight, point);
    line(topRight, bottomRight, point);
    line(bottomRight, bottomLeft, point);
    line(bottomLeft, topLeft, point);
}

void Algorithms::fillRectangle(const QPoint &topLeft, const QPoint &bottomRight, std::function<void(const QPoint &)> point)
{
    for (int y = topLeft.y(); y <= bottomRight.y(); y++) {
        QPoint left(topLeft.x(), y);
        QPoint right(bottomRight.x(), y);
        line(left, right, point);
    }
}
