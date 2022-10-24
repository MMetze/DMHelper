#include "texteditformatterframe.h"
//#include "ui_texteditformatterframe.h"
#include <QFontDatabase>
#include <QTextEdit>
#include <QDebug>



#include <QTextCursor>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextBlockFormat>

TextEditFormatterFrame::TextEditFormatterFrame(QWidget *parent) :
    QObject(parent),
    //ui(new Ui::TextEditFormatterFrame),
    _textEdit(nullptr)
{
    /*
    ui->setupUi(this);

    QFontDatabase fontDB;
    ui->cmbFont->addItems(fontDB.families());
    connect(ui->cmbFont,SIGNAL(currentIndexChanged(QString)),this,SLOT(setFont()));

    ui->edtSize->setValidator(new QIntValidator(1,999,this));
    connect(ui->edtSize,SIGNAL(textEdited(QString)),this,SLOT(setFontSize()));

    connect(ui->chkBold,SIGNAL(clicked(bool)),this,SLOT(setBold(bool)));
    connect(ui->chkItalics,SIGNAL(clicked(bool)),this,SLOT(setItalics(bool)));
    connect(ui->chkUnderline,SIGNAL(clicked(bool)),this,SLOT(setUnterline(bool)));

    ui->buttonGroup->setId(ui->btnLeftText,Qt::AlignLeft);
    ui->buttonGroup->setId(ui->btnCenterText,Qt::AlignHCenter);
    ui->buttonGroup->setId(ui->btnRightText,Qt::AlignRight);
    connect(ui->buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(setAlignment(int)));
    */
}

TextEditFormatterFrame::~TextEditFormatterFrame()
{
    //delete ui;
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
        //connect(_textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(loadCurrentFormat()));
        connect(_textEdit, &QTextEdit::currentCharFormatChanged, this, &TextEditFormatterFrame::loadCurrentCharFormat);
        loadCurrentFormat();
    }
}

void TextEditFormatterFrame::loadCurrentFormat()
{
    if(!_textEdit)
        return;

    /*
    QTextCursor cursor = _textEdit->textCursor();
    QTextBlock textBlock = cursor.block();

    for(int i = 0; i < textBlock.textFormats().count(); ++i)
    {
        QTextLayout::FormatRange fmt = textBlock.textFormats().at(i);
        qDebug() << "[TextEditFormatterFrame] loadCurrentFormat textBlock " << i << ", start: " << fmt.start << ", len: " << fmt.length << ", fmt weight: " << fmt.format.fontWeight();
    }
    qDebug() << "[TextEditFormatterFrame] loadCurrentFormat textEdit block: " << cursor.blockNumber() << ", fontWeight " << _textEdit->fontWeight();

    emit fontFamilyChanged(_textEdit->currentFont().family());
    emit fontSizeChanged(_textEdit->fontPointSize());
    //emit fontBoldChanged(((textBlock.textFormats().count() > 0 ? textBlock.textFormats().at(0).format.fontWeight() : _textEdit->fontWeight()) == QFont::Bold));
    emit fontBoldChanged(_textEdit->fontWeight() == QFont::Bold);
    emit fontItalicsChanged(_textEdit->fontItalic());
    emit fontUnderlineChanged(_textEdit->fontUnderline());
    emit alignmentChanged(_textEdit->alignment());
    emit colorChanged(_textEdit->textColor());
    */

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
        // TODO: This still doesn't seem particularly solid, can it be improved in the interactions with various formats?
        /*
        QTextCursor cursor = _textEdit->textCursor();
        QTextCharFormat format = cursor.charFormat();
        QFont formatFont(fontFamily, format.fontPointSize(), format.fontWeight(), format.fontItalic());
        format.setFont(formatFont);
        cursor.mergeCharFormat(format);
        emit fontFamilyChanged(fontFamily);
        */
        QTextCharFormat format;
        QFont formatFont(fontFamily, format.fontPointSize(), format.fontWeight(), format.fontItalic());
        format.setFont(formatFont);
        _textEdit->mergeCurrentCharFormat(format);
        emit fontFamilyChanged(fontFamily);
        qDebug() << "[TextEditFormatterFrame] current: " << _textEdit->fontFamily() << ", new: " << fontFamily << ", font: " << _textEdit->font() << ", format: " << formatFont;
    }
}

void TextEditFormatterFrame::setFontSize(int fontSize)
{
    if(!_textEdit)
        return;

    if((fontSize > 0) && (fontSize != _textEdit->fontPointSize()))
    {
        //_textEdit->setFontPointSize(fontSize);
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

    /*
    qDebug() << "[TextEditFormatterFrame] setBold bold = " << bold << ", weight: " << _textEdit->fontWeight();

    QTextCursor cursor = _textEdit->textCursor();
    QTextBlock textBlock = cursor.block();
    for(int i = 0; i < textBlock.textFormats().count(); ++i)
    {
        QTextLayout::FormatRange fmt = textBlock.textFormats().at(i);
        qDebug() << "[TextEditFormatterFrame] textBlock " << i << ": start: " << fmt.start << ", len: " << fmt.length << ", fmt weight: " << fmt.format.fontWeight();
    }
    qDebug() << "[TextEditFormatterFrame] block: " << cursor.blockNumber() << ", weight: " <<_textEdit->fontWeight();


    QTextCharFormat textBlockCharFormat = cursor.blockCharFormat();
    qDebug() << "[TextEditFormatterFrame] textBlockCharFormat weight: " << textBlockCharFormat.fontWeight();

    QTextBlockFormat textBlockFormat = cursor.blockFormat();
    qDebug() << "[TextEditFormatterFrame] textBlockFormat " << textBlockFormat;

    int textBlockNumber = cursor.blockNumber();
    qDebug() << "[TextEditFormatterFrame] textBlockNumber: " << textBlockNumber;

    QTextCharFormat textCharFormat = cursor.charFormat();
    qDebug() << "[TextEditFormatterFrame] textCharFormat weight: " << textCharFormat.fontWeight();
    */

//    if(((textBlock.textFormats().count() > 0 ? textBlock.textFormats().at(0).format.fontWeight() : _textEdit->fontWeight()) == QFont::Bold) != bold)
    if((_textEdit->fontWeight() == QFont::Bold) != bold)
    {
        //_textEdit->setFontWeight(bold ? QFont::Bold : QFont::Normal);
        QTextCharFormat format;
        format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
        _textEdit->mergeCurrentCharFormat(format);
        /*
        QTextCharFormat format;
        format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
        //cursor.mergeCharFormat(format);

        //_textEdit->setTextCursor(cursor);
        _textEdit->mergeCurrentCharFormat(format);
        */


        //_textEdit->setFontWeight(bold ? QFont::Bold : QFont::Normal);
        //for(int i = 0; i < textBlock.textFormats().count(); ++i)
        //    textBlock.textFormats()[i].format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
        emit fontBoldChanged(bold);
    }
}

void TextEditFormatterFrame::setItalics(bool italics)
{
    if(!_textEdit)
        return;

    if(_textEdit->fontItalic() != italics)
    {
        //_textEdit->setFontItalic(italics);
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
        //_textEdit->setFontUnderline(underline);
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
        //_textEdit->setTextColor(color);
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

