#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QFileDialog;
class PropertiesDialog;
class QGraphicsScene;
class PaletteItem;
class BufferView;
class Buffer;
class PenTip;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:
    void initialize();
    void updatePalette();
    void runPaletteActionForPaintColor(unsigned paletteIndex);
    void runPaletteActionForEraseColor(unsigned paletteIndex);
    void openFile(const QString &path = QString());
    void saveFile(const QString &path = QString());
    void saveAs();
    void newWindow();
    void closeWindow();
    void showProperties();
    void setBuffer(Buffer *buffer);
    void startCopyColor();
    void startSwapColors();

private:
    enum PaletteMode { Pick, Copy, Swap };

    void copyColor(unsigned paletteIndex);
    void swapColors(unsigned paletteIndex);
    void setPaintColor(unsigned paletteIndex);
    void setEraseColor(unsigned paletteIndex);
    void updateWindowTitle();

    Ui::MainWindow *ui;
    QFileDialog *openDialog;
    PropertiesDialog *propertiesDialog;
    QVector<BufferView *> bufferViews;
    BufferView *activeBufferView;
    Buffer *buffer;
    PenTip *penTip;
    PaletteMode paletteMode;
    unsigned paintColor;
    unsigned eraseColor;
};

#endif // MAINWINDOW_H
