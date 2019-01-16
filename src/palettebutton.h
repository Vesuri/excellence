#ifndef PALETTEBUTTON_H
#define PALETTEBUTTON_H

#include <QAbstractButton>

class PaletteButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit PaletteButton(QWidget *parent = nullptr);

    unsigned paletteIndex() const;
    void setPaletteIndex(unsigned paletteIndex);
    void setColor(const QColor &color);

    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

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
