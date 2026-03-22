#include <QImage>
#include <QPoint>
#include <QRect>
#include <QStack>
#include <cmath>
#include "algorithms.h"

static const double ALG_PI = 3.14159265358979323846;

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

void Algorithms::ellipse(int cx, int cy, int rx, int ry, double angle,
                          std::function<void(const QPoint &)> fn)
{
    if (rx < 0) rx = -rx;
    if (ry < 0) ry = -ry;
    if (rx == 0 && ry == 0) { fn(QPoint(cx, cy)); return; }
    if (rx == 0) { line(QPoint(cx, cy - ry), QPoint(cx, cy + ry), fn); return; }
    if (ry == 0) { line(QPoint(cx - rx, cy), QPoint(cx + rx, cy), fn); return; }

    int steps = qMax(4, 4 * (rx + ry));
    double cosA = std::cos(angle), sinA = std::sin(angle);
    auto pt = [&](double t) {
        double ex = rx * std::cos(t);
        double ey = ry * std::sin(t);
        return QPoint(cx + qRound(ex * cosA - ey * sinA),
                      cy + qRound(ex * sinA + ey * cosA));
    };
    QPoint prev = pt(0.0);
    for (int i = 1; i <= steps; i++) {
        QPoint cur = pt(2.0 * ALG_PI * i / steps);
        line(prev, cur, fn);
        prev = cur;
    }
}

QRect Algorithms::floodFill(QImage &image, const QPoint &seed, int targetColor, int fillColor)
{
    if (!image.rect().contains(seed))
        return QRect();
    if (targetColor == fillColor)
        return QRect();

    QStack<QPoint> stack;
    stack.push(seed);
    QRect changedRect;

    while (!stack.isEmpty()) {
        QPoint p = stack.pop();
        if (!image.rect().contains(p) || image.pixelIndex(p) != targetColor)
            continue;

        int x1 = p.x();
        while (x1 > 0 && image.pixelIndex(x1 - 1, p.y()) == targetColor) x1--;
        int x2 = p.x();
        while (x2 < image.width() - 1 && image.pixelIndex(x2 + 1, p.y()) == targetColor) x2++;

        for (int x = x1; x <= x2; x++)
            image.setPixel(x, p.y(), static_cast<uint>(fillColor));
        changedRect = changedRect.united(QRect(x1, p.y(), x2 - x1 + 1, 1));

        bool prevAbove = false, prevBelow = false;
        for (int x = x1; x <= x2; x++) {
            if (p.y() > 0) {
                bool above = image.pixelIndex(x, p.y() - 1) == targetColor;
                if (above && !prevAbove) stack.push(QPoint(x, p.y() - 1));
                prevAbove = above;
            }
            if (p.y() < image.height() - 1) {
                bool below = image.pixelIndex(x, p.y() + 1) == targetColor;
                if (below && !prevBelow) stack.push(QPoint(x, p.y() + 1));
                prevBelow = below;
            }
        }
    }
    return changedRect;
}

void Algorithms::fillEllipse(int cx, int cy, int rx, int ry, double angle,
                               std::function<void(const QPoint &)> fn)
{
    if (rx < 0) rx = -rx;
    if (ry < 0) ry = -ry;
    if (rx == 0 && ry == 0) { fn(QPoint(cx, cy)); return; }
    if (rx == 0) { line(QPoint(cx, cy - ry), QPoint(cx, cy + ry), fn); return; }
    if (ry == 0) { line(QPoint(cx - rx, cy), QPoint(cx + rx, cy), fn); return; }

    double cosA = std::cos(angle), sinA = std::sin(angle);
    double rx2 = (double)rx * rx, ry2 = (double)ry * ry;
    int yBound = (int)std::ceil(std::sqrt(rx2 * sinA * sinA + ry2 * cosA * cosA)) + 1;

    for (int dy = -yBound; dy <= yBound; dy++) {
        // Solve (dx*cosA + dy*sinA)^2/rx^2 + (-dx*sinA + dy*cosA)^2/ry^2 = 1 for dx
        double A = cosA * cosA / rx2 + sinA * sinA / ry2;
        double B = 2.0 * dy * cosA * sinA * (1.0 / rx2 - 1.0 / ry2);
        double C = (double)dy * dy * (sinA * sinA / rx2 + cosA * cosA / ry2) - 1.0;
        double disc = B * B - 4.0 * A * C;
        if (disc < 0) continue;
        double sqrtD = std::sqrt(disc);
        int x1 = cx + qRound((-B - sqrtD) / (2.0 * A));
        int x2 = cx + qRound((-B + sqrtD) / (2.0 * A));
        if (x1 > x2) qSwap(x1, x2);
        line(QPoint(x1, cy + dy), QPoint(x2, cy + dy), fn);
    }
}
