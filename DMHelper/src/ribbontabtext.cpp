#include "ribbontabtext.h"
#include "ui_ribbontabtext.h"
#include <QFontDatabase>

RibbonTabText::RibbonTabText(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabText)
{
    ui->setupUi(this);

    connect(ui->frameText, &RibbonFrameText::colorChanged, this, &RibbonTabText::colorChanged);
    connect(ui->frameText, &RibbonFrameText::fontFamilyChanged, this, &RibbonTabText::fontFamilyChanged);
    connect(ui->frameText, &RibbonFrameText::fontSizeChanged, this, &RibbonTabText::fontSizeChanged);
    connect(ui->frameText, &RibbonFrameText::fontBoldChanged, this, &RibbonTabText::fontBoldChanged);
    connect(ui->frameText, &RibbonFrameText::fontItalicsChanged, this, &RibbonTabText::fontItalicsChanged);
    connect(ui->frameText, &RibbonFrameText::alignmentChanged, this, &RibbonTabText::alignmentChanged);
}

RibbonTabText::~RibbonTabText()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabText::getPublishRibbon()
{
    return ui->framePublish;
}

/*
QColor RibbonTabText::getColor() const
{
    return ui->btnColor->getColor();
}

QString RibbonTabText::getFontFamily() const
{
    return ui->cmbFont->currentText();
}

int RibbonTabText::getFontSize() const
{
    return ui->edtSize->text().toInt();
}

bool RibbonTabText::isFontBold() const
{
    return ui->btnBold->isChecked();
}

bool RibbonTabText::isFontItalics() const
{
    return ui->btnItalics->isChecked();
}

int RibbonTabText::getAlignment() const
{
    switch(ui->btnGrpAlignment->checkedId())
    {
        case Qt::AlignLeft:
        case Qt::AlignHCenter:
        case Qt::AlignRight:
            return static_cast<Qt::AlignmentFlag>(ui->btnGrpAlignment->checkedId());
        default:
            return Qt::AlignHCenter;
    }
}
*/

void RibbonTabText::setColor(QColor color)
{
    ui->frameText->setColor(color);
}

void RibbonTabText::setFontFamily(const QString& fontFamily)
{
    ui->frameText->setFontFamily(fontFamily);
}

void RibbonTabText::setFontSize(int fontSize)
{
    ui->frameText->setFontSize(fontSize);
}

void RibbonTabText::setFontBold(bool fontBold)
{
    ui->frameText->setFontBold(fontBold);
}

void RibbonTabText::setFontItalics(bool fontItalics)
{
    ui->frameText->setFontItalics(fontItalics);
}

void RibbonTabText::setAlignment(Qt::Alignment alignment)
{
    ui->frameText->setAlignment(alignment);
}

/*
void RibbonTabText::handleFontSizeChanged()
{
    bool ok = false;
    int intSize = ui->edtSize->text().toInt(&ok);
    if(ok)
        emit fontSizeChanged(intSize);

}

void RibbonTabText::handleAlignmentChanged()
{
    emit alignmentChanged(getAlignment());
}
*/

/*
void RibbonTabText::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    setStandardButtonSize(*ui->lblColor, *ui->btnColor);

    int iconSize = getIconSize(ui->btnColor->width(), ui->btnColor->height());
    setWidgetSize(*ui->btnColor, iconSize, iconSize);

    int labelHeight = getLabelHeight(*ui->lblColor);
    int buttonSize = height() - labelHeight;
    QFontMetrics metrics = ui->lblFont->fontMetrics();
    int halfHeight = (height() - ui->fontLayout->spacing()) / 2;

    // Font cluster
    setWidgetSize(*ui->edtSize, metrics.maxWidth() * 4, halfHeight);
    setButtonSize(*ui->btnBold, halfHeight, halfHeight);
    setButtonSize(*ui->btnItalics, halfHeight, halfHeight);
    setWidgetSize(*ui->cmbFont, ui->edtSize->width() + ui->btnBold->width() + ui->btnItalics->width(), halfHeight);

    // Alignment cluster
    setButtonSize(*ui->btnLeftText, buttonSize, buttonSize);
    setButtonSize(*ui->btnCenterText, buttonSize, buttonSize);
    setButtonSize(*ui->btnRightText, buttonSize, buttonSize);
    setWidgetSize(*ui->lblAlignment, 3*buttonSize, buttonSize);
}
*/