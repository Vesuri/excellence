#include <cmath>
#include <functional>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QToolButton>
#include <QVBoxLayout>
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
    const int W = 14, H = 13;
    QPixmap pm(W, H);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    const int cx = W / 2, cy = H / 2;
    int hw = w / 2, hh = h / 2;
    for (int dy = -hh; dy <= hh; dy++) {
        for (int dx = -hw; dx <= hw; dx++) {
            bool inside = (shape == PenTip::Square)
                          ? true
                          : (dx * dx + dy * dy <= hw * hw + hw / 2);
            if (inside)
                p.fillRect(cx + dx, cy + dy, 1, 1, Qt::black);
        }
    }
    return pm;
}

// Interactive widget for sizing a circular pen tip by dragging from centre.
class PenTipCircleSizer : public QWidget {
public:
    std::function<void(int)> onSizeChosen;

    explicit PenTipCircleSizer(int initialSize, QWidget *parent = nullptr)
        : QWidget(parent), dragging_(false), size_(initialSize)
    {
        setFixedSize(60, 60);
        setCursor(Qt::CrossCursor);
    }

protected:
    void mousePressEvent(QMouseEvent *e) override {
        dragging_ = true;
        center_ = e->pos();
        size_ = 1;
        update();
    }

    void mouseMoveEvent(QMouseEvent *e) override {
        if (!dragging_) return;
        QPoint d = e->pos() - center_;
        int r = static_cast<int>(std::round(std::sqrt(double(d.x()*d.x() + d.y()*d.y()))));
        size_ = qMax(1, 2*r + 1);
        update();
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        if (!dragging_) return;
        mouseMoveEvent(e);
        dragging_ = false;
        if (onSizeChosen) onSizeChosen(size_);
        update();
    }

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.fillRect(rect(), palette().window());
        QPoint c = dragging_ ? center_ : QPoint(width()/2, height()/2);
        int r = size_ / 2;
        for (int dy = -r; dy <= r; dy++)
            for (int dx = -r; dx <= r; dx++)
                if (dx*dx + dy*dy <= r*r + r/2)
                    p.fillRect(c.x()+dx, c.y()+dy, 1, 1, Qt::black);
        p.setPen(palette().mid().color());
        p.setBrush(Qt::NoBrush);
        p.drawRect(0, 0, width()-1, height()-1);
    }

private:
    bool dragging_;
    QPoint center_;
    int size_;
};

// Interactive widget for sizing a square pen tip by dragging from the bottom-right corner.
class PenTipRectSizer : public QWidget {
public:
    std::function<void(int,int)> onSizeChosen;

    explicit PenTipRectSizer(int initialSize, QWidget *parent = nullptr)
        : QWidget(parent), dragging_(false), size_(initialSize)
    {
        setFixedSize(60, 60);
        setCursor(Qt::CrossCursor);
    }

protected:
    void mousePressEvent(QMouseEvent *e) override {
        dragging_ = true;
        corner_ = e->pos();
        size_ = 1;
        update();
    }

    void mouseMoveEvent(QMouseEvent *e) override {
        if (!dragging_) return;
        QPoint d = corner_ - e->pos();
        size_ = qMax(1, qMax(qAbs(d.x()), qAbs(d.y())));
        update();
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        if (!dragging_) return;
        mouseMoveEvent(e);
        dragging_ = false;
        if (onSizeChosen) onSizeChosen(size_, size_);
        update();
    }

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.fillRect(rect(), palette().window());
        QRect box;
        if (dragging_) {
            box = QRect(corner_.x() - size_ + 1, corner_.y() - size_ + 1, size_, size_);
        } else {
            int half = size_ / 2;
            box = QRect(width()/2 - half, height()/2 - half, size_, size_);
        }
        p.fillRect(box, Qt::black);
        p.setPen(palette().mid().color());
        p.setBrush(Qt::NoBrush);
        p.drawRect(0, 0, width()-1, height()-1);
    }

private:
    bool dragging_;
    QPoint corner_;
    int size_;
};

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

    QVBoxLayout *vbox = new QVBoxLayout(w);
    vbox->setSpacing(4);
    vbox->setContentsMargins(4, 4, 4, 4);

    // Row 1: fixed-size preset buttons
    QHBoxLayout *presetsRow = new QHBoxLayout;
    presetsRow->setSpacing(2);
    presetsRow->setContentsMargins(0, 0, 0, 0);

    struct Preset { PenTip::Shape shape; int pw; int ph; };
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
        QToolButton *btn = new QToolButton(w);
        btn->setIcon(QIcon(renderTip(preset.shape, preset.pw, preset.ph)));
        PenTip::Shape shape = preset.shape;
        int pw = preset.pw, ph = preset.ph;
        connect(btn, &QToolButton::clicked, [this, shape, pw, ph]() {
            PenTip *tip = qobject_cast<PenTip *>(buffer_->pen());
            if (tip) {
                tip->setSize(pw, ph);
                tip->setShape(shape);
                updateButtonIcon();
            }
        });
        presetsRow->addWidget(btn);
    }
    presetsRow->addStretch();
    vbox->addLayout(presetsRow);

    // Row 2: interactive size-by-drag widgets
    QHBoxLayout *sizersRow = new QHBoxLayout;
    sizersRow->setSpacing(4);
    sizersRow->setContentsMargins(0, 0, 0, 0);

    auto *circleSizer = new PenTipCircleSizer(13, w);
    circleSizer->onSizeChosen = [this](int s) {
        PenTip *tip = qobject_cast<PenTip *>(buffer_->pen());
        if (tip) {
            tip->setSize(s, s);
            tip->setShape(PenTip::Circle);
            updateButtonIcon();
        }
    };

    auto *rectSizer = new PenTipRectSizer(8, w);
    rectSizer->onSizeChosen = [this](int sw, int sh) {
        PenTip *tip = qobject_cast<PenTip *>(buffer_->pen());
        if (tip) {
            tip->setSize(sw, sh);
            tip->setShape(PenTip::Square);
            updateButtonIcon();
        }
    };

    sizersRow->addWidget(circleSizer);
    sizersRow->addWidget(rectSizer);
    sizersRow->addStretch();
    vbox->addLayout(sizersRow);

    vbox->addStretch();
    return w;
}

void PenTipTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 9);
}
