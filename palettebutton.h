#ifndef PALETTEBUTTON_H
#define PALETTEBUTTON_H

#include <QAbstractButton>

class PaletteButton : public QAbstractButton
{
public:
    PaletteButton(QWidget *parent = 0);

    void setColor(const QColor &color);

    virtual void paintEvent(QPaintEvent *e);

private:
    QColor color;
};

#endif // PALETTEBUTTON_H
