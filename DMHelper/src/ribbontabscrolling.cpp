#include "ribbontabscrolling.h"
#include "ui_ribbontabscrolling.h"

RibbonTabScrolling::RibbonTabScrolling(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabScrolling)
{
    ui->setupUi(this);

    // Animation
    connect(ui->btnBackground, SIGNAL(clicked()), this, SIGNAL(backgroundClicked()));
    connect(ui->spinSpeed, SIGNAL(valueChanged(double)), this, SIGNAL(speedChanged(double)));
    connect(ui->sliderWidth, SIGNAL(valueChanged(int)), this, SIGNAL(widthChanged(int)));

    // Text Frame
    connect(ui->frameText, &RibbonFrameText::colorChanged, this, &RibbonTabScrolling::colorChanged);
    connect(ui->frameText, &RibbonFrameText::fontFamilyChanged, this, &RibbonTabScrolling::fontFamilyChanged);
    connect(ui->frameText, &RibbonFrameText::fontSizeChanged, this, &RibbonTabScrolling::fontSizeChanged);
    connect(ui->frameText, &RibbonFrameText::fontBoldChanged, this, &RibbonTabScrolling::fontBoldChanged);
    connect(ui->frameText, &RibbonFrameText::fontItalicsChanged, this, &RibbonTabScrolling::fontItalicsChanged);
    connect(ui->frameText, &RibbonFrameText::alignmentChanged, this, &RibbonTabScrolling::alignmentChanged);
}

RibbonTabScrolling::~RibbonTabScrolling()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabScrolling::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabScrolling::setSpeed(double speed)
{
    if((speed != ui->spinSpeed->value()) &&
       (speed >= ui->spinSpeed->minimum()) &&
       (speed <= ui->spinSpeed->maximum()))
    {
        ui->spinSpeed->setValue(speed);
    }
}

void RibbonTabScrolling::setWidth(int width)
{
    if((width != ui->sliderWidth->value()) &&
       (width >= ui->sliderWidth->minimum()) &&
       (width <= ui->sliderWidth->maximum()))
    {
        ui->sliderWidth->setValue(width);
    }
}

void RibbonTabScrolling::setColor(QColor color)
{
    ui->frameText->setColor(color);
}

void RibbonTabScrolling::setFontFamily(const QString& fontFamily)
{
    ui->frameText->setFontFamily(fontFamily);
}

void RibbonTabScrolling::setFontSize(int fontSize)
{
    ui->frameText->setFontSize(fontSize);
}

void RibbonTabScrolling::setFontBold(bool fontBold)
{
    ui->frameText->setFontBold(fontBold);
}

void RibbonTabScrolling::setFontItalics(bool fontItalics)
{
    ui->frameText->setFontItalics(fontItalics);
}

void RibbonTabScrolling::setAlignment(Qt::Alignment alignment)
{
    ui->frameText->setAlignment(alignment);
}

void RibbonTabScrolling::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    setStandardButtonSize(*ui->lblBackground, *ui->btnBackground);
    setLineHeight(*ui->line_6);

    QFontMetrics metrics = ui->lblBackground->fontMetrics();
    int labelHeight = getLabelHeight(metrics);
    int halfHeight = (height() - labelHeight - ui->layoutAnimation->spacing()) / 2;

    // Animation cluster
    int speedLabelWidth = metrics.horizontalAdvance(ui->lblSpeed->text());
    int sliderLabelWidth = metrics.horizontalAdvance(ui->lblSliderWidth->text());
    int labelWidth = qMax(speedLabelWidth, sliderLabelWidth);
    int spinBoxWidth = metrics.maxWidth() * 4;
    int sliderWidth = ui->btnBackground->width() * 3 / 2;
    int controlWidth = qMax(spinBoxWidth, sliderWidth);
    setWidgetSize(*ui->lblSpeed, labelWidth, halfHeight);
    setWidgetSize(*ui->spinSpeed, controlWidth, halfHeight);
    setWidgetSize(*ui->lblSliderWidth, labelWidth, halfHeight);
    setWidgetSize(*ui->sliderWidth, controlWidth, halfHeight);
    setWidgetSize(*ui->lblAnimation, labelWidth + ui->layoutSlider->spacing() + controlWidth, labelHeight);

    setLineHeight(*ui->line_3);
}
