#include "ribbontabbattlemap.h"
#include "ui_ribbontabbattlemap.h"
#include "dmconstants.h"
#include "grid.h"
#include <QMenu>

const int GRID_SIZE_PAUSE_TIMER = 750;

RibbonTabBattleMap::RibbonTabBattleMap(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabBattleMap),
    _menu(new QMenu(this)),
    _timerId(0),
    _lastGridScale(1),
    _lastGridAngle(1)
{
    ui->setupUi(this);

    connect(ui->btnNewMap, SIGNAL(clicked(bool)), this, SIGNAL(newMapClicked()));
    connect(ui->btnReloadMap, SIGNAL(clicked(bool)), this, SIGNAL(reloadMapClicked()));

    connect(ui->btnGrid, SIGNAL(toggled(bool)), this, SIGNAL(gridClicked(bool)));
    ui->btnGrid->setMenu(_menu);

    RibbonTabBattleMap_GridAction* gridAction = new RibbonTabBattleMap_GridAction(Grid::GridType_Square, QString(":/img/data/icon_grid.png"), QString("Square Grid"));
    _menu->addAction(gridAction);
    RibbonTabBattleMap_GridAction* gridIsoAction = new RibbonTabBattleMap_GridAction(Grid::GridType_Isosquare, QString(":/img/data/icon_gridiso.png"), QString("Isometric Grid"));
    _menu->addAction(gridIsoAction);
    RibbonTabBattleMap_GridAction* gridHexAction = new RibbonTabBattleMap_GridAction(Grid::GridType_Hex, QString(":/img/data/icon_gridhex.png"), QString("Hex Grid"));
    _menu->addAction(gridHexAction);
    RibbonTabBattleMap_GridAction* gridHexIsoAction = new RibbonTabBattleMap_GridAction(Grid::GridType_Isohex, QString(":/img/data/icon_gridhexiso.png"), QString("Isometric Hex Grid"));
    _menu->addAction(gridHexIsoAction);
    selectAction(gridAction);
    connect(_menu, &QMenu::triggered, this, &RibbonTabBattleMap::selectAction);

    connect(ui->spinGridScale, SIGNAL(valueChanged(int)), this, SLOT(spinChanged(int)));
    connect(ui->spinGridAngle, SIGNAL(valueChanged(int)), this, SLOT(spinChanged(int)));
    connect(ui->btnGridCount, &QAbstractButton::clicked, this, &RibbonTabBattleMap::gridScaleSetClicked);
    connect(ui->sliderX, SIGNAL(valueChanged(int)), this, SIGNAL(gridXOffsetChanged(int)));
    connect(ui->sliderY, SIGNAL(valueChanged(int)), this, SIGNAL(gridYOffsetChanged(int)));

    ui->btnGridColor->setRotationVisible(false);
    ui->btnGridColor->setColor(Qt::black);
    connect(ui->btnGridColor, &ColorPushButton::colorChanged, this, &RibbonTabBattleMap::gridColorChanged);
    connect(ui->spinGridWidth, SIGNAL(valueChanged(int)), this, SIGNAL(gridWidthChanged(int)));

    connect(ui->btnMapEdit, SIGNAL(clicked(bool)), this, SIGNAL(editFoWClicked(bool)));
    connect(ui->btnFoWErase, SIGNAL(clicked(bool)), this, SIGNAL(drawEraseClicked(bool)));
    connect(ui->btnSmooth, SIGNAL(clicked(bool)), this, SIGNAL(smoothClicked(bool)));
    connect(ui->btnBrushCircle, SIGNAL(clicked(bool)), this, SIGNAL(brushCircleClicked()));
    connect(ui->btnBrushSquare, SIGNAL(clicked(bool)), this, SIGNAL(brushSquareClicked()));
    connect(ui->spinSize, SIGNAL(valueChanged(int)), this, SIGNAL(brushSizeChanged(int)));
    connect(ui->btnBrushSelect, SIGNAL(clicked(bool)), this, SIGNAL(selectFoWClicked(bool)));
    connect(ui->btnFillFoW, SIGNAL(clicked(bool)), this, SIGNAL(fillFoWClicked()));

    // Set up the brush mode button group
    ui->btnGrpBrush->setId(ui->btnBrushCircle, DMHelper::BrushType_Circle);
    ui->btnGrpBrush->setId(ui->btnBrushSquare, DMHelper::BrushType_Square);
    connect(ui->btnGrpBrush, SIGNAL(buttonClicked(int)), this, SIGNAL(brushModeChanged(int)));

    // Set up the extra slot to configure the erase button
    connect(ui->btnFoWErase, SIGNAL(clicked(bool)), this, SLOT(setEraseMode()));

    setEraseMode();
}

