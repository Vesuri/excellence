#ifndef CURRENTCOLORSBUTTON_H
#define CURRENTCOLORSBUTTON_H

#include <QAbstractButton>

class CurrentColorsButton : public QAbstractButton
{
    Q_OBJECT
public:
    explicit CurrentColorsButton(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *e) override;

public slots:
    void setPaintColor(unsigned colorIndex, const QColor &color);
    void setEraseColor(unsigned colorIndex, const QColor &color);

private:
    QColor paintColor;
    QColor eraseColor;
};

#endif // CURRENTCOLORSBUTTON_H
