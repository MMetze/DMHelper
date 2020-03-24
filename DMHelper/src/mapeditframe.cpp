#include "mapeditframe.h"
#include "ui_mapeditframe.h"

MapEditFrame::MapEditFrame(QWidget *parent) :
    QFrame(parent),
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

PublishButtonRibbon* MapEditFrame::getPublishRibbon() const
{
    return ui->publishFrame;
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
