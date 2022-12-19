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
    connect(ui->frameText, &RibbonFrameText::fontUnderlineChanged, this, &RibbonTabText::fontUnderlineChanged);
    connect(ui->frameText, &RibbonFrameText::alignmentChanged, this, &RibbonTabText::alignmentChanged);
    connect(ui->btnPasteRich, &QAbstractButton::toggled, this, &RibbonTabText::pasteRichChanged);

    connect(ui->btnHyperlink, &QAbstractButton::clicked, this, &RibbonTabText::hyperlinkClicked);
    connect(ui->btnTranslateText, &QAbstractButton::clicked, this, &RibbonTabText::translateTextClicked);
    connect(ui->btnCode, &QAbstractButton::clicked, this, &RibbonTabText::codeViewClicked);

    connect(ui->sliderWidth, &QAbstractSlider::valueChanged, this, &RibbonTabText::widthChanged);
    connect(ui->spinSpeed, SIGNAL(valueChanged(int)), this, SIGNAL(speedChanged(int)));
    connect(ui->btnRewind, SIGNAL(clicked()), this, SIGNAL(rewindClicked()));

    connect(ui->btnPlayPause, &QAbstractButton::toggled, this, &RibbonTabText::playPauseClicked);

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
    ui->spinSpeed->setEnabled(checked);
    ui->btnRewind->setEnabled(checked);
    ui->btnPlayPause->setEnabled(checked);
}

void RibbonTabText::setImageFile(const QString& imageFile)
{
    ui->btnBackground->setChecked(!imageFile.isEmpty());
}

void RibbonTabText::setColor(const QColor& color)
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

void RibbonTabText::setFontUnderline(bool fontUnderline)
{
    ui->frameText->setFontUnderline(fontUnderline);
}

void RibbonTabText::setAlignment(Qt::Alignment alignment)
{
    ui->frameText->setAlignment(alignment);
}

void RibbonTabText::setPasteRich(bool pasteRich)
{
    ui->btnPasteRich->setChecked(pasteRich);
}

void RibbonTabText::setWidth(int width)
{
    if((width != ui->sliderWidth->value()) &&
       (width >= ui->sliderWidth->minimum()) &&
       (width <= ui->sliderWidth->maximum()))
    {
        ui->sliderWidth->setValue(width);
    }
}

void RibbonTabText::setSpeed(int speed)
{
    if((speed != ui->spinSpeed->value()) &&
       (speed >= ui->spinSpeed->minimum()) &&
       (speed <= ui->spinSpeed->maximum()))
    {
        ui->spinSpeed->setValue(speed);
    }
}

void RibbonTabText::setPlaying(bool playing)
{
    ui->lblPlayPause->setText(playing ? QString("Pause") : QString("Play"));
    ui->btnPlayPause->setIcon(playing ? QIcon(":/img/data/icon_pause.png") : QIcon(":/img/data/icon_play.png"));
    ui->btnPlayPause->setChecked(playing);
}

void RibbonTabText::setHyperlinkActive(bool active)
{
    ui->btnHyperlink->setEnabled(active);
}

void RibbonTabText::setTranslationActive(bool active)
{
    ui->btnTranslateText->setChecked(active);
}

void RibbonTabText::setCodeView(bool active)
{
    ui->btnCode->setChecked(active);
}

void RibbonTabText::showCodeView(bool visible)
{
    ui->lblTranslateText->setVisible(!visible);
    ui->btnTranslateText->setVisible(!visible);
    ui->lblCode->setVisible(visible);
    ui->btnCode->setVisible(visible);
}

void RibbonTabText::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblBackground, *ui->btnBackground, frameHeight);
    setLineHeight(*ui->line_1, frameHeight);
    setStandardButtonSize(*ui->lblPasteRich, *ui->btnPasteRich, frameHeight);
    setStandardButtonSize(*ui->lblHyperlink, *ui->btnHyperlink, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setStandardButtonSize(*ui->lblAnimation, *ui->btnAnimation, frameHeight);
    setStandardButtonSize(*ui->lblPlayPause, *ui->btnPlayPause, frameHeight);
    setStandardButtonSize(*ui->lblRewind, *ui->btnRewind, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setStandardButtonSize(*ui->lblTranslateText, *ui->btnTranslateText, frameHeight);
    setStandardButtonSize(*ui->lblCode, *ui->btnCode, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);
}

