#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class ImageWindow;
class DrawTool;
class PenTip;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    ImageWindow *imageWindow;
    QImage *image;
    DrawTool *drawTool;
    PenTip *penTip;
    QVector<QRgb> palette;
};

#endif // MAINWINDOW_H
