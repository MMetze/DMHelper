#include "texteditmargins.h"
#include <QMargins>

TextEditMargins::TextEditMargins(QWidget *parent) :
    QTextEdit(parent),
    _textWidth(100)
{
}

void TextEditMargins::setTextWidth(int textWidth)
{
    if((textWidth < 10) || (textWidth > 100) || (textWidth == _textWidth))
        return;

    _textWidth = textWidth;
    updateTextWidth();
}

void TextEditMargins::resizeEvent(QResizeEvent *event)
{
    updateTextWidth();
    QTextEdit::resizeEvent(event);
}

void TextEditMargins::updateTextWidth()
{
    int absoluteWidth = width() * _textWidth / 100;
    int targetMargin = (width() - absoluteWidth) / 2;

    QMargins margins = viewportMargins();
    margins.setLeft(targetMargin);
    margins.setRight(targetMargin);
    setViewportMargins(margins);
}