RibbonTabBattleMap::~RibbonTabBattleMap()
{
    if(_timerId)
        killTimer(_timerId);

    delete ui;
}

PublishButtonRibbon* RibbonTabBattleMap::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabBattleMap::setGridOn(bool checked)
{
    ui->btnGrid->setChecked(checked);
}

void RibbonTabBattleMap::setGridType(int gridType)
{
    if((gridType < Grid::GridType_Square) || (gridType > Grid::GridType_Isohex))
        return;

    QList<QAction*> actionList = _menu->actions();
    for(QAction* action : actionList)
    {
        RibbonTabBattleMap_GridAction* gridAction = dynamic_cast<RibbonTabBattleMap_GridAction*>(action);
        if((gridAction) && (gridAction->getGridType() == gridType))
        {
            selectAction(gridAction);
            return;
        }
    }
}

void RibbonTabBattleMap::setGridScale(int scale)
{
    ui->spinGridScale->setValue(scale);
    _lastGridScale = scale;
}

void RibbonTabBattleMap::setGridAngle(int angle)
{
    ui->spinGridAngle->setValue(angle);
    _lastGridAngle = angle;
}

void RibbonTabBattleMap::setGridXOffset(int offset)
{
    ui->sliderX->setValue(offset);
}

void RibbonTabBattleMap::setGridYOffset(int offset)
{
    ui->sliderY->setValue(offset);
}

void RibbonTabBattleMap::setGridWidth(int gridWidth)
{
    ui->spinGridWidth->setValue(gridWidth);
}

void RibbonTabBattleMap::setGridColor(const QColor& gridColor)
{
    ui->btnGridColor->setColor(gridColor);
}

void RibbonTabBattleMap::setEditFoW(bool checked)
{
    ui->btnMapEdit->setChecked(checked);
}

void RibbonTabBattleMap::setDrawErase(bool checked)
{
    ui->btnFoWErase->setChecked(checked);
    setEraseMode();
}

void RibbonTabBattleMap::setSmooth(bool checked)
{
    ui->btnSmooth->setChecked(checked);
}

void RibbonTabBattleMap::setBrushSize(int size)
{
    ui->spinSize->setValue(size);
}

void RibbonTabBattleMap::setSelectFoW(bool checked)
{
    ui->btnBrushSelect->setChecked(checked);
}

