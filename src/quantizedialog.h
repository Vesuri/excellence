#ifndef QUANTIZEDIALOG_H
#define QUANTIZEDIALOG_H

#include <QDialog>
#include <QVector>
#include <QRgb>

namespace Ui {
class QuantizeDialog;
}

class QuantizeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuantizeDialog(QWidget *parent = nullptr);
    ~QuantizeDialog() override;

    void setCurrentPalette(const QVector<QRgb> &palette);

    bool useOptimalPalette() const;
    int colors() const;
    int outOf() const;

private slots:
    void setPaletteMode(int index);

private:
    Ui::QuantizeDialog *ui;
    int currentColorsIndex_ = 0;
    int currentOutOfIndex_ = 7;
};

#endif // QUANTIZEDIALOG_H
