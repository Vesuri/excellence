#ifndef PALETTEBUTTON_H
#define PALETTEBUTTON_H

#include <QAbstractButton>

class PaletteButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit PaletteButton(QWidget *parent = 0);

    unsigned paletteIndex() const;
    void setPaletteIndex(unsigned paletteIndex);
    void setColor(const QColor &color);

    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

signals:
    void paintColorSelected(unsigned paletteIndex);
    void eraseColorSelected(unsigned paletteIndex);

private:
    unsigned paletteIndex_;
    QColor color;
    bool paintButtonDown;
    bool eraseButtonDown;
};

#endif // PALETTEBUTTON_H
