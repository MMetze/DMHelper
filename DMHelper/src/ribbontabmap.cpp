#include "ribbontabmap.h"
#include "ui_ribbontabmap.h"

RibbonTabMap::RibbonTabMap(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabMap)
{
    ui->setupUi(this);

    connect(ui->btnEditFile, SIGNAL(clicked()), this, SIGNAL(editFileClicked()));

    connect(ui->btnMapEdit, SIGNAL(clicked(bool)), this, SIGNAL(mapEditClicked(bool)));
    connect(ui->btnFoWErase, SIGNAL(clicked(bool)), this, SIGNAL(drawEraseClicked(bool)));
    connect(ui->btnSmooth, SIGNAL(clicked(bool)), this, SIGNAL(smoothClicked(bool)));
    connect(ui->spinSize, SIGNAL(valueChanged(int)), this, SIGNAL(brushSizeChanged(int)));
    connect(ui->btnFillFoW, SIGNAL(clicked(bool)), this, SIGNAL(fillFoWClicked()));

    connect(ui->btnPublishZoom, SIGNAL(clicked(bool)), this, SIGNAL(publishZoomChanged(bool)));
    connect(ui->btnPublishVisible, SIGNAL(clicked(bool)), this, SIGNAL(publishVisibleChanged(bool)));

    // Set up the brush mode button group
    ui->btnGrpBrush->setId(ui->btnBrushCircle, DMHelper::BrushType_Circle);
    ui->btnGrpBrush->setId(ui->btnBrushSquare, DMHelper::BrushType_Square);
    ui->btnGrpBrush->setId(ui->btnBrushSelect, DMHelper::BrushType_Select);
    connect(ui->btnGrpBrush, SIGNAL(buttonClicked(int)), this, SIGNAL(brushModeChanged(int)));

    // Set up the extra slot to configure the erase button
    connect(ui->btnFoWErase, SIGNAL(clicked(bool)), this, SLOT(setEraseMode()));

    connect(ui->btnColorize, SIGNAL(clicked(bool)), this, SIGNAL(colorizeClicked()));

    connect(ui->btnPublishVisible, SIGNAL(clicked(bool)), this, SLOT(publishModeVisibleClicked()));
    connect(ui->btnPublishZoom, SIGNAL(clicked(bool)), this, SLOT(publishModeZoomClicked()));

    setEraseMode();
}

RibbonTabMap::~RibbonTabMap()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabMap::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabMap::setMapEdit(bool checked)
{
    ui->btnMapEdit->setChecked(checked);
}

void RibbonTabMap::setBrushMode(int brushMode)
{
    QAbstractButton* button = ui->btnGrpBrush->button(brushMode);
    if(button)
        button->click();
}

void RibbonTabMap::setDrawErase(bool checked)
{
    ui->btnFoWErase->setChecked(checked);
    setEraseMode();
}

void RibbonTabMap::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblEditFile, *ui->btnEditFile, frameHeight);
    setLineHeight(*ui->line_7, frameHeight);
    setStandardButtonSize(*ui->lblMapEdit, *ui->btnMapEdit, frameHeight);
    setStandardButtonSize(*ui->lblFoWErase, *ui->btnFoWErase, frameHeight);
    setStandardButtonSize(*ui->lblSmooth, *ui->btnSmooth, frameHeight);

    // Brush cluster
    int labelHeight = getLabelHeight(*ui->lblSmooth, frameHeight);
    int iconDim = height() - labelHeight;
    QFontMetrics metrics = ui->lblSmooth->fontMetrics();
    int textWidth = metrics.maxWidth();

    setButtonSize(*ui->btnBrushCircle, iconDim / 2, iconDim / 2);
    setButtonSize(*ui->btnBrushSquare, iconDim / 2, iconDim / 2);
    setButtonSize(*ui->btnBrushSelect, iconDim / 2, iconDim / 2);
    int sizeWidth = metrics.horizontalAdvance(ui->lblSize->text());
    setWidgetSize(*ui->lblSize, sizeWidth, iconDim / 2);
    setWidgetSize(*ui->spinSize, sizeWidth*3, iconDim / 2);
    setWidgetSize(*ui->lblBrush, qMax((textWidth * 4), 2 * sizeWidth), labelHeight);

    setStandardButtonSize(*ui->lblFillFoW, *ui->btnFillFoW, frameHeight);
    setLineHeight(*ui->line_5, frameHeight);

    setStandardButtonSize(*ui->lblColorize, *ui->btnColorize, frameHeight);

    setLineHeight(*ui->line_8, frameHeight);

    setStandardButtonSize(*ui->lblPublishVisible, *ui->btnPublishVisible, frameHeight);
    setStandardButtonSize(*ui->lblPublishZoom, *ui->btnPublishZoom, frameHeight);
}

void RibbonTabMap::setEraseMode()
{
    if(ui->btnFoWErase->isChecked())
    {
        ui->btnFillFoW->setIcon(QPixmap(":/img/data/icon_square.png"));
        ui->lblFillFoW->setText(QString("Clear"));
    }
    else
    {
        ui->btnFillFoW->setIcon(QPixmap(":/img/data/square.png"));
        ui->lblFillFoW->setText(QString("Fill"));
    }
}

void RibbonTabMap::publishModeVisibleClicked()
{
    if((ui->btnPublishVisible->isChecked()) &&
       (ui->btnPublishZoom->isChecked()))
    {
        ui->btnPublishZoom->click();
    }
}

void RibbonTabMap::publishModeZoomClicked()
{
    if((ui->btnPublishVisible->isChecked()) &&
       (ui->btnPublishZoom->isChecked()))
    {
        ui->btnPublishVisible->click();
    }
}
