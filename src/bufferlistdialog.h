#ifndef BUFFERLISTDIALOG_H
#define BUFFERLISTDIALOG_H

#include <QDialog>

namespace Ui {
class BufferListDialog;
}

class MainWindow;

class BufferListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BufferListDialog(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~BufferListDialog();

public slots:
    void refresh();

private:
    Ui::BufferListDialog *ui;
    MainWindow *mainWindow_;
};

#endif // BUFFERLISTDIALOG_H
