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
    DrawTool *drawTool;
    PenTip *penTip;
};

#endif // MAINWINDOW_H
