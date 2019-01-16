#ifndef CURRENTCOLORSBUTTON_H
#define CURRENTCOLORSBUTTON_H

#include <QAbstractButton>

class CurrentColorsButton : public QAbstractButton
{
    Q_OBJECT
public:
    explicit CurrentColorsButton(QWidget *parent = nullptr);

    void setPaintColor(const QColor &color);
    void setEraseColor(const QColor &color);

    void paintEvent(QPaintEvent *e) override;

private:
    QColor paintColor;
    QColor eraseColor;
};

#endif // CURRENTCOLORSBUTTON_H
