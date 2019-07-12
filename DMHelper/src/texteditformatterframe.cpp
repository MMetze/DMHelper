#include "texteditformatterframe.h"
#include "ui_texteditformatterframe.h"
#include <QFontDatabase>
#include <QTextEdit>

TextEditFormatterFrame::TextEditFormatterFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TextEditFormatterFrame),
    _textEdit(nullptr)
{
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
}

TextEditFormatterFrame::~TextEditFormatterFrame()
{
    delete ui;
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

    ui->cmbFont->setEditText(_textEdit->currentFont().family());
    ui->edtSize->setText(QString::number(_textEdit->fontPointSize()));
    ui->chkBold->setChecked(_textEdit->fontWeight() == QFont::Bold);
    ui->chkItalics->setChecked(_textEdit->fontItalic());
    ui->chkUnderline->setChecked(_textEdit->fontUnderline());

    switch(_textEdit->alignment())
    {
        case Qt::AlignHCenter:
            ui->btnCenterText->setChecked(true);
            break;
        case Qt::AlignRight:
            ui->btnRightText->setChecked(true);
            break;
        default: //case Qt::AlignLeft:
            ui->btnLeftText->setChecked(true);
            break;
    }
}

void TextEditFormatterFrame::setFont()
{
    if(!_textEdit)
        return;

    _textEdit->setFontFamily(ui->cmbFont->currentText());
}

void TextEditFormatterFrame::setFontSize()
{
    if(!_textEdit)
        return;

    int textSize = ui->edtSize->text().toInt();
    if(textSize > 0)
        _textEdit->setFontPointSize(textSize);
}

void TextEditFormatterFrame::setBold(bool bold)
{
    if(!_textEdit)
        return;

    _textEdit->setFontWeight(bold ? QFont::Bold : QFont::Normal);
}

void TextEditFormatterFrame::setItalics(bool italics)
{
    if(!_textEdit)
        return;

    _textEdit->setFontItalic(italics);
}

void TextEditFormatterFrame::setUnterline(bool underline)
{
    if(!_textEdit)
        return;

    _textEdit->setFontUnderline(underline);
}

void TextEditFormatterFrame::setColor()
{

}

void TextEditFormatterFrame::setAlignment(int id)
{
    if(!_textEdit)
        return;

    _textEdit->setAlignment(static_cast<Qt::AlignmentFlag>(id));
}

