#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QFileDialog;
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
    void initialize();
    void setPaintColor(unsigned paletteIndex);
    void setEraseColor(unsigned paletteIndex);
    void openFile(const QString &path);

private:
    Ui::MainWindow *ui;
    QFileDialog *openDialog;
    BufferView *bufferView;
    Buffer *buffer;
    PenTip *penTip;
};

#endif // MAINWINDOW_H
