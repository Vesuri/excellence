#include <QDialogButtonBox>
#include <QFontDialog>
#include <QFontMetrics>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include "brush.h"
#include "buffer.h"
#include "drawtool.h"
#include "texttool.h"

// --- TextDialog ---

TextDialog::TextDialog(QWidget *parent) : QDialog(parent),
    font_("Arial", 18)
{
    setWindowTitle("Text");

    textEdit_ = new QLineEdit;
    textEdit_->setPlaceholderText("Enter text...");

    QPushButton *fontButton = new QPushButton("Choose Font...");
    fontLabel_ = new QLabel;
    preview_ = new QLabel;
    preview_->setAlignment(Qt::AlignCenter);
    preview_->setMinimumHeight(60);
    preview_->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    updatePreview();

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textEdit_);
    layout->addWidget(fontButton);
    layout->addWidget(fontLabel_);
    layout->addWidget(preview_);
    layout->addWidget(buttons);

    connect(fontButton, SIGNAL(clicked()), this, SLOT(chooseFont()));
    connect(textEdit_, SIGNAL(textChanged(QString)), this, SLOT(updatePreview()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

QString TextDialog::text() const
{
    return textEdit_->text();
}

QFont TextDialog::font() const
{
    return font_;
}

void TextDialog::chooseFont()
{
    bool ok;
    QFont selected = QFontDialog::getFont(&ok, font_, this);
    if (ok) {
        font_ = selected;
        updatePreview();
    }
}

void TextDialog::updatePreview()
{
    QString desc = QString("%1, %2pt").arg(font_.family()).arg(font_.pointSize());
    if (font_.bold()) desc += ", Bold";
    if (font_.italic()) desc += ", Italic";
    fontLabel_->setText(desc);

    preview_->setFont(font_);
    QString t = textEdit_->text();
    preview_->setText(t.isEmpty() ? "Preview" : t);
}

// --- TextTool ---

TextTool TextTool::instance;

TextTool::TextTool(QObject *parent) : Tool(parent)
{
}

void TextTool::setBuffer(Buffer *buffer)
{
    if (buffer_ != nullptr)
        disconnect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
    Tool::setBuffer(buffer);
    if (buffer_ != nullptr)
        connect(buffer_, SIGNAL(toolChanged(Tool*)), this, SLOT(setCheckedIfEqual(Tool*)));
}

QRect TextTool::press(const QPoint &, const Qt::KeyboardModifiers &)
{
    return QRect();
}

QRect TextTool::move(const QPoint &)
{
    return QRect();
}

QRect TextTool::release(const QPoint &)
{
    return QRect();
}

void TextTool::registerTool()
{
    Tool::registerTool();
    button_->setIcon(QIcon(":/text.png"));
    button_->setCheckable(true);
    connect(button_, SIGNAL(clicked(bool)), this, SLOT(activate()));
}

void TextTool::activate()
{
    Tool::activate();

    TextDialog dialog;
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString text = dialog.text();
    if (text.isEmpty())
        return;

    QFont font = dialog.font();
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);
    if (textRect.isEmpty())
        return;

    // Render text to ARGB32 image with transparent background
    QImage textImage(textRect.width(), textRect.height(), QImage::Format_ARGB32);
    textImage.fill(Qt::transparent);
    {
        QPainter p(&textImage);
        p.setFont(font);
        p.setPen(Qt::white);
        p.drawText(-textRect.left(), -textRect.top(), text);
    }

    // Convert to indexed image using buffer's palette
    QImage indexedImage(textImage.size(), QImage::Format_Indexed8);
    indexedImage.setColorTable(buffer_->image().colorTable());
    indexedImage.fill(0);
    int paintIdx = static_cast<int>(buffer_->paintColor());
    for (int y = 0; y < textImage.height(); y++) {
        for (int x = 0; x < textImage.width(); x++) {
            if (qAlpha(textImage.pixel(x, y)) > 127)
                indexedImage.setPixel(x, y, paintIdx);
        }
    }

    // Set brush as pen and activate DrawTool
    buffer_->setPen(new Brush(indexedImage, 0, buffer_));
    for (Tool *t : tools) {
        if (qobject_cast<DrawTool *>(t)) {
            t->click();
            break;
        }
    }
}

void TextTool::addButtonToGridLayout(QGridLayout *layout)
{
    layout->addWidget(button_, 1, 4);
}
