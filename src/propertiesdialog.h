#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QDialog>

namespace Ui {
class PropertiesDialog;
}

class Buffer;

class PropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PropertiesDialog(QWidget *parent = nullptr);
    ~PropertiesDialog();

    void setBuffer(Buffer *buffer);

private slots:
    void setProperties();
    void setRetainImageState(int state);

signals:
    void bufferChanged(Buffer *buffer);

private:
    Ui::PropertiesDialog *ui;
    Buffer *buffer;
};

#endif // PROPERTIESDIALOG_H
