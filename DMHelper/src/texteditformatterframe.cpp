#include "texteditformatterframe.h"
#include <QFontDatabase>
#include <QTextEdit>
#include <QTextBlock>
#include <QDebug>

TextEditFormatterFrame::TextEditFormatterFrame(QWidget *parent) :
    QObject(parent),
    _textEdit(nullptr)
{
}

TextEditFormatterFrame::~TextEditFormatterFrame()
{
}

void TextEditFormatterFrame::setTextEdit(QTextEdit* textEdit)
{
    if(textEdit == _textEdit)
        return;

    if(_textEdit)
        disconnect(_textEdit, nullptr, this, nullptr);

    _textEdit = textEdit;
    if(_textEdit)
    {
        connect(_textEdit, &QTextEdit::currentCharFormatChanged, this, &TextEditFormatterFrame::loadCurrentCharFormat);
        loadCurrentFormat();
    }
}

void TextEditFormatterFrame::loadCurrentFormat()
{
    if(!_textEdit)
        return;

    loadCurrentCharFormat(_textEdit->currentCharFormat());
}

void TextEditFormatterFrame::loadCurrentCharFormat(const QTextCharFormat &f)
{
    if(!_textEdit)
        return;

    QStringList families = f.fontFamilies().toStringList();
    if(!families.isEmpty())
        emit fontFamilyChanged(families.first());

    emit fontSizeChanged(f.fontPointSize());
    emit fontBoldChanged(f.fontWeight() == QFont::Bold);
    emit fontItalicsChanged(f.fontItalic());
    emit fontUnderlineChanged(f.fontUnderline());
    emit alignmentChanged(_textEdit->alignment());
    emit colorChanged(f.foreground().color());
}

void TextEditFormatterFrame::setFont(const QString& fontFamily)
{
    if(!_textEdit)
        return;

    if(fontFamily != _textEdit->fontFamily())
    {
        QTextCharFormat format;
        QFont formatFont(fontFamily, format.fontPointSize(), format.fontWeight(), format.fontItalic());
        format.setFont(formatFont);
        _textEdit->mergeCurrentCharFormat(format);
        emit fontFamilyChanged(fontFamily);
    }
}

void TextEditFormatterFrame::setFontSize(int fontSize)
{
    if(!_textEdit)
        return;

    if((fontSize > 0) && (fontSize != _textEdit->fontPointSize()))
    {
        QTextCharFormat format;
        format.setFontPointSize(fontSize);
        _textEdit->mergeCurrentCharFormat(format);
        emit fontSizeChanged(fontSize);
    }
}

void TextEditFormatterFrame::setBold(bool bold)
{
    if(!_textEdit)
        return;

    if((_textEdit->fontWeight() == QFont::Bold) != bold)
    {
        QTextCharFormat format;
        format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
        _textEdit->mergeCurrentCharFormat(format);
        emit fontBoldChanged(bold);
    }
}

void TextEditFormatterFrame::setItalics(bool italics)
{
    if(!_textEdit)
        return;

    if(_textEdit->fontItalic() != italics)
    {
        QTextCharFormat format;
        format.setFontItalic(italics);
        _textEdit->mergeCurrentCharFormat(format);
        emit fontItalicsChanged(italics);
    }
}

void TextEditFormatterFrame::setUnterline(bool underline)
{
    if(!_textEdit)
        return;

    if(_textEdit->fontUnderline() != underline)
    {
        QTextCharFormat format;
        format.setFontUnderline(underline);
        _textEdit->mergeCurrentCharFormat(format);
        emit fontUnderlineChanged(underline);
    }
}

void TextEditFormatterFrame::setColor(const QColor& color)
{
    if(!_textEdit)
        return;

    if(_textEdit->textColor() != color)
    {
        QTextCharFormat format;
        format.setForeground(QBrush(color));
        _textEdit->mergeCurrentCharFormat(format);
        emit colorChanged(color);
    }
}

void TextEditFormatterFrame::setAlignment(Qt::Alignment alignment)
{
    if(!_textEdit)
        return;

    if(_textEdit->alignment() != alignment)
    {
        _textEdit->setAlignment(alignment);
        emit alignmentChanged(alignment);
    }
}

void TextEditFormatterFrame::toggleCheckbox()
{
    if(!_textEdit)
        return;

    static const QChar checkboxUnchecked(0x2610); // ☐
    static const QChar checkboxChecked(0x2611);   // ☑

    QTextCursor cursor = _textEdit->textCursor();
    int originalPos = cursor.position();

    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    QString firstChar = cursor.selectedText();

    if(!firstChar.isEmpty() && (firstChar[0] == checkboxUnchecked || firstChar[0] == checkboxChecked))
    {
        // Remove checkbox and trailing space
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        // Check for trailing space
        QTextCursor spaceCheck = cursor;
        spaceCheck.movePosition(QTextCursor::StartOfBlock);
        spaceCheck.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
        QString twoChars = spaceCheck.selectedText();
        if(twoChars.length() == 2 && twoChars[1] == QChar(' '))
            cursor = spaceCheck;

        cursor.removeSelectedText();

        // Adjust cursor position
        int removed = cursor.position() - (originalPos - twoChars.length());
        int newPos = qMax(cursor.position(), originalPos - twoChars.length());
        cursor.setPosition(newPos);
    }
    else
    {
        // Insert checkbox at block start
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.insertText(QString(checkboxUnchecked) + QChar(' '));

        // Restore cursor position (shifted by 2 inserted chars)
        cursor.setPosition(originalPos + 2);
    }

    _textEdit->setTextCursor(cursor);
}

