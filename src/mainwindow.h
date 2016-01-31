#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QGraphicsScene;
class PaletteItem;
class BufferView;
class Buffer;
class PenTip;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void setPaintColor(unsigned paletteIndex);
    void setEraseColor(unsigned paletteIndex);
    void setupTools();

private:
    Ui::MainWindow *ui;
    BufferView *bufferView;
    Buffer *buffer;
    PenTip *penTip;
};

#endif // MAINWINDOW_H
