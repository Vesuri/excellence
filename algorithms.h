#ifndef ALGORITHMS_H
#define ALGORITHMS_H

class QImage;
class QPoint;

class Algorithms
{
public:
    static void drawLine(QImage &image, const QPoint &from, const QPoint &to);
};

#endif // ALGORITHMS_H
