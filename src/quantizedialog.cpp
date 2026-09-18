#include <QtMath>
#include "quantizedialog.h"
#include "ui_quantizedialog.h"

namespace {

bool componentFitsPrecision(int component, int levels)
{
    int level = qRound(component * (levels - 1) / 255.0);
    return qRound(level * 255.0 / (levels - 1)) == component;
}

int palettePrecisionIndex(const QVector<QRgb> &palette)
{
    for (int index = 0; index < 8; index++) {
        int levels = 1 << (index + 1);
        bool fits = true;
        for (QRgb color : palette) {
            if (!componentFitsPrecision(qRed(color), levels)
                    || !componentFitsPrecision(qGreen(color), levels)
                    || !componentFitsPrecision(qBlue(color), levels)) {
                fits = false;
                break;
            }
        }
        if (fits)
            return index;
    }
    return 7;
}

}

QuantizeDialog::QuantizeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuantizeDialog)
{
    ui->setupUi(this);

    connect(ui->comboBoxPalette, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QuantizeDialog::setPaletteMode);

    ui->comboBoxPalette->setCurrentIndex(1);
    setPaletteMode(1);
}

QuantizeDialog::~QuantizeDialog()
{
    delete ui;
}

void QuantizeDialog::setCurrentPalette(const QVector<QRgb> &palette)
{
    int count = qBound(2, palette.count(), 256);
    int colors = 2;
    currentColorsIndex_ = 0;
    while (colors < count && currentColorsIndex_ < 7) {
        colors <<= 1;
        currentColorsIndex_++;
    }
    currentOutOfIndex_ = palettePrecisionIndex(palette);

    ui->comboBoxColors->setCurrentIndex(currentColorsIndex_);
    ui->comboBoxOutOf->setCurrentIndex(currentOutOfIndex_);
}

bool QuantizeDialog::useOptimalPalette() const
{
    return ui->comboBoxPalette->currentIndex() == 1;
}

int QuantizeDialog::colors() const
{
    return qRound(qPow(2, ui->comboBoxColors->currentIndex() + 1));
}

int QuantizeDialog::outOf() const
{
    return qRound(qPow(8, ui->comboBoxOutOf->currentIndex() + 1));
}

DitherMode QuantizeDialog::ditherMode() const
{
    switch (ui->comboBoxDither->currentIndex()) {
    case 1: return DitherMode::FloydSteinberg;
    case 2: return DitherMode::Pattern;
    default: return DitherMode::None;
    }
}

PaletteSortMode QuantizeDialog::sortMode() const
{
    switch (ui->comboBoxSorting->currentIndex()) {
    case 1: return PaletteSortMode::DarkToLight;
    case 2: return PaletteSortMode::LightToDark;
    default: return PaletteSortMode::None;
    }
}

void QuantizeDialog::setPaletteMode(int index)
{
    bool optimal = index == 1;
    if (!optimal) {
        ui->comboBoxColors->setCurrentIndex(currentColorsIndex_);
        ui->comboBoxOutOf->setCurrentIndex(currentOutOfIndex_);
    }
    ui->comboBoxColors->setEnabled(optimal);
    ui->comboBoxOutOf->setEnabled(optimal);
    ui->comboBoxSorting->setEnabled(optimal);
}
