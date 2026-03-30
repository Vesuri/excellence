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

static QPixmap renderTip(PenTip::Shape shape, int w, int h)
{
    const int W = 40, H = 40;
    QPixmap pm(W, H);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    const int scale = 2;
    const int cx = W / 2, cy = H / 2;
    if (w == 1 && h == 1) {
        p.fillRect(cx - 1, cy - 1, scale, scale, Qt::black);
    } else {
        int hw = w / 2, hh = h / 2;
        for (int dy = -hh; dy <= hh; dy++) {
            for (int dx = -hw; dx <= hw; dx++) {
                bool inside = (shape == PenTip::Square)
                              ? true
                              : (dx * dx + dy * dy <= hw * hw + hw / 2);
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
        button_->setIcon(QIcon(renderTip(tip->shape(), tip->width(), tip->height())));
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
    hbox->setSpacing(2);
    hbox->setContentsMargins(4, 4, 4, 4);

    struct Preset { PenTip::Shape shape; int w; int h; };
    const Preset presets[] = {
        { PenTip::Circle, 1,  1  },
        { PenTip::Circle, 5,  5  },
        { PenTip::Circle, 9,  9  },
        { PenTip::Circle, 13, 13 },
        { PenTip::Circle, 17, 17 },
        { PenTip::Square, 4,  4  },
        { PenTip::Square, 6,  6  },
        { PenTip::Square, 8,  8  },
        { PenTip::Square, 10, 10 },
        { PenTip::Square, 12, 12 },
        { PenTip::Square, 1,  13 },
        { PenTip::Square, 18, 1  },
    };

    for (const auto &preset : presets) {
        QPushButton *btn = new QPushButton(w);
        btn->setFixedSize(44, 44);
        btn->setIconSize(QSize(40, 40));
        btn->setIcon(QIcon(renderTip(preset.shape, preset.w, preset.h)));
        PenTip::Shape shape = preset.shape;
        int pw = preset.w, ph = preset.h;
        connect(btn, &QPushButton::clicked, [this, shape, pw, ph]() {
            PenTip *tip = qobject_cast<PenTip *>(buffer_->pen());
            if (tip) {
                tip->setSize(pw, ph);
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
    layout->addWidget(button_, 1, 9);
}
