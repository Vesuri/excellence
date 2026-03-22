#ifndef CURRENTCOLORSBUTTON_H
#define CURRENTCOLORSBUTTON_H

#include <QWidget>

class CurrentColorsButton : public QWidget
{
    Q_OBJECT
public:
    explicit CurrentColorsButton(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

public slots:
    void setPaintColor(unsigned colorIndex, const QColor &color);
    void setEraseColor(unsigned colorIndex, const QColor &color);

signals:
    void foregroundClicked();
    void backgroundClicked();

private:
    QColor paintColor;
    QColor eraseColor;
};

#endif // CURRENTCOLORSBUTTON_H
