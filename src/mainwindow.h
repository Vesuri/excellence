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
class DrawTool;
class LineTool;
class Tool;
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

private:
    Ui::MainWindow *ui;
    BufferView *bufferView;
    Buffer *buffer;
    DrawTool *drawTool;
    LineTool *lineTool;
    Tool *tool;
    PenTip *penTip;
};

#endif // MAINWINDOW_H
