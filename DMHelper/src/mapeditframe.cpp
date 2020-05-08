#include "mapeditframe.h"
#include "ui_mapeditframe.h"

MapEditFrame::MapEditFrame(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::MapEditFrame)
{
    ui->setupUi(this);

    connect(ui->btnZoomIn, SIGNAL(clicked()), this, SIGNAL(zoomInClicked()));
    connect(ui->btnZoomOut, SIGNAL(clicked()), this, SIGNAL(zoomOutClicked()));
    connect(ui->btnZoomOne, SIGNAL(clicked()), this, SIGNAL(zoomOneClicked()));
    connect(ui->btnZoomFull, SIGNAL(clicked()), this, SIGNAL(zoomFullClicked()));
    connect(ui->btnZoomSelect, SIGNAL(clicked(bool)), this, SIGNAL(zoomSelectClicked(bool)));

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

    connect(ui->btnPublishVisible, SIGNAL(clicked(bool)), this, SLOT(publishModeVisibleClicked()));
    connect(ui->btnPublishZoom, SIGNAL(clicked(bool)), this, SLOT(publishModeZoomClicked()));

    setEraseMode();
}

MapEditFrame::~MapEditFrame()
{
    delete ui;
}

PublishButtonRibbon* MapEditFrame::getPublishRibbon()
{
    return ui->framePublish;
}

void MapEditFrame::setZoomSelect(bool checked)
{
    ui->btnZoomSelect->setChecked(checked);
}

void MapEditFrame::setBrushMode(int brushMode)
{
    QAbstractButton* button = ui->btnGrpBrush->button(brushMode);
    if(button)
        button->click();
}

void MapEditFrame::setDrawErase(bool checked)
{
    ui->btnFoWErase->setChecked(checked);
    setEraseMode();
}

void MapEditFrame::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    setStandardButtonSize(*ui->lblZoomIn, *ui->btnZoomIn);
    setStandardButtonSize(*ui->lblZoomOut, *ui->btnZoomOut);
    setStandardButtonSize(*ui->lblZoomOne, *ui->btnZoomOne);
    setStandardButtonSize(*ui->lblZoomFull, *ui->btnZoomFull);
    setStandardButtonSize(*ui->lblZoomSelect, *ui->btnZoomSelect);
    setLineHeight(*ui->line_6);
    setStandardButtonSize(*ui->lblFoWErase, *ui->btnFoWErase);
    setStandardButtonSize(*ui->lblSmooth, *ui->btnSmooth);

    // Brush cluster
    int labelHeight = getLabelHeight(*ui->lblSmooth);
    int iconDim = height() - labelHeight;
    QFontMetrics metrics = ui->lblSmooth->fontMetrics();
    int textWidth = metrics.maxWidth();

    setButtonSize(*ui->btnBrushCircle, iconDim / 2, iconDim / 2);
    setButtonSize(*ui->btnBrushSquare, iconDim / 2, iconDim / 2);
    setButtonSize(*ui->btnBrushSelect, iconDim / 2, iconDim / 2);
    int sizeWidth = metrics.horizontalAdvance(ui->lblSize->text());
    setWidgetSize(*ui->lblSize, sizeWidth, iconDim / 2);
    setWidgetSize(*ui->spinSize, sizeWidth, iconDim / 2);
    setWidgetSize(*ui->lblBrush, qMax((textWidth * 4), 2 * sizeWidth), labelHeight);

    setStandardButtonSize(*ui->lblFillFoW, *ui->btnFillFoW);
    setLineHeight(*ui->line_5);
    setStandardButtonSize(*ui->lblPublishVisible, *ui->btnPublishVisible);
    setStandardButtonSize(*ui->lblPublishZoom, *ui->btnPublishZoom);
}

void MapEditFrame::setEraseMode()
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

void MapEditFrame::publishModeVisibleClicked()
{
    if((ui->btnPublishVisible->isChecked()) &&
       (ui->btnPublishZoom->isChecked()))
    {
        ui->btnPublishZoom->click();
    }
}

void MapEditFrame::publishModeZoomClicked()
{
    if((ui->btnPublishVisible->isChecked()) &&
       (ui->btnPublishZoom->isChecked()))
    {
        ui->btnPublishVisible->click();
    }
}
