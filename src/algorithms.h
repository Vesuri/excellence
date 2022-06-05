#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <functional>

class QImage;
class QPoint;

class Algorithms
{
public:
    static void line(const QPoint &from, const QPoint &to, std::function<void(const QPoint &)> point);
    static void rectangle(const QPoint &topLeft, const QPoint &bottomRight, std::function<void(const QPoint &)> point);
    static void fillRectangle(const QPoint &topLeft, const QPoint &bottomRight, std::function<void(const QPoint &)> point);
};

#endif // ALGORITHMS_H
