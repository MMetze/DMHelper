#include "texteditformatterframe.h"
#include <QFontDatabase>
#include <QTextEdit>
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

    emit fontFamilyChanged(f.fontFamily());
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

