#include <QtMath>
#include "importimagedialog.h"
#include "ui_importimagedialog.h"

ImportImageDialog::ImportImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportImageDialog)
{
    ui->setupUi(this);

    connect(ui->comboBoxPalette, SIGNAL(currentIndexChanged(int)), this, SLOT(setPaletteMode(int)));

    ui->comboBoxPalette->setCurrentIndex(1);
    setPaletteMode(1);
}

ImportImageDialog::~ImportImageDialog()
{
    delete ui;
}

void ImportImageDialog::setImageSize(int width, int height)
{
    ui->spinBoxWidth->setValue(width);
    ui->spinBoxHeight->setValue(height);
}

int ImportImageDialog::width() const
{
    return ui->spinBoxWidth->value();
}

int ImportImageDialog::height() const
{
    return ui->spinBoxHeight->value();
}

void ImportImageDialog::setCurrentColorCount(int colors)
{
    currentColorsIndex_ = 0;
    for (int c = colors; c > 2; c >>= 1) {
        currentColorsIndex_++;
    }

    if (ui->comboBoxPalette->currentIndex() == 0) {
        ui->comboBoxColors->setCurrentIndex(currentColorsIndex_);
        ui->comboBoxOutOf->setCurrentIndex(currentColorsIndex_);
    }
}

bool ImportImageDialog::useOptimalPalette() const
{
    return ui->comboBoxPalette->currentIndex() == 1;
}

int ImportImageDialog::colors() const
{
    return qRound(qPow(2, ui->comboBoxColors->currentIndex() + 1));
}

DitherMode ImportImageDialog::ditherMode() const
{
    switch (ui->comboBoxDither->currentIndex()) {
    case 1: return DitherMode::FloydSteinberg;
    case 2: return DitherMode::Pattern;
    default: return DitherMode::None;
    }
}

void ImportImageDialog::setPaletteMode(int index)
{
    bool optimal = index == 1;

    if (!optimal) {
        ui->comboBoxColors->setCurrentIndex(currentColorsIndex_);
        ui->comboBoxOutOf->setCurrentIndex(currentColorsIndex_);
    }

    ui->comboBoxColors->setEnabled(optimal);
    ui->comboBoxOutOf->setEnabled(optimal);
}
