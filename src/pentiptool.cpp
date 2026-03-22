#include <QGridLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>
#include <QWidget>
#include "buffer.h"
#include "pentip.h"
#include "pentiptool.h"

PenTipTool PenTipTool::instance;

PenTipTool::PenTipTool(QObject *parent) : Tool(parent)
{
}

static QPixmap renderTip(PenTip::Shape shape, int size)
{
    const int W = 24, H = 24;
    QPixmap pm(W, H);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    const int scale = 2;
    const int cx = W / 2, cy = H / 2;
    if (size == 1) {
        p.fillRect(cx - 1, cy - 1, scale, scale, Qt::black);
    } else {
        int r = size / 2;
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                bool inside = (shape == PenTip::Square)
                              ? true
                              : (dx * dx + dy * dy <= r * r + r / 2);
                if (inside)
                    p.fillRect(cx + dx * scale - scale / 2,
                               cy + dy * scale - scale / 2,
                               scale, scale, Qt::black);
            }
        }
    }
    return pm;
}

void PenTipTool::updateButtonIcon()
{
    PenTip *tip = buffer_ ? qobject_cast<PenTip *>(buffer_->pen()) : nullptr;
    if (tip)
        button_->setIcon(QIcon(renderTip(tip->shape(), tip->size())));
    else
        button_->setIcon(QIcon(":/pentip.png"));
}

void PenTipTool::setBuffer(Buffer *buffer)
{
    Tool::setBuffer(buffer);
    updateButtonIcon();
}

void PenTipTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/pentip.png"));
    button_->setToolTip("Pen Tip");
    button_->setCheckable(false);
    connect(button_, &QToolButton::clicked, this, &PenTipTool::activate);
}

void PenTipTool::activate()
{
    toggleOptionsWidget();
}

QWidget *PenTipTool::createOptionsWidget()
{
    QWidget *w = new QWidget;
    w->setWindowTitle("Pen Tip");

    QHBoxLayout *hbox = new QHBoxLayout(w);
    hbox->setSpacing(4);
    hbox->setContentsMargins(4, 4, 4, 4);

    struct Preset { PenTip::Shape shape; int size; };
    const Preset presets[] = {
        { PenTip::Circle, 1 },
        { PenTip::Circle, 3 },
        { PenTip::Circle, 5 },
        { PenTip::Circle, 7 },
        { PenTip::Square, 1 },
        { PenTip::Square, 3 },
        { PenTip::Square, 5 },
        { PenTip::Square, 7 },
    };

    for (const auto &preset : presets) {
        QPushButton *btn = new QPushButton(w);
        btn->setIcon(QIcon(renderTip(preset.shape, preset.size)));
        PenTip::Shape shape = preset.shape;
        int size = preset.size;
        connect(btn, &QPushButton::clicked, [this, shape, size]() {
            PenTip *tip = qobject_cast<PenTip *>(buffer_->pen());
            if (tip) {
                tip->setSize(size);
                tip->setShape(shape);
                updateButtonIcon();
            }
        });
        hbox->addWidget(btn);
    }

    hbox->addStretch();
    return w;
}

void PenTipTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 12);
}
