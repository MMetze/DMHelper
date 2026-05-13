#include "textbrowsermargins.h"
#include <QMargins>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTextCursor>
#include <QTextBlock>

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
    QTextBrowser::resizeEvent(event);
}

void TextBrowserMargins::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::AltModifier)
        event->ignore();
    else
        QTextBrowser::keyPressEvent(event);
}

void TextBrowserMargins::mouseReleaseEvent(QMouseEvent *event)
{
    static const QChar checkboxUnchecked(0x2610); // ☐
    static const QChar checkboxChecked(0x2611);   // ☑

    if(event->button() == Qt::LeftButton)
    {
        QTextCursor clickCursor = cursorForPosition(event->pos());
        QTextBlock block = clickCursor.block();
        QString blockText = block.text();

        if(!blockText.isEmpty() && (blockText[0] == checkboxUnchecked || blockText[0] == checkboxChecked))
        {
            int posInBlock = clickCursor.position() - block.position();
            // Toggle if the user clicked on the checkbox character itself (position 0)
            // or the space right after it (position 1)
            if(posInBlock <= 1)
            {
                QTextCursor editCursor = textCursor();
                int savedPos = editCursor.position();

                QTextCursor toggleCursor(document());
                toggleCursor.setPosition(block.position());
                toggleCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);

                QChar replacement = (blockText[0] == checkboxUnchecked) ? checkboxChecked : checkboxUnchecked;
                toggleCursor.insertText(QString(replacement));

                // Restore the user's editing cursor
                editCursor.setPosition(savedPos);
                setTextCursor(editCursor);

                event->accept();
                return;
            }
        }
    }

    QTextBrowser::mouseReleaseEvent(event);
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

