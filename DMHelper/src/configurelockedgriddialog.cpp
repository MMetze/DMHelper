#include "configurelockedgriddialog.h"
#include "ui_configurelockedgriddialog.h"
#include "dmconstants.h"
#include "grid.h"
#include "gridconfig.h"
#include "gridsizer.h"
#include "ribbontabbattlemap.h"
#include <QGraphicsScene>
#include <QScreen>
#include <QMenu>
#include <QDebug>

const int CONFIGURE_GRID_DEFAULT_ANGLE = 50;

ConfigureLockedGridDialog::ConfigureLockedGridDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureLockedGridDialog),
    _grid(nullptr),
    _gridConfig(nullptr),
    _gridSizer(nullptr),
    _scene(nullptr),
    _gridScale(10.0),
    _menu(new QMenu(this))
{
    ui->setupUi(this);

    _scene = new QGraphicsScene();
    ui->graphicsView->setScene(_scene);

    _gridConfig = new GridConfig(this);

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
    connect(_menu, &QMenu::triggered, this, &ConfigureLockedGridDialog::selectAction);

    connect(ui->btnFullscreen, &QAbstractButton::clicked, this, &ConfigureLockedGridDialog::toggleFullscreen);

    ui->spinGridSize->setValue(DMHelper::STARTING_GRID_SCALE);
    connect(ui->spinGridSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigureLockedGridDialog::gridScaleChanged);
    connect(ui->btnAutoFit, &QAbstractButton::clicked, this, &ConfigureLockedGridDialog::autoFit);

    ui->spinGridAngle->setValue(CONFIGURE_GRID_DEFAULT_ANGLE);
    connect(ui->spinGridAngle, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigureLockedGridDialog::gridAngleChanged);
    _gridConfig->setGridAngle(CONFIGURE_GRID_DEFAULT_ANGLE);

    _grid = new Grid(_scene, QRect());

    _gridSizer = new GridSizer(DMHelper::STARTING_GRID_SCALE);
    _gridSizer->setPenColor(QColor(115, 18, 0));
    _gridSizer->setPenWidth(3);
    _gridSizer->setBackgroundColor(Qt::gray);
    _scene->addItem(_gridSizer);
    _gridSizer->setPos(DMHelper::STARTING_GRID_SCALE, DMHelper::STARTING_GRID_SCALE);

    connect(_scene, &QGraphicsScene::changed, this, &ConfigureLockedGridDialog::gridSizerResized);
}

ConfigureLockedGridDialog::~ConfigureLockedGridDialog()
{
    delete ui;
}

qreal ConfigureLockedGridDialog::getGridScale()
{
    return _gridScale;
}

void ConfigureLockedGridDialog::setGridScale(qreal gridScale)
{
    if((gridScale > 0.0) && (ui->spinGridSize->value() != static_cast<int>(gridScale)))
        ui->spinGridSize->setValue(gridScale);
}

void ConfigureLockedGridDialog::resizeEvent(QResizeEvent* event)
{
    if(_grid)
    {
        QRect sceneRect(QPoint(), ui->graphicsView->size());
        ui->graphicsView->setSceneRect(sceneRect);
        _grid->setGridShape(sceneRect);
        rebuildGrid();
    }

    QDialog::resizeEvent(event);
}

void ConfigureLockedGridDialog::toggleFullscreen()
{
    if(ui->btnFullscreen->isChecked())
        setWindowState(windowState() | Qt::WindowFullScreen);
    else
        setWindowState(windowState() & ~Qt::WindowFullScreen);
}

void ConfigureLockedGridDialog::gridScaleChanged(int value)
{
    if((!_gridConfig) || (!_gridSizer) || (value <= 0) || (value == _gridScale))
        return;

    _gridScale = value;
    _gridSizer->setSize(_gridScale);
    _gridConfig->setGridScale(_gridScale);
    rebuildGrid();
}

void ConfigureLockedGridDialog::gridAngleChanged(int value)
{
    if((!_gridConfig) || (!_gridSizer) || (value <= 0) || (value == _gridConfig->getGridAngle()))
        return;

    _gridConfig->setGridScale(value);
    rebuildGrid();
}

void ConfigureLockedGridDialog::autoFit()
{
    QScreen* thisScreen = screen();
    if(!thisScreen)
        return;

    QSize screenSize = thisScreen->size();
    QSizeF physicalSize = thisScreen->physicalSize();

    qDebug() << "[ConfigureLockedGridDialog] Autofitting, screen size: " << screenSize << ", physical size: " << physicalSize;
    if(physicalSize.width() <= 0.0)
        return;

    qreal ppi = static_cast<qreal>(screenSize.width()) / (physicalSize.width() / 25.4);
    qDebug() << "[ConfigureLockedGridDialog] Autofitting, PPI: " << ppi << ", graphics view width: " << ui->graphicsView->width();

    if((ppi > 0.0) && (ui->graphicsView->width() > 0.0))
        ui->spinGridSize->setValue(ppi);
}

void ConfigureLockedGridDialog::gridSizerResized()
{
    if(!_gridSizer)
        return;

    setGridScale(_gridSizer->getSize());
}

void ConfigureLockedGridDialog::selectAction(QAction* action)
{
    if(!action)
        return;

    RibbonTabBattleMap_GridAction* gridAction = dynamic_cast<RibbonTabBattleMap_GridAction*>(action);
    if(!gridAction)
        return;

    ui->btnGrid->setIcon(action->icon());
    _gridConfig->setGridType(gridAction->getGridType());
    ui->spinGridAngle->setEnabled((gridAction->getGridType() == Grid::GridType_Isosquare) || (gridAction->getGridType() == Grid::GridType_Isohex));
    rebuildGrid();
}

void ConfigureLockedGridDialog::rebuildGrid()
{
    if((!_grid) || (!_gridConfig))
        return;

    _grid->rebuildGrid(*_gridConfig);
    if(ui->graphicsView->width() > 0)
        _gridScale = ui->spinGridSize->value();
}
