#include "layergrid.h"
#include "grid.h"
#include "gridconfig.h"
#include "publishglbattlegrid.h"
#include <QGraphicsScene>
#include <QImage>
#include <QDebug>

LayerGrid::LayerGrid(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    _grid(nullptr),
    _gridGLObject(nullptr),
    _scene(nullptr),
    _config()
{
    connect(&_config, &GridConfig::dirty, this, &LayerGrid::triggerRebuild);
}

LayerGrid::~LayerGrid()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerGrid::inputXML(const QDomElement &element, bool isImport)
{
    _config.inputXML(element, isImport);
    Layer::inputXML(element, isImport);
}

QImage LayerGrid::getLayerIcon() const
{
    return QImage(":/img/data/icon_grid.png");
}

bool LayerGrid::defaultShader() const
{
    return false;
}

DMHelper::LayerType LayerGrid::getType() const
{
    return DMHelper::LayerType_Grid;
}

Layer* LayerGrid::clone() const
{
    LayerGrid* newLayer = new LayerGrid(_name, _order);

    copyBaseValues(newLayer);
    newLayer->_config.copyValues(_config);

    return newLayer;
}

void LayerGrid::applyOrder(int order)
{
    if(_grid)
        _grid->setGridZValue(order);
}

void LayerGrid::applyLayerVisibleDM(bool layerVisible)
{
    if(_grid)
        _grid->setGridVisible(layerVisible);
}

void LayerGrid::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerGrid::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;

    if(_grid)
        _grid->setGridOpacity(opacity);

    if(_gridGLObject)
        _gridGLObject->setOpacity(opacity);
}

void LayerGrid::applyPosition(const QPoint& position)
{
    if(_grid)
        _grid->setGridPosition(position);

    if(_gridGLObject)
    {
        QPoint pointTopLeft = _scene ? _scene->getSceneRect().toRect().topLeft() : QPoint();
        _gridGLObject->setPosition(QPoint(pointTopLeft.x() + position.x(),
                                          -pointTopLeft.y() - position.y()));
    }
}

void LayerGrid::applySize(const QSize& size)
{
    if(getSize() == size)
        return;

    if(_grid)
        _grid->setGridSize(size);

    if(_gridGLObject)
        _gridGLObject->setSize(size);

    triggerRebuild();
}

GridConfig& LayerGrid::getConfig()
{
    return _config;
}

const GridConfig& LayerGrid::getConfig() const
{
    return _config;
}

void LayerGrid::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    if(_grid)
    {
        qDebug() << "[LayerImage] ERROR: dmInitialize called although the grid item already exists!";
        return;
    }

    _grid = new Grid(scene, QRect(QPoint(0, 0), getSize()));
    _grid->rebuildGrid(_config, getOrder());
    _grid->setFlag(QGraphicsItem::ItemIsMovable, false);
    _grid->setFlag(QGraphicsItem::ItemIsSelectable, false);
    _grid->setZValue(getOrder());
    _grid->setGridOpacity(getOpacity());

    Layer::dmInitialize(scene);
}

void LayerGrid::dmUninitialize()
{
    cleanupDM();
}

void LayerGrid::dmUpdate()
{
}

void LayerGrid::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    if(_gridGLObject)
    {
        qDebug() << "[LayerGrid] ERROR: playerGLInitialize called although the grid object already exists!";
        return;
    }

    _scene = scene;

    _gridGLObject = new PublishGLBattleGrid(_config, getOpacity(), getSize());

    Layer::playerGLInitialize(renderer, scene);
}

void LayerGrid::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerGrid::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if((!functions) || (!projectionMatrix) || (!_gridGLObject))
        return;

    DMH_DEBUG_OPENGL_PAINTGL();

    _gridGLObject->setProjectionMatrix(projectionMatrix);
    _gridGLObject->paintGL(functions, projectionMatrix);
}

void LayerGrid::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

bool LayerGrid::playerIsInitialized()
{
    return _gridGLObject != nullptr;
}

void LayerGrid::initialize(const QSize& sceneSize)
{
    if(_grid)
        return;

    if(getSize().isEmpty())
        setSize(sceneSize);
}

void LayerGrid::uninitialize()
{
}

void LayerGrid::setScale(int scale)
{
    if(_config.getGridScale() == scale)
        return;

    _config.setGridScale(scale);
    triggerRebuild();

    emit layerScaleChanged(this);
}

void LayerGrid::setConfig(const GridConfig& config)
{
    _config.copyValues(config);
    triggerRebuild();
}

void LayerGrid::triggerRebuild()
{
    if(_grid)
    {
        _grid->rebuildGrid(_config, getOrder());
        _grid->setGridVisible(_layerVisibleDM);
    }

    if(_gridGLObject)
        _gridGLObject->setConfig(_config);

    emit dirty();
}

void LayerGrid::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(targetDirectory);

    _config.outputXML(doc, element, isExport);

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerGrid::cleanupDM()
{
    if(!_grid)
        return;

    _grid->clear();
    QGraphicsScene* scene = _grid->scene();
    if(scene)
        scene->removeItem(_grid);

    delete _grid;
    _grid = nullptr;
}

void LayerGrid::cleanupPlayer()
{
    delete _gridGLObject;
    _gridGLObject = nullptr;

    _scene = nullptr;
}
