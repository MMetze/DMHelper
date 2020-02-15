#include "mapeditframe.h"
#include "ui_mapeditframe.h"

MapEditFrame::MapEditFrame(DMHelper::BrushType brushType, int brushSize, bool gridVisible, int gridScale, int gridOffsetX, int gridOffsetY, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MapEditFrame)
{
    ui->setupUi(this);

    setBrushType(brushType);
    setBrushSize(brushSize);
    setGridVisible(gridVisible);
    setGridScale(gridScale);
    setGridOffsetX(gridOffsetX);
    setGridOffsetY(gridOffsetY);

    // Set up the brush mode button group
    ui->grpBrush->setId(ui->btnBrushCircle, DMHelper::BrushType_Circle);
    ui->grpBrush->setId(ui->btnBrushSquare, DMHelper::BrushType_Square);
    ui->grpBrush->setId(ui->btnBrushSelect, DMHelper::BrushType_Select);
    connect(ui->grpBrush,SIGNAL(buttonClicked(int)),this,SLOT(setMapCursor()));

    connect(ui->btnClearFoW,SIGNAL(clicked()),this,SIGNAL(clearFoW()));
    connect(ui->btnResetFoW,SIGNAL(clicked()),this,SIGNAL(resetFoW()));

//    connect(ui->spinBox,SIGNAL(valueChanged(int)),this,SLOT(setMapCursor()));

    connect(ui->chkShowGrid, SIGNAL(clicked(bool)), this, SLOT(setGridVisible(bool)));

    connect(ui->spinGridScale, SIGNAL(valueChanged(int)), this, SLOT(setGridScale(int)));
//    connect(ui->sliderX, SIGNAL(valueChanged(int)), this, SLOT(setXOffset(int)));
//    connect(ui->sliderY, SIGNAL(valueChanged(int)), this, SLOT(setYOffset(int)));


}

MapEditFrame::~MapEditFrame()
{
    delete ui;
}

DMHelper::BrushType MapEditFrame::getBrushType() const
{
    int checkedId = ui->grpBrush->checkedId();
    if((checkedId < 0) || (checkedId >= DMHelper::BrushType_Count))
        return DMHelper::BrushType_Circle;
    else
        return static_cast<DMHelper::BrushType>(checkedId);
}

int MapEditFrame::getBrushSize() const
{
    return ui->spinBrushSize->value();
}

bool MapEditFrame::isGridVisible() const
{
    return ui->chkShowGrid->isChecked();
}

int MapEditFrame::getGridScale() const
{
    return ui->spinGridScale->value();
}

int MapEditFrame::getGridOffsetX() const
{
    return ui->gridOffsetX->value();
}

int MapEditFrame::getGridOffsetY() const
{
    return ui->gridOffsetY->value();
}

void MapEditFrame::setBrushType(DMHelper::BrushType brushType)
{
    QPushButton* brushTypeButton = nullptr;

    switch(brushType)
    {
        case DMHelper::BrushType_Circle:
            brushTypeButton = ui->btnBrushCircle;
            break;
        case DMHelper::BrushType_Square:
            brushTypeButton = ui->btnBrushSquare;
            break;
        case DMHelper::BrushType_Select:
            brushTypeButton = ui->btnBrushSelect;
            break;
        default:
            return;
    }

    if((!brushTypeButton) || (brushTypeButton->isChecked()))
        return;

    brushTypeButton->setChecked(true);
    emit brushTypeChanged(brushType);
    return;
}

void MapEditFrame::setBrushSize(int brushSize)
{
    if(brushSize == ui->spinBrushSize->value())
        return;

    ui->spinBrushSize->setValue(brushSize);
    emit brushSizeChanged(brushSize);
}

void MapEditFrame::setGridVisible(bool gridVisible)
{
    if(gridVisible == ui->chkShowGrid->isChecked())
        return;

    ui->chkShowGrid->setChecked(gridVisible);
    emit gridVisibleChanged(gridVisible);
}

void MapEditFrame::setGridScale(int gridScale)
{
    if(gridScale == ui->spinGridScale->value())
        return;

    ui->spinGridScale->setValue(gridScale);
    emit gridScaleChanged(gridScale);
}

void MapEditFrame::setGridOffsetX(int gridOffsetX)
{
    if(gridOffsetX == ui->gridOffsetX->value())
        return;

    ui->gridOffsetX->setValue(gridOffsetX);
    emit gridOffsetXChanged(gridOffsetX);
}

void MapEditFrame::setGridOffsetY(int gridOffsetY)
{
    if(gridOffsetY == ui->gridOffsetY->value())
        return;

    ui->gridOffsetY->setValue(gridOffsetY);
    emit gridOffsetYChanged(gridOffsetY);
}
