#include "ribbontabtext.h"
#include "ui_ribbontabtext.h"
#include <QFontDatabase>

RibbonTabText::RibbonTabText(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabText)
{
    ui->setupUi(this);

    connect(ui->btnBackground, &QAbstractButton::clicked, this, &RibbonTabText::backgroundClicked);

    connect(ui->frameText, &RibbonFrameText::colorChanged, this, &RibbonTabText::colorChanged);
    connect(ui->frameText, &RibbonFrameText::fontFamilyChanged, this, &RibbonTabText::fontFamilyChanged);
    connect(ui->frameText, &RibbonFrameText::fontSizeChanged, this, &RibbonTabText::fontSizeChanged);
    connect(ui->frameText, &RibbonFrameText::fontBoldChanged, this, &RibbonTabText::fontBoldChanged);
    connect(ui->frameText, &RibbonFrameText::fontItalicsChanged, this, &RibbonTabText::fontItalicsChanged);
    connect(ui->frameText, &RibbonFrameText::alignmentChanged, this, &RibbonTabText::alignmentChanged);

//    connect(ui->btnHyperlink, SIGNAL(clicked()), this, SIGNAL(hyperlinkClicked()));
    connect(ui->btnHyperlink, &QAbstractButton::clicked, this, &RibbonTabText::hyperlinkClicked);

    connect(ui->btnAnimation, &QAbstractButton::clicked, this, &RibbonTabText::animationClicked);
}

RibbonTabText::~RibbonTabText()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabText::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabText::setAnimation(bool checked)
{
    ui->btnAnimation->setChecked(checked);
}

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

void RibbonTabText::setHyperlinkActive(bool active)
{
    ui->btnHyperlink->setEnabled(active);
}

void RibbonTabText::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblBackground, *ui->btnBackground, frameHeight);
    setLineHeight(*ui->line_1, frameHeight);
    setStandardButtonSize(*ui->lblHyperlink, *ui->btnHyperlink, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setStandardButtonSize(*ui->lblAnimation, *ui->btnAnimation, frameHeight);
    setStandardButtonSize(*ui->lblRewind, *ui->btnRewind, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setStandardButtonSize(*ui->lblTranslateText, *ui->btnTranslateText, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);
}

