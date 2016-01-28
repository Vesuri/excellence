#ifndef PALETTEBUTTON_H
#define PALETTEBUTTON_H

#include <QAbstractButton>

class PaletteButton : public QAbstractButton
{
public:
    PaletteButton(QWidget *parent = 0);

    int paletteIndex() const;
    void setPaletteIndex(int paletteIndex);
    void setColor(const QColor &color);

    virtual void paintEvent(QPaintEvent *e);

signals:
    void selectedAsPaintColor();
    void selectedAsEraseColor();

private:
    int paletteIndex_;
    QColor color;
};

#endif // PALETTEBUTTON_H
