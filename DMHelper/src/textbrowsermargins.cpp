#include "textbrowsermargins.h"
#include <QMargins>

TextBrowserMargins::TextBrowserMargins(QWidget *parent) :
    QTextBrowser(parent),
    _textWidth(100)
{
}

void TextBrowserMargins::setTextWidth(int textWidth)
{
    if((textWidth < 10) || (textWidth > 100) || (textWidth == _textWidth))
        return;

    _textWidth = textWidth;
    updateTextWidth();
}

void TextBrowserMargins::resizeEvent(QResizeEvent *event)
{
    updateTextWidth();
    QTextEdit::resizeEvent(event);
}

void TextBrowserMargins::updateTextWidth()
{
    int absoluteWidth = width() * _textWidth / 100;
    int targetMargin = (width() - absoluteWidth) / 2;

    QMargins margins = viewportMargins();
    margins.setLeft(targetMargin);
    margins.setRight(targetMargin);
    setViewportMargins(margins);
}

