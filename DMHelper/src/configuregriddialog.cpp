#include "configuregriddialog.h"
#include "ui_configuregriddialog.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include "grid.h"
#include <QGraphicsScene>
#include <QScreen>
#include <QDebug>

ConfigureGridDialog::ConfigureGridDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureGridDialog),
    _model(nullptr),
    _grid(nullptr),
    _scene(nullptr),
    _gridScale(10.0)
{
    ui->setupUi(this);

    connect(ui->btnFullscreen, &QAbstractButton::clicked, this, &ConfigureGridDialog::toggleFullscreen);

    ui->spinGridSize->setValue(DMHelper::STARTING_GRID_SCALE);
    connect(ui->spinGridSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigureGridDialog::gridScaleChanged);
    connect(ui->btnAutoFit, &QAbstractButton::clicked, this, &ConfigureGridDialog::autoFit);

    _scene = new QGraphicsScene();
    ui->graphicsView->setScene(_scene);

    // Requred model contents for a Grid as follows:
    //      model.getGridPen() - default black, 1 pixel
    //      model.getGridType() - default Square
    //      model.getGridScale() - default START_GRID_SCALE
    //      model.getGridOffsetX() - default 0
    //      model.getGridOffsetY() - default 0
    //      model.getGridAngle() - default 0
    _model = new BattleDialogModel(QString(), this);

    _grid = new Grid(*_scene, QRect());
}

ConfigureGridDialog::~ConfigureGridDialog()
{
    delete ui;
}

qreal ConfigureGridDialog::getGridScale()
{
    return _gridScale;
}

void ConfigureGridDialog::setGridScale(qreal gridScale)
{
    if((gridScale > 0.0) && (ui->spinGridSize->value() != static_cast<int>(gridScale)))
        ui->spinGridSize->setValue(gridScale);
}

void ConfigureGridDialog::resizeEvent(QResizeEvent* event)
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

void ConfigureGridDialog::toggleFullscreen()
{
    if(ui->btnFullscreen->isChecked())
        setWindowState(windowState() | Qt::WindowFullScreen);
    else
        setWindowState(windowState() & ~Qt::WindowFullScreen);
}

void ConfigureGridDialog::gridScaleChanged(int value)
{
    if(_model)
    {
        _model->setGridScale(value);
        rebuildGrid();
    }
}

void ConfigureGridDialog::autoFit()
{
    QScreen* thisScreen = screen();
    if(!thisScreen)
        return;

    QSize screenSize = thisScreen->size();
    QSizeF physicalSize = thisScreen->physicalSize();

    qDebug() << "[ConfigureGridDialog] Autofitting, screen size: " << screenSize << ", physical size: " << physicalSize;
    if(physicalSize.width() <= 0.0)
        return;

    qreal ppi = static_cast<qreal>(screenSize.width()) / (physicalSize.width() / 25.4);
    qDebug() << "[ConfigureGridDialog] Autofitting, PPI: " << ppi << ", graphics view width: " << ui->graphicsView->width();

    if((ppi > 0.0) && (ui->graphicsView->width() > 0.0))
        ui->spinGridSize->setValue(ppi);
}

void ConfigureGridDialog::rebuildGrid()
{
    if((!_grid) || (!_model))
        return;

    _grid->rebuildGrid(*_model);
    if(ui->graphicsView->width() > 0)
        _gridScale = ui->spinGridSize->value();
}
