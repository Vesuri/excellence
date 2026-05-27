#ifndef PALETTEBUTTON_H
#define PALETTEBUTTON_H

#include <QAbstractButton>
#include <QPoint>

class PaletteButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit PaletteButton(QWidget *parent = nullptr);

    unsigned paletteIndex() const;
    void setPaletteIndex(unsigned paletteIndex);
    void setColor(const QColor &color);
    void setIsPaintColor(bool isPaintColor);
    void setIsEraseColor(bool isEraseColor);
    QSize minimumSizeHint() const override { return sizeHint(); }

signals:
    void paintColorSelected(unsigned paletteIndex);
    void eraseColorSelected(unsigned paletteIndex);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    void setColorFlag(bool &member, bool value);

    unsigned paletteIndex_;
    QColor color_;
    bool paintButtonDown;
    bool eraseButtonDown;
    bool isPaintColor_ = false;
    bool isEraseColor_ = false;
    QPoint dragStartPos_;
};

#endif // PALETTEBUTTON_H
