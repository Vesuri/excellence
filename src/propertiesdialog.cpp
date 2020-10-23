#include <QtMath>
#include "palettequantizer.h"
#include "buffer.h"
#include "propertiesdialog.h"
#include "ui_propertiesdialog.h"

PropertiesDialog::PropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertiesDialog),
    buffer(nullptr)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(setProperties()));
    connect(ui->checkBoxRetainImage, SIGNAL(stateChanged(int)), this, SLOT(setRetainImageState(int)));
}

PropertiesDialog::~PropertiesDialog()
{
    delete ui;
}

void PropertiesDialog::setBuffer(Buffer *buffer)
{
    this->buffer = buffer;

    ui->spinBoxWidth->setValue(buffer->image().width());
    ui->spinBoxHeight->setValue(buffer->image().height());

    int colorsIndex = 0;
    for (int i = buffer->image().colorCount(); i > 2; i >>= 1) {
        colorsIndex++;
    }
    ui->comboBoxColors->setCurrentIndex(colorsIndex);
}

void PropertiesDialog::setProperties()
{
    int width = ui->spinBoxWidth->value();
    int height = ui->spinBoxHeight->value();
    int colors = qPow(2, ui->comboBoxColors->currentIndex() + 1);
    bool remapPalette = ui->comboBoxPalette->currentIndex() == 1;
    PaletteQuantizer paletteQuantizer;

    if (remapPalette) {
        for (int y = 0; y < buffer->image().height(); y++) {
            for (int x = 0; x < buffer->image().width(); x++) {
                paletteQuantizer.addColor(buffer->image().pixelColor(x, y));
            }
        }
    }

    Buffer *newBuffer = new Buffer(width, height, colors, buffer->parent());
    if (remapPalette) {
        QList<QColor> palette = paletteQuantizer.getPalette(colors);
        for (int i = 0; i < palette.count(); i++) {
            newBuffer->image().setColor(i, palette[i].rgba());
        }
    } else {
        for (int i = 0; i < qMin(colors, buffer->image().colorCount()); i++) {
            newBuffer->image().setColor(i, buffer->image().color(i));
        }
    }

    if (ui->checkBoxRetainImage->isChecked()) {
        if (ui->comboBoxScaling->currentIndex() == 0) {
            for (int y = 0; y < qMin(height, buffer->image().height()); y++) {
                for (int x = 0; x < qMin(width, buffer->image().width()); x++) {
                    if (remapPalette) {
                        newBuffer->image().setPixel(x, y, paletteQuantizer.getPaletteIndex(buffer->image().pixelColor(x, y)));
                    } else {
                        newBuffer->image().setPixel(x, y, static_cast<uint>(qMin(colors - 1, buffer->image().pixelIndex(x, y))));
                    }
                }
            }
        } else {
            qreal sxDelta = static_cast<qreal>(buffer->image().width()) / static_cast<qreal>(width);
            qreal syDelta = static_cast<qreal>(buffer->image().height()) / static_cast<qreal>(height);
            qreal sy = 0;
            for (int dy = 0; dy < height; dy++, sy += syDelta) {
                qreal sx = 0;
                for (int dx = 0; dx < width; dx++, sx += sxDelta) {
                    newBuffer->image().setPixel(dx, dy, static_cast<uint>(qMin(colors - 1, buffer->image().pixelIndex(static_cast<int>(sx), static_cast<int>(sy)))));
                }
            }
        }
    }

    emit bufferChanged(newBuffer);
}

void PropertiesDialog::setRetainImageState(int state)
{
    bool enabled = state == Qt::Checked;

    ui->comboBoxScaling->setEnabled(enabled);
    ui->comboBoxPalette->setEnabled(enabled);
}
