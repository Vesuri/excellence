#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <functional>

class QImage;
class QPoint;

class Algorithms
{
public:
    using PointCallback = std::function<void(const QPoint &)>;

    static void line(const QPoint &from, const QPoint &to, const PointCallback &point);
    static void rectangle(const QPoint &topLeft, const QPoint &bottomRight,
                          const PointCallback &point);
    static void fillRectangle(const QPoint &topLeft, const QPoint &bottomRight,
                              const PointCallback &point);
    static void ellipse(int cx, int cy, int rx, int ry, double angle,
                        const PointCallback &point);
    static void fillEllipse(int cx, int cy, int rx, int ry, double angle,
                            const PointCallback &point);
    static QRect floodFill(QImage &image, const QPoint &seed, int targetColor, int fillColor);
};

#endif // ALGORITHMS_H
