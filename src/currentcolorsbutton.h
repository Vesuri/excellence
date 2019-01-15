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

    virtual void paintEvent(QPaintEvent *e);

private:
    QColor paintColor;
    QColor eraseColor;
};

#endif // CURRENTCOLORSBUTTON_H
