#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QGraphicsScene;
class PaletteItem;
class ImageWindow;
class DrawTool;
class PenTip;
template <class T> class QVector;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void setPaintColor(unsigned paletteIndex);
    void setEraseColor(unsigned paletteIndex);

private:
    Ui::MainWindow *ui;
    ImageWindow *imageWindow;
    QImage *image;
    DrawTool *drawTool;
    PenTip *penTip;
    QVector<QRgb> *palette;
};

#endif // MAINWINDOW_H
