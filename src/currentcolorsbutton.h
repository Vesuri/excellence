#ifndef CURRENTCOLORSBUTTON_H
#define CURRENTCOLORSBUTTON_H

#include <QWidget>

class CurrentColorsButton : public QWidget
{
    Q_OBJECT
public:
    explicit CurrentColorsButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

public slots:
    void setPaintColor(unsigned colorIndex, const QColor &color);
    void setEraseColor(unsigned colorIndex, const QColor &color);

signals:
    void foregroundClicked();
    void backgroundClicked();

private:
    QColor paintColor_;
    QColor eraseColor_;
    int paintIndex_ = 0;
    int eraseIndex_ = 0;
};

#endif // CURRENTCOLORSBUTTON_H
