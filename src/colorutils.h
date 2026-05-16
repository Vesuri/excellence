#ifndef COLORUTILS_H
#define COLORUTILS_H

#include <QColor>
#include <Qt>

inline Qt::GlobalColor contrastColor(const QColor &color)
{
    int luma = (color.red() * 299 + color.green() * 587 + color.blue() * 114) / 1000;
    return luma > 128 ? Qt::black : Qt::white;
}

#endif // COLORUTILS_H