void RibbonTabBattleMap::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblNewMap, *ui->btnNewMap, frameHeight);
    setStandardButtonSize(*ui->lblReloadMap, *ui->btnReloadMap, frameHeight);
    setLineHeight(*ui->line_6, frameHeight);

    setStandardButtonSize(*ui->lblGrid, *ui->btnGrid, frameHeight);

    setStandardButtonSize(*ui->lblMapEdit, *ui->btnMapEdit, frameHeight);
    setStandardButtonSize(*ui->lblFoWErase, *ui->btnFoWErase, frameHeight);
    setStandardButtonSize(*ui->lblSmooth, *ui->btnSmooth, frameHeight);

    setStandardButtonSize(*ui->lblBrushSelect, *ui->btnBrushSelect, frameHeight);
    setStandardButtonSize(*ui->lblFillFoW, *ui->btnFillFoW, frameHeight);

    setLineHeight(*ui->line_4, frameHeight);
    setLineHeight(*ui->line_5, frameHeight);

    int labelHeight = getLabelHeight(*ui->lblGrid, frameHeight);
    int iconDim = height() - labelHeight;
    QFontMetrics metrics = ui->lblGrid->fontMetrics();
    int textWidth = metrics.maxWidth();

    // Grid size cluster
    int labelWidth = qMax(metrics.horizontalAdvance(ui->lblGridScale->text()),
                          qMax(metrics.horizontalAdvance(ui->lblSliderX->text()),
                               metrics.horizontalAdvance(ui->lblSliderY->text())));
    int sliderWidth = ui->btnGrid->width() * 3 / 2;
    setWidgetSize(*ui->lblGridScale, labelWidth, height() / 3);
    setWidgetSize(*ui->spinGridScale, sliderWidth, height() / 3);
    setWidgetSize(*ui->btnGridCount, height() / 3, height() / 3);
    setWidgetSize(*ui->lblGridAngle, labelWidth, height() / 3);
    ui->spacerAngle->changeSize(height() / 3, height() / 3, QSizePolicy::Fixed, QSizePolicy::Fixed);
    setWidgetSize(*ui->spinGridAngle, sliderWidth, height() / 3);
    setWidgetSize(*ui->lblSliderX, labelWidth, height() / 3);
    setWidgetSize(*ui->sliderX, sliderWidth, height() / 3);
    setWidgetSize(*ui->lblSliderY, labelWidth, height() / 3);
    setWidgetSize(*ui->sliderY, sliderWidth, height() / 3);

    setStandardButtonSize(*ui->lblGridColor, *ui->btnGridColor, frameHeight);
    int squareButtonSize = qMin(ui->btnGridColor->width(), iconDim);
    int colorButtonSize = squareButtonSize * 3 / 4;

    setWidgetSize(*ui->btnGridColor, colorButtonSize, colorButtonSize);
    setWidgetSize(*ui->spinGridWidth, iconDim, iconDim);
    setLabelHeight(*ui->lblGridWidth, frameHeight);

    // Brush cluster
    setButtonSize(*ui->btnBrushCircle, iconDim / 2, iconDim / 2);
    setButtonSize(*ui->btnBrushSquare, iconDim / 2, iconDim / 2);
    int sizeWidth = metrics.horizontalAdvance(ui->lblSize->text());
    setWidgetSize(*ui->lblSize, sizeWidth, iconDim / 2);
    setWidgetSize(*ui->spinSize, sizeWidth*3, iconDim / 2);
    setWidgetSize(*ui->lblBrush, qMax((textWidth * 4), 2 * sizeWidth), labelHeight);
}

void RibbonTabBattleMap::timerEvent(QTimerEvent *event)
{
    if((!event) && (event->timerId() != _timerId))
        return;

    killTimer(_timerId);

    if(ui->spinGridScale->value() != _lastGridScale)
    {
        _lastGridScale = ui->spinGridScale->value();
        emit gridScaleChanged(_lastGridScale);
    }

    if(ui->spinGridAngle->value() != _lastGridAngle)
    {
        _lastGridAngle = ui->spinGridAngle->value();
        emit gridAngleChanged(_lastGridAngle);
    }

    _timerId = 0;
}

void RibbonTabBattleMap::setEraseMode()
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

void RibbonTabBattleMap::spinChanged(int value)
{
    Q_UNUSED(value);

    // Need to buffer changes here for a second
    if(_timerId != 0)
        killTimer(_timerId);

    _timerId = startTimer(GRID_SIZE_PAUSE_TIMER);
}

void RibbonTabBattleMap::selectAction(QAction* action)
{
    if(!action)
        return;

    RibbonTabBattleMap_GridAction* gridAction = dynamic_cast<RibbonTabBattleMap_GridAction*>(action);
    if(!gridAction)
        return;

    setGridButtonIcon(action->icon());
    ui->spinGridAngle->setEnabled((gridAction->getGridType() == Grid::GridType_Isosquare) || (gridAction->getGridType() == Grid::GridType_Isohex));
    emit gridTypeChanged(gridAction->getGridType());
}

void RibbonTabBattleMap::setGridButtonIcon(const QIcon &icon)
{
    ui->btnGrid->setIcon(icon);
}
