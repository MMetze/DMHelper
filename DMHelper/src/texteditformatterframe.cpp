#include "texteditformatterframe.h"
//#include "ui_texteditformatterframe.h"
#include <QFontDatabase>
#include <QTextEdit>
#include <QDebug>

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
        connect(_textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(loadCurrentFormat()));
        loadCurrentFormat();
    }
}

void TextEditFormatterFrame::loadCurrentFormat()
{
    if(!_textEdit)
        return;

    emit fontFamilyChanged(_textEdit->currentFont().family());
    emit fontSizeChanged(_textEdit->fontPointSize());
    emit fontBoldChanged(_textEdit->fontWeight() == QFont::Bold);
    emit fontItalicsChanged(_textEdit->fontItalic());
    emit fontUnderlineChanged(_textEdit->fontUnderline());
    emit alignmentChanged(_textEdit->alignment());
    emit colorChanged(_textEdit->textColor());
}

void TextEditFormatterFrame::setFont(const QString& fontFamily)
{
    if(!_textEdit)
        return;

    if(fontFamily != _textEdit->fontFamily())
    {
        // TODO: This still doesn't seem particularly solid, can it be improved in the interactions with various formats?
        QTextCursor cursor = _textEdit->textCursor();
        QTextCharFormat format = cursor.charFormat();
        QFont formatFont(fontFamily, format.fontPointSize(), format.fontWeight(), format.fontItalic());
        format.setFont(formatFont);
        cursor.mergeCharFormat(format);
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
        _textEdit->setFontPointSize(fontSize);
        emit fontSizeChanged(fontSize);
    }
}

void TextEditFormatterFrame::setBold(bool bold)
{
    if(!_textEdit)
        return;

    if((_textEdit->fontWeight() == QFont::Bold) != bold)
    {
        _textEdit->setFontWeight(bold ? QFont::Bold : QFont::Normal);
        emit fontBoldChanged(bold);
    }
}

void TextEditFormatterFrame::setItalics(bool italics)
{
    if(!_textEdit)
        return;

    if(_textEdit->fontItalic() != italics)
    {
        _textEdit->setFontItalic(italics);
        emit fontItalicsChanged(italics);
    }
}

void TextEditFormatterFrame::setUnterline(bool underline)
{
    if(!_textEdit)
        return;

    if(_textEdit->fontUnderline() != underline)
    {
        _textEdit->setFontUnderline(underline);
        emit fontUnderlineChanged(underline);
    }
}

void TextEditFormatterFrame::setColor(QColor color)
{
    if(!_textEdit)
        return;

    if(_textEdit->textColor() != color)
    {
        _textEdit->setTextColor(color);
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

