#include "mapframe.h"
#include "ui_mapframe.h"
#include "mapframescene.h"
#include "dmconstants.h"
#include "map.h"
#include "mapmarkergraphicsitem.h"
#include "undofowfill.h"
#include "undofowshape.h"
#include "undomarker.h"
#include "layerscene.h"
#include "layervideo.h"
#include "layergrid.h"
#include "mapmarkerdialog.h"
#include "mapcolorizedialog.h"
#include "layerseditdialog.h"
#include "layerfow.h"
#include "party.h"
#include "unselectedpixmap.h"
#include "camerarect.h"
#include "publishglmaprenderer.h"
#include "gridsizer.h"
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QMutexLocker>
#include <QFileDialog>
#include <QStyleOptionGraphicsItem>
#include <QMessageBox>
#include <QDebug>

MapFrame::MapFrame(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::MapFrame),
    _scene(nullptr),
    _partyIcon(nullptr),
    _cameraRect(nullptr),
    _editMode(-1),
    _erase(true),
    _smooth(true),
    _brushMode(DMHelper::BrushType_Circle),
    _brushSize(30),
    _isPublishing(false),
    _isVideo(false),
    _rotation(0),
    _spaceDown(false),
    _mapMoveMouseDown(false),
    _mouseDown(false),
    _mouseDownPos(),
    _undoPath(nullptr),
    _distanceLine(nullptr),
    _mapItem(nullptr),
    _distancePath(nullptr),
    _distanceText(nullptr),
    _pointerFile(),
    _publishMouseDown(false),
    _publishMouseDownPos(),
    _rubberBand(nullptr),
    _scale(1.0),
    _gridSizer(nullptr),
    _mapSource(nullptr),
    _renderer(nullptr),
    _targetSize(),
    _targetLabelSize(),
    _publishRect(),
    _bwFoWImage(),
    _activatedId()
{
    ui->setupUi(this);

    ui->graphicsView->viewport()->installEventFilter(this);
    ui->graphicsView->installEventFilter(this);
    ui->graphicsView->setStyleSheet("background-color: transparent;");

    connect(this, &MapFrame::dirty, this, &MapFrame::checkPartyUpdate);

    editModeToggled(DMHelper::EditMode_Move);
}

MapFrame::~MapFrame()
{
    delete ui;
}

void MapFrame::activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer)
{
    Map* map = dynamic_cast<Map*>(object);
    if(!map)
        return;

    if(_mapSource != nullptr)
    {
        qDebug() << "[MapFrame] ERROR: New map object activated without deactivating the existing map object first!";
        deactivateObject();
    }

    setMap(map);
    connect(this, SIGNAL(dirty()), _mapSource, SIGNAL(dirty()));

    rendererActivated(dynamic_cast<PublishGLMapRenderer*>(currentRenderer));

    _isPublishing = (currentRenderer) && (_mapSource) && (currentRenderer->getObject() == _mapSource);
    if(_cameraRect)
        _cameraRect->setPublishing(_isPublishing);

    emit checkableChanged(_isVideo);
    emit setPublishEnabled(true, true);
    emit backgroundColorChanged(_mapSource->getBackgroundColor());
    emit setLayers(_mapSource->getLayerScene().getLayers(), _mapSource->getLayerScene().getSelectedLayerIndex());
}

void MapFrame::deactivateObject()
{
    if(!_mapSource)
    {
        qDebug() << "[MapFrame] WARNING: Invalid (nullptr) map object deactivated!";
        return;
    }

    if(_partyIcon)
        _mapSource->setPartyIconPos(_partyIcon->pos().toPoint());

    rendererDeactivated();
    cancelSelect();

    disconnect(this, SIGNAL(dirty()), _mapSource, SIGNAL(dirty()));

#if defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)
    _mapSource->uninitialize();
#endif
    setMap(nullptr);

    emit setLayers(QList<Layer*>(), 0);

    _spaceDown = false;
}

void MapFrame::setMap(Map* map)
{
    if(_mapSource)
    {
        editModeToggled(DMHelper::EditMode_Move);
        uninitializeMap();
    }

    _mapSource = map;
    if(!_mapSource)
        return;

    initializeMap();
    setMapCursor();

    emit
}

void MapFrame::mapMarkerMoved(UndoMarker* marker)
{
    if((!_mapSource) || (!marker))
        return;

    if(marker->getMarkerItem())
        marker->getMarker().setPosition(marker->getMarkerItem()->pos().toPoint());

    emit dirty();
    emit markerChanged();
}

void MapFrame::activateMapMarker(UndoMarker* marker)
{
    if((!_mapSource) || (!marker))
        return;

    if(marker->getMarker().getEncounter().isNull())
        return;

    _activatedId = marker->getMarker().getEncounter();
    QTimer::singleShot(0, this, SLOT(handleActivateMapMarker()));
}

bool MapFrame::eventFilter(QObject *obj, QEvent *event)
{
    if((event) && event->type() == QEvent::Wheel)
    {
        QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>(event);
        if((wheelEvent) && ((wheelEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) && (wheelEvent->angleDelta().y() != 0))
        {
            if(wheelEvent->angleDelta().y() > 0)
                zoomIn();
            else
                zoomOut();

            wheelEvent->accept();
            return true;
        }
    }
    else if(checkMapMove(event))
    {
        setMapCursor();
        return true;
    }
    else
    {
        if(execEventFilter(obj, event))
            return true;
    }

    return CampaignObjectFrame::eventFilter(obj, event);
}

QAction* MapFrame::getUndoAction(QObject* parent)
{
    Q_UNUSED(parent);
    // TODO: layers
    //return _mapSource->getUndoStack()->createUndoAction(parent);
    return nullptr;
}

QAction* MapFrame::getRedoAction(QObject* parent)
{
    Q_UNUSED(parent);
    // TODO: layers
    //return _mapSource->getUndoStack()->createRedoAction(parent);
    return nullptr;
}

void MapFrame::fillFoW()
{
    if(_erase)
        clearFoW();
    else
        resetFoW();
}

void MapFrame::resetFoW()
{
    if(!_mapSource)
        return;

    if(QMessageBox::question(nullptr, QString("Confirm Fill FoW"), QString("Are you sure you would like to fill the entire Fog of War?")) == QMessageBox::No)
        return;

    // TODO: layers
    LayerFow* layer = dynamic_cast<LayerFow*>(_mapSource->getLayerScene().getPriority(DMHelper::LayerType_Fow));
    if(layer)
    {
        UndoFowFill* undoFill = new UndoFowFill(layer, MapEditFill(QColor(128, 0, 0, 255)));
        layer->getUndoStack()->push(undoFill);
        emit dirty();
    }
}

void MapFrame::clearFoW()
{
    if(!_mapSource)
        return;

    if(QMessageBox::question(nullptr, QString("Confirm Clear FoW"), QString("Are you sure you would like to clear the entire Fog of War?")) == QMessageBox::No)
        return;

    // TODO: layers
    LayerFow* layer = dynamic_cast<LayerFow*>(_mapSource->getLayerScene().getPriority(DMHelper::LayerType_Fow));
    if(layer)
    {
        UndoFowFill* undoFill = new UndoFowFill(layer, MapEditFill(QColor(128, 0, 0, 0)));
        layer->getUndoStack()->push(undoFill);
        emit dirty();
    }
}

void MapFrame::undoPaint()
{
    if(!_mapSource)
        return;

    // TODO: layers
    //_mapSource->applyPaintTo(nullptr, QColor(0, 0, 0, 128), _mapSource->getUndoStack()->index() - 1)
    // updateFoW();
}

void MapFrame::colorize()
{
    if(!_mapSource)
        return;

    MapColorizeDialog dlg(_mapSource->getUnfilteredBackgroundImage(), _mapSource->getFilter());
    dlg.resize(width() / 2, height() / 2);
    if(dlg.exec() == QDialog::Accepted)
    {
        _mapSource->setFilter(dlg.getFilter());
        _mapSource->setApplyFilter(dlg.getFilter().isValid());
    }
}

void MapFrame::setParty(Party* party)
{
    if(!_mapSource)
        return;

    _mapSource->setParty(party);
}

void MapFrame::setPartyIcon(const QString& partyIcon)
{
    if(!_mapSource)
        return;

    _mapSource->setPartyIcon(partyIcon);
}

void MapFrame::setShowParty(bool showParty)
{
    if(!_mapSource)
        return;

    _mapSource->setShowParty(showParty);
}

void MapFrame::setPartyScale(int partyScale)
{
    if(!_mapSource)
        return;

    _mapSource->setPartyScale(partyScale);
}

void MapFrame::setPartySelected(bool selected)
{
    if(_partyIcon)
        _partyIcon->setSelected(selected);
}

void MapFrame::resizeGrid()
{
    if((!_scene) || (!_mapSource) || (_gridSizer))
        return;

    // Add a resizeable grid setter with a 5x5 grid to the battle frame
    qreal currentScale = DMHelper::STARTING_GRID_SCALE;
    LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(_mapSource->getLayerScene().getNearest(_mapSource->getLayerScene().getSelectedLayer(), DMHelper::LayerType_Grid));
    if(gridLayer)
        currentScale = gridLayer->getConfig().getGridScale();
    else
        currentScale = _mapSource->getLayerScene().getScale();

    _gridSizer = new GridSizer(currentScale);
    _gridSizer->setBackgroundColor(QColor(255,255,255,204));
    _scene->addItem(_gridSizer);
    _gridSizer->setPos(currentScale, currentScale);
    connect(_gridSizer, &GridSizer::accepted, this, &MapFrame::gridSizerAccepted);
    connect(_gridSizer, &GridSizer::rejected, this, &MapFrame::gridSizerRejected);
}

void MapFrame::setShowMarkers(bool show)
{
    // TODO: Layers
    _mapSource->setShowMarkers(show);
}

void MapFrame::addNewMarker()
{
    QRect viewportRect = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect().toAlignedRect();
    QPoint centerPos = viewportRect.topLeft() + QPoint(viewportRect.width() / 2, viewportRect.height() / 2);
    addMarker(centerPos);
}

void MapFrame::addMarker(const QPointF& markerPosition)
{
    if((!_scene) || (!_mapSource))
        return;

    MapMarkerDialog dlg(MapMarker(), *_mapSource, this);
    dlg.resize(width() / 2, height() / 2);
    dlg.move(ui->graphicsView->mapFromScene(markerPosition) + mapToGlobal(ui->graphicsView->pos()));
    if(dlg.exec() == QDialog::Accepted)
    {
        MapMarker marker = dlg.getMarker();
        marker.setPosition(markerPosition.toPoint());
        UndoMarker* undoMarker = new UndoMarker(marker);
        undoMarker->createMarkerItem(_scene, 0.04 * static_cast<qreal>(_mapSource->getPartyScale()));
        _mapSource->addMarker(undoMarker);
        emit dirty();
        setShowMarkers(true);
    }
}

void MapFrame::editMapMarker(UndoMarker* marker)
{
    if((!_mapSource) || (!marker))
        return;

    MapMarkerDialog dlg(marker->getMarker(), *_mapSource, this);
    dlg.resize(width() / 2, height() / 2);
    dlg.move(ui->graphicsView->mapFromScene(marker->getMarker().getPosition()) + mapToGlobal(ui->graphicsView->pos()));
    int result = dlg.exec();
    if(result == QDialog::Accepted)
    {
        marker->setMarker(dlg.getMarker());
        emit dirty();
        emit markerChanged();
    }
    else if(result == MapMarkerDialog::MAPMARKERDIALOG_DELETE)
    {
        deleteMapMarker(marker);
    }
}

void MapFrame::deleteMapMarker(UndoMarker* marker)
{
    if((!_mapSource) || (!marker))
        return;

    QMessageBox::StandardButton deleteConfirm = QMessageBox::question(this,
                                                                      QString("Delete Marker"),
                                                                      QString("Are you sure that you want to delete this marker?"));

    if(deleteConfirm == QMessageBox::Yes)
    {
        if(marker->getMarkerItem())
            marker->getMarkerItem()->setVisible(false);
        _mapSource->removeMarker(marker);
        emit dirty();
        emit markerChanged();
    }
}

void MapFrame::setMapEdit(bool enabled)
{
    editModeToggled(enabled ? DMHelper::EditMode_FoW : DMHelper::EditMode_Move);
}

void MapFrame::setBrushMode(int brushMode)
{
    if(_brushMode != brushMode)
    {
        _brushMode = brushMode;
        setMapCursor();
        emit brushModeSet(_brushMode);
    }
}

void MapFrame::brushSizeChanged(int size)
{
    if(_brushSize != size)
    {
        _brushSize = size;
        setMapCursor();
    }
}

void MapFrame::editMapFile()
{
    if(!_mapSource)
        return;

    QString filename = QFileDialog::getOpenFileName(this, QString("Select Map Image..."));
    if(!filename.isEmpty())
    {
        uninitializeMap();
        _mapSource->uninitialize();
        _mapSource->setFileName(filename);
        initializeMap();
    }
}

void MapFrame::zoomIn()
{
    setScale(_scale * 1.1);
}

void MapFrame::zoomOut()
{
    setScale(_scale * 0.9);
}

void MapFrame::zoomOne()
{
    setScale(1.0);
}

void MapFrame::zoomFit()
{
    if((!ui->graphicsView->viewport()) || (!_scene))
        return;

    qreal widthFactor = static_cast<qreal>(ui->graphicsView->viewport()->width()) / _scene->width();
    qreal heightFactor = static_cast<qreal>(ui->graphicsView->viewport()->height()) / _scene->height();
    setScale(qMin(widthFactor, heightFactor));
}

void MapFrame::zoomSelect(bool enabled)
{
    editModeToggled(enabled ? DMHelper::EditMode_ZoomSelect : DMHelper::EditMode_Move);
}

void MapFrame::zoomDelta(int delta)
{
    if(delta > 0)
        zoomIn();
    else if(delta < 0)
        zoomOut();
}

void MapFrame::centerWindow(const QPointF& position)
{
    ui->graphicsView->centerOn(position);
    setMapCursor();
    storeViewRect();
}

void MapFrame::cancelSelect()
{
    gridSizerRejected();
    editModeToggled(DMHelper::EditMode_Move);
}

void MapFrame::setErase(bool enabled)
{
    _erase = enabled;
}

void MapFrame::setSmooth(bool enabled)
{
    _smooth = enabled;
}

void MapFrame::setDistance(bool enabled)
{
    editModeToggled(enabled ? DMHelper::EditMode_Distance : DMHelper::EditMode_Move);
}

void MapFrame::setFreeDistance(bool enabled)
{
    editModeToggled(enabled ? DMHelper::EditMode_FreeDistance : DMHelper::EditMode_Move);
}

void MapFrame::setDistanceScale(int scale)
{
    if(!_mapSource)
        return;

    _mapSource->setMapScale(scale);
}

void MapFrame::setDistanceLineColor(const QColor& color)
{
    if(!_mapSource)
        return;

    _mapSource->setDistanceLineColor(color);
}

void MapFrame::setDistanceLineType(int lineType)
{
    if(!_mapSource)
        return;

    _mapSource->setDistanceLineType(lineType);
}

void MapFrame::setDistanceLineWidth(int lineWidth)
{
    if(!_mapSource)
        return;

    _mapSource->setDistanceLineWidth(lineWidth);
}

void MapFrame::setCameraCouple()
{
    setCameraToView();
}

void MapFrame::setCameraMap()
{
    if((!_cameraRect) || (!_mapSource))
        return;

    QRectF newRect = _mapSource->getLayerScene().boundingRect();
    _cameraRect->setCameraRect(newRect);
    emit cameraRectChanged(newRect);
}

void MapFrame::setCameraVisible()
{
    if((!_cameraRect) || (!_mapSource))
        return;

    QRectF visibleRect = _mapSource->getLayerScene().boundingRect();

    QList<Layer*> fowLayers = _mapSource->getLayerScene().getLayers(DMHelper::LayerType_Fow);
    foreach(Layer* layer, fowLayers)
    {
        LayerFow* fowLayer = dynamic_cast<LayerFow*>(layer->getFinalLayer());
        if((fowLayer) && (fowLayer->getLayerVisiblePlayer()))
        {
            QRectF newRect = fowLayer->getFoWVisibleRect();
            visibleRect = visibleRect.intersected(newRect);
        }
    }

    if(visibleRect.isEmpty())
        return;

    _cameraRect->setCameraRect(visibleRect);
    emit cameraRectChanged(visibleRect);
}

void MapFrame::setCameraSelect(bool enabled)
{
    editModeToggled(enabled ? DMHelper::EditMode_CameraSelect : DMHelper::EditMode_Move);
}

void MapFrame::setCameraEdit(bool enabled)
{
    editModeToggled(enabled ? DMHelper::EditMode_CameraEdit : DMHelper::EditMode_Move);
}

void MapFrame::targetResized(const QSize& newSize)
{
    if(_targetSize != newSize)
    {
        qDebug() << "[MapFrame] Target size being set to: " << newSize;
        _targetSize = newSize;
    }
}

void MapFrame::setPointerOn(bool enabled)
{
    editModeToggled(enabled ? DMHelper::EditMode_Pointer : DMHelper::EditMode_Move);
}

void MapFrame::setPointerFile(const QString& filename)
{
    if(_pointerFile != filename)
    {
        _pointerFile = filename;
        setMapCursor();
        emit pointerFileNameChanged(_pointerFile);
    }
}

void MapFrame::setTargetLabelSize(const QSize& targetSize)
{
    _targetLabelSize = targetSize;
}

void MapFrame::publishWindowMouseDown(const QPointF& position)
{
    if((!_mapSource) || (!_partyIcon) || (!_scene))
        return;

    QPointF newPosition;
    if(!convertPublishToScene(position, newPosition))
        return;

    if(_partyIcon->contains(newPosition - _partyIcon->pos()))
    {
        _publishMouseDown = true;
        _publishMouseDownPos = newPosition;
    }
}

void MapFrame::publishWindowMouseMove(const QPointF& position)
{
    if((!_mapSource) || (!_partyIcon) || (!_publishMouseDown))
        return;

    QPointF newPosition;
    if(!convertPublishToScene(position, newPosition))
        return;

    if(newPosition == _publishMouseDownPos)
        return;

    _partyIcon->setPos(newPosition);
}

void MapFrame::publishWindowMouseRelease(const QPointF& position)
{
    Q_UNUSED(position);
    _publishMouseDown = false;
}

void MapFrame::layerSelected(int selected)
{
    if(_mapSource)
        _mapSource->getLayerScene().setSelectedLayerIndex(selected);
}

void MapFrame::publishClicked(bool checked)
{
    if((!_mapSource) || ((_isPublishing == checked) && (_renderer) && (_renderer->getObject() == _mapSource)))
        return;

    _isPublishing = checked;
    if(_cameraRect)
        _cameraRect->setPublishing(_isPublishing);

    if(_isPublishing)
    {
        if(_renderer)
            emit registerRenderer(nullptr);

        PublishGLMapRenderer* newRenderer = new PublishGLMapRenderer(_mapSource);

        rendererActivated(newRenderer);
        emit registerRenderer(newRenderer);
        emit showPublishWindow();
    }
    else
    {
        emit registerRenderer(nullptr);
    }
}

void MapFrame::setRotation(int rotation)
{
    if(_rotation == rotation)
        return;

    _rotation = rotation;
    if(_renderer)
        _renderer->setRotation(_rotation);
}

void MapFrame::setBackgroundColor(const QColor& color)
{
    if(_mapSource)
        _mapSource->setBackgroundColor(color);
}

void MapFrame::editLayers()
{
    if(!_mapSource)
        return;

    LayersEditDialog dlg(_mapSource->getLayerScene());
    dlg.resize(width() * 9 / 10, height() * 9 / 10);
    dlg.exec();

    emit setLayers(_mapSource->getLayerScene().getLayers(), _mapSource->getLayerScene().getSelectedLayerIndex());
}

void MapFrame::initializeMap()
{
    if(_scene)
    {
        qDebug() << "[MapFrame] ERROR: Cleanup of previous map frame scene contents NOT done. Undefined behavior!";
        delete _scene;
    }

    _scene = new MapFrameScene(this);
    ui->graphicsView->setScene(_scene);
    connect(_scene, &MapFrameScene::mapMousePress, this, &MapFrame::handleMapMousePress);
    connect(_scene, &MapFrameScene::mapMouseMove, this, &MapFrame::handleMapMouseMove);
    connect(_scene, &MapFrameScene::mapMouseRelease, this, &MapFrame::handleMapMouseRelease);
    connect(_scene, &MapFrameScene::mapZoom, this, &MapFrame::zoomDelta);
    connect(_scene, &MapFrameScene::addMarker, this, &MapFrame::addMarker);

    connect(_scene, &MapFrameScene::editMarker, this, &MapFrame::editMapMarker);
    connect(_scene, &MapFrameScene::deleteMarker, this, &MapFrame::deleteMapMarker);
    connect(_scene, &MapFrameScene::centerView, this, &MapFrame::centerWindow);
    connect(_scene, &MapFrameScene::clearFoW, this, &MapFrame::clearFoW);

    connect(_scene, &MapFrameScene::itemChanged, this, &MapFrame::handleItemChanged);
    connect(_scene, &MapFrameScene::changed, this, &MapFrame::handleSceneChanged);

    if(!_mapSource)
        return;

    if(_mapSource->initialize())
    {
        if(!_mapSource->isInitialized())
            return;

        qDebug() << "[MapFrame] Initializing map frame image";

        _mapSource->getLayerScene().dmInitialize(_scene);
        _mapSource->initializeMarkers(_scene);

        loadViewRect();
        checkPartyUpdate();

        if(_cameraRect)
            _cameraRect->setCameraRect(_mapSource->getCameraRect());
        else
            _cameraRect = new CameraRect(_mapSource->getCameraRect(), *_scene, ui->graphicsView->viewport());

        if(_cameraRect->getCameraRect().isValid())
        {
            QList<Layer*> videoLayers = _mapSource->getLayerScene().getLayers(DMHelper::LayerType_Video);
            foreach(Layer* layer, videoLayers)
            {
                LayerVideo* videoLayer = dynamic_cast<LayerVideo*>(layer->getFinalLayer());
                connect(videoLayer, &LayerVideo::screenshotAvailable, this, &MapFrame::zoomFit);
            }
        }

        emit cameraRectChanged(_mapSource->getCameraRect());
        emit fowChanged(_bwFoWImage);
    }

    connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    connect(_mapSource, &Map::partyChanged, this, &MapFrame::partyChanged);
    connect(_mapSource, &Map::partyIconChanged, this, &MapFrame::partyIconChanged);
    connect(_mapSource, &Map::showPartyChanged, this, &MapFrame::showPartyChanged);
    connect(_mapSource, &Map::partyScaleChanged, this, &MapFrame::partyScaleChanged);
    connect(_mapSource, &Map::mapScaleChanged, this, &MapFrame::distanceScaleChanged);
    connect(_mapSource, &Map::distanceLineColorChanged, this, &MapFrame::distanceLineColorChanged);
    connect(_mapSource, &Map::distanceLineTypeChanged, this, &MapFrame::distanceLineTypeChanged);
    connect(_mapSource, &Map::distanceLineWidthChanged, this, &MapFrame::distanceLineWidthChanged);
    connect(_mapSource, &Map::showMarkersChanged, this, &MapFrame::showMarkersChanged);
    connect(_mapSource, &Map::partyChanged, this, &MapFrame::dirty);
    connect(_mapSource, &Map::partyIconChanged, this, &MapFrame::dirty);
    connect(_mapSource, &Map::showPartyChanged, this, &MapFrame::dirty);
    connect(_mapSource, &Map::partyScaleChanged, this, &MapFrame::dirty);
    connect(_mapSource, &Map::mapScaleChanged, this, &MapFrame::dirty);
    connect(_mapSource, &Map::distanceLineColorChanged, this, &MapFrame::dirty);
    connect(_mapSource, &Map::distanceLineTypeChanged, this, &MapFrame::dirty);
    connect(_mapSource, &Map::distanceLineWidthChanged, this, &MapFrame::dirty);
    connect(&_mapSource->getLayerScene(), &LayerScene::sceneChanged, this, &MapFrame::handleMapSceneChanged);
    connect(this, &MapFrame::cameraRectChanged, _mapSource, QOverload<const QRectF&>::of(&Map::setCameraRect));

    connect(_mapSource, &Map::mapMarkerMoved, this, &MapFrame::mapMarkerMoved);
    connect(_mapSource, &Map::mapMarkerEdited, this, &MapFrame::editMapMarker);
    connect(_mapSource, &Map::unselectParty, this, &MapFrame::setPartySelected);
    connect(_mapSource, &Map::mapMarkerActivated, this, &MapFrame::activateMapMarker);

    if(_mapSource->getParty())
        emit partyChanged(_mapSource->getParty());
    else
        emit partyIconChanged(_mapSource->getPartyAltIcon());

    emit showPartyChanged(_mapSource->getShowParty());
    emit partyScaleChanged(_mapSource->getPartyScale());
    emit distanceScaleChanged(_mapSource->getMapScale());
    emit showMarkersChanged(_mapSource->getShowMarkers());
    emit distanceLineColorChanged(_mapSource->getDistanceLineColor());
    emit distanceLineTypeChanged(_mapSource->getDistanceLineType());
    emit distanceLineWidthChanged(_mapSource->getDistanceLineWidth());

    _isVideo = !_mapSource->isInitialized();
}

void MapFrame::uninitializeMap()
{
    qDebug() << "[MapFrame] Uninitializing MapFrame...";

    if((_mapSource) && (_partyIcon))
        _mapSource->setPartyIconPos(_partyIcon->pos().toPoint());

    if(_mapSource)
    {
        disconnect(this, &MapFrame::cameraRectChanged, _mapSource, QOverload<const QRectF&>::of(&Map::setCameraRect));
        disconnect(&_mapSource->getLayerScene(), &LayerScene::sceneChanged, this, &MapFrame::handleMapSceneChanged);
        disconnect(_mapSource, &Map::distanceLineColorChanged, this, &MapFrame::dirty);
        disconnect(_mapSource, &Map::distanceLineTypeChanged, this, &MapFrame::dirty);
        disconnect(_mapSource, &Map::distanceLineWidthChanged, this, &MapFrame::dirty);
        disconnect(_mapSource, &Map::mapScaleChanged, this, &MapFrame::dirty);
        disconnect(_mapSource, &Map::partyChanged, this, &MapFrame::dirty);
        disconnect(_mapSource, &Map::partyIconChanged, this, &MapFrame::dirty);
        disconnect(_mapSource, &Map::showPartyChanged, this, &MapFrame::dirty);
        disconnect(_mapSource, &Map::distanceLineColorChanged, this, &MapFrame::distanceLineColorChanged);
        disconnect(_mapSource, &Map::distanceLineTypeChanged, this, &MapFrame::distanceLineTypeChanged);
        disconnect(_mapSource, &Map::distanceLineWidthChanged, this, &MapFrame::distanceLineWidthChanged);
        disconnect(_mapSource, &Map::mapScaleChanged, this, &MapFrame::distanceScaleChanged);
        disconnect(_mapSource, &Map::partyScaleChanged, this, &MapFrame::dirty);
        disconnect(_mapSource, &Map::showMarkersChanged, this, &MapFrame::showMarkersChanged);
        disconnect(_mapSource, &Map::partyChanged, this, &MapFrame::partyChanged);
        disconnect(_mapSource, &Map::partyIconChanged, this, &MapFrame::partyIconChanged);
        disconnect(_mapSource, &Map::showPartyChanged, this, &MapFrame::showPartyChanged);
        disconnect(_mapSource, &Map::partyScaleChanged, this, &MapFrame::partyScaleChanged);
        disconnect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
        disconnect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));

        disconnect(_mapSource, &Map::mapMarkerMoved, this, &MapFrame::mapMarkerMoved);
        disconnect(_mapSource, &Map::mapMarkerEdited, this, &MapFrame::editMapMarker);
        disconnect(_mapSource, &Map::unselectParty, this, &MapFrame::setPartySelected);
        disconnect(_mapSource, &Map::mapMarkerActivated, this, &MapFrame::activateMapMarker);

    }

    cleanupBuffers();
    cleanupSelectionItems();

    delete _scene;
    _scene = nullptr;
}

void MapFrame::cleanupSelectionItems()
{
    if(!_scene)
        return;

    if(_distanceLine)
    {
        _scene->removeItem(_distanceLine);
        delete _distanceLine;
        _distanceLine = nullptr;
    }

    if(_mapItem)
    {
        if(_mapSource)
            _mapSource->removeMapItem(_mapItem);
        delete _mapItem;
        _mapItem = nullptr;
    }

    if(_distancePath)
    {
        _scene->removeItem(_distancePath);
        delete _distancePath;
        _distancePath = nullptr;
    }

    if(_distanceText)
    {
        _scene->removeItem(_distanceText);
        delete _distanceText;
        _distanceText = nullptr;
    }

    if(_rubberBand)
    {
        delete _rubberBand;
        _rubberBand = nullptr;
    }
}

void MapFrame::hideEvent(QHideEvent * event)
{
    uninitializeMap();
    emit windowClosed(this);

    QWidget::hideEvent(event);
}

void MapFrame::resizeEvent(QResizeEvent *event)
{
    qDebug() << "[MapFrame] resized: " << event->size().width() << "x" << event->size().height();
    loadViewRect();
    QWidget::resizeEvent(event);
}

void MapFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    loadViewRect();
    QWidget::showEvent(event);
}

void MapFrame::keyPressEvent(QKeyEvent *event)
{
    if(!event)
        return;

    if(event->key() == Qt::Key_Escape)
    {
        cancelSelect();
        return;
    }

    gridSizerAccepted();

    if((event->key() == Qt::Key_Space) || (event->key() == Qt::Key_Control))
    {
        _spaceDown = true;
        setMapCursor();
        event->accept();
        return;
    }
    else if(event->key() == Qt::Key_A)
    {
        editModeToggled(_editMode == DMHelper::EditMode_Pointer ? DMHelper::EditMode_Move : DMHelper::EditMode_Pointer);
        event->accept();
        return;
    }

    CampaignObjectFrame::keyPressEvent(event);
}

void MapFrame::keyReleaseEvent(QKeyEvent *event)
{
    if(event)
    {
        if((event->key() == Qt::Key_Space) || (event->key() == Qt::Key_Control))
        {
            _spaceDown = false;
            setMapCursor();
            event->accept();
            return;
        }
    }

    CampaignObjectFrame::keyReleaseEvent(event);
}

bool MapFrame::editModeToggled(int editMode)
{
    if(_editMode == editMode)
        return false;

    changeEditMode(_editMode, false);
    changeEditMode(editMode, true);

    cleanupSelectionItems();

    _editMode = editMode;
    _undoPath = nullptr;
    switch(editMode)
    {
        case DMHelper::EditMode_FoW:
        case DMHelper::EditMode_Edit:
        case DMHelper::EditMode_ZoomSelect:
        case DMHelper::EditMode_Distance:
        case DMHelper::EditMode_FreeDistance:
        case DMHelper::EditMode_CameraEdit:
        case DMHelper::EditMode_CameraSelect:
        case DMHelper::EditMode_Pointer:
            ui->graphicsView->viewport()->installEventFilter(this);
            ui->graphicsView->removeEventFilter(this);
            break;
        case DMHelper::EditMode_Move:
            ui->graphicsView->viewport()->removeEventFilter(this);
            ui->graphicsView->installEventFilter(this);
            break;
        default:
            break;
    }

    setMapCursor();
    return true;
}

void MapFrame::changeEditMode(int editMode, bool active)
{
    switch(editMode)
    {
        case DMHelper::EditMode_FoW:
            emit mapEditChanged(active);
            break;
        case DMHelper::EditMode_Distance:
            emit showDistanceChanged(active);
            break;
        case DMHelper::EditMode_FreeDistance:
            emit showFreeDistanceChanged(active);
            break;
        case DMHelper::EditMode_ZoomSelect:
            emit zoomSelectChanged(active);
            break;
        case DMHelper::EditMode_CameraEdit:
            if(_cameraRect)
                _cameraRect->setCameraSelectable(active);
            emit cameraEditToggled(active);
            break;
        case DMHelper::EditMode_CameraSelect:
            emit cameraSelectToggled(active);
            break;
        case DMHelper::EditMode_Pointer:
            setMouseTracking(active);
            emit pointerToggled(active);
            break;
        case DMHelper::EditMode_Edit:
        case DMHelper::EditMode_Move:
        default:
            break;
    }
}

bool MapFrame::checkMapMove(QEvent* event)
{
    if(!event)
        return false;

    if((event->type() != QEvent::MouseButtonPress) && (event->type() != QEvent::MouseMove) && (event->type() != QEvent::MouseButtonRelease))
        return false;

    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if((!mouseEvent) ||
       ((!_spaceDown) &&
        (!_mapMoveMouseDown) &&
        ((mouseEvent->modifiers() & Qt::ControlModifier) == 0) &&
        ((mouseEvent->buttons() & Qt::MiddleButton) == 0)))
        return false;

    if(event->type() == QEvent::MouseButtonPress)
    {
        _mapMoveMouseDown = true;
        _mouseDownPos = mouseEvent->pos();
        return true;
    }
    else if((event->type() == QEvent::MouseMove) && (_mapMoveMouseDown))
    {
        QPoint newPos = mouseEvent->pos();
        QPoint delta = _mouseDownPos - newPos;

        _mapMoveMouseDown = false;
        if(ui->graphicsView->horizontalScrollBar())
            ui->graphicsView->horizontalScrollBar()->setValue(ui->graphicsView->horizontalScrollBar()->value() + delta.x());
        if(ui->graphicsView->verticalScrollBar())
            ui->graphicsView->verticalScrollBar()->setValue(ui->graphicsView->verticalScrollBar()->value() + delta.y());
        _mapMoveMouseDown = true;

        _mouseDownPos = newPos;
        return true;
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        _mapMoveMouseDown = false;
        return true;
    }

    return false;
}

bool MapFrame::execEventFilter(QObject *obj, QEvent *event)
{
    if((!obj) || (!event))
        return false;

    switch(_editMode)
    {
        case DMHelper::EditMode_Edit:
        case DMHelper::EditMode_Move:
            return execEventFilterEditModeEdit(obj, event);
        case DMHelper::EditMode_FoW:
            return execEventFilterEditModeFoW(obj, event);
        case DMHelper::EditMode_ZoomSelect:
            return execEventFilterSelectZoom(obj, event);
        case DMHelper::EditMode_Distance:
            return execEventFilterEditModeDistance(obj, event);
        case DMHelper::EditMode_FreeDistance:
            return execEventFilterEditModeFreeDistance(obj, event);
        case DMHelper::EditMode_CameraSelect:
            return execEventFilterCameraSelect(obj, event);
        case DMHelper::EditMode_CameraEdit:
            return execEventFilterCameraEdit(obj, event);
        case DMHelper::EditMode_Pointer:
            return execEventFilterPointer(obj, event);
        default:
            break;
    };

    return false;
}

bool MapFrame::execEventFilterSelectZoom(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        _mouseDownPos = mouseEvent->pos();
        if(!_rubberBand)
        {
            _rubberBand = new QRubberBand(QRubberBand::Rectangle, ui->graphicsView);
        }
        _rubberBand->setGeometry(QRect(_mouseDownPos, QSize()));
        _rubberBand->show();

        return true;
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        if(_rubberBand)
        {
            int h = ui->graphicsView->horizontalScrollBar()->value();
            int v = ui->graphicsView->verticalScrollBar()->value();

            QRect target;
            target.setLeft(h + _rubberBand->x());
            if(_scene->width() < ui->graphicsView->width())
                target.setLeft(target.left() + ui->graphicsView->x() - ((ui->graphicsView->width() - _scene->width()) / 2)); // why the graphics view x??
            target.setTop(v + _rubberBand->y());
            if(_scene->height() < ui->graphicsView->height())
                target.setTop(target.top() + ui->graphicsView->y() - ((ui->graphicsView->height() - _scene->height()) / 2)); // why the graphics view y??
            target.setWidth(_rubberBand->width());
            target.setHeight(_rubberBand->height());

            target.setRect(target.x() / _scale, target.y() / _scale, target.width() / _scale, target.height() / _scale);

            qreal hScale = (static_cast<qreal>(ui->graphicsView->width())) / (static_cast<qreal>(target.width()));
            qreal vScale = (static_cast<qreal>(ui->graphicsView->height())) / (static_cast<qreal>(target.height()));
            qreal minScale = qMin(hScale, vScale);

            setScale(minScale);
            ui->graphicsView->horizontalScrollBar()->setValue(target.left() * minScale);
            ui->graphicsView->verticalScrollBar()->setValue(target.top() * minScale);

            cleanupSelectionItems();
        }
        return true;
    }
    else if(event->type() == QEvent::MouseMove)
    {
        if(_rubberBand)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            _rubberBand->setGeometry(QRect(_mouseDownPos, mouseEvent->pos()).normalized());
        }
        return true;
    }
    else if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape)
        {
            editModeToggled(DMHelper::EditMode_Move);
            return true;
        }
    }

    return false;
}

bool MapFrame::execEventFilterEditModeFoW(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(!_mapSource)
        return false;

    if(_brushMode == DMHelper::BrushType_Select)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            _mouseDownPos = mouseEvent->pos();
            if(!_rubberBand)
                _rubberBand = new QRubberBand(QRubberBand::Rectangle, ui->graphicsView);

            _rubberBand->setGeometry(QRect(_mouseDownPos, QSize()));
            _rubberBand->show();

            return true;
        }
        else if(event->type() == QEvent::MouseButtonRelease)
        {
            if(_rubberBand)
            {
                QRect bandRect = _rubberBand->rect();
                bandRect.moveTo(_rubberBand->pos());
                QRect shapeRect(ui->graphicsView->mapToScene(bandRect.topLeft()).toPoint(),
                                ui->graphicsView->mapToScene(bandRect.bottomRight()).toPoint());
                LayerFow* layer = dynamic_cast<LayerFow*>(_mapSource->getLayerScene().getPriority(DMHelper::LayerType_Fow));
                if(layer)
                {
                    shapeRect.translate(-layer->getPosition());
                    UndoFowShape* undoShape = new UndoFowShape(layer, MapEditShape(shapeRect, _erase, false));
                    layer->getUndoStack()->push(undoShape);
                    emit dirty();
                }
                cleanupSelectionItems();
            }
            return true;
        }
        else if(event->type() == QEvent::MouseMove)
        {
            if(_rubberBand)
            {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                _rubberBand->setGeometry(QRect(_mouseDownPos, mouseEvent->pos()).normalized());
            }
            return true;
        }
        else if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->key() == Qt::Key_Escape)
            {
                editModeToggled(DMHelper::EditMode_Move);
                return true;
            }
        }
    }
    else
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            _mouseDownPos = mouseEvent->pos();
            _mouseDown = true;

            QPoint drawPoint = ui->graphicsView->mapToScene(_mouseDownPos).toPoint();
            LayerFow* layer = dynamic_cast<LayerFow*>(_mapSource->getLayerScene().getPriority(DMHelper::LayerType_Fow));
            if(layer)
            {
                drawPoint -= layer->getPosition();
                _undoPath = new UndoFowPath(layer, MapDrawPath(_mapSource->getPartyScale() * _brushSize / 10, _brushMode, _erase, _smooth, drawPoint));
                layer->getUndoStack()->push(_undoPath);
                emit dirty();
            }
            return true;
        }
        else if(event->type() == QEvent::MouseButtonRelease)
        {
            if(_undoPath)
            {
                _undoPath = nullptr;
                emit dirty();
            }
            return true;
        }
        else if(event->type() == QEvent::MouseMove)
        {
            if(_undoPath)
            {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                QPoint drawPoint =  ui->graphicsView->mapToScene(mouseEvent->pos()).toPoint();
                if(_undoPath->getLayer())
                    drawPoint -= _undoPath->getLayer()->getPosition();
                _undoPath->addPoint(drawPoint);
                emit dirty();
            }
            return true;
        }
    }

    return false;
}

bool MapFrame::execEventFilterEditModeEdit(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(!_mapSource)
        return false;

    // TODO: Determine the right implementation approach

    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        _mouseDownPos = mouseEvent->pos();
        _mouseDown = true;
        return true;
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        _mouseDown = false;
        return true;
    }

    return false;
}

bool MapFrame::execEventFilterEditModeMove(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    Q_UNUSED(event);

    if(!_mapSource)
        return false;

    //TODO: Implement
    _scene->clearSelection();
    return false;
}

bool MapFrame::execEventFilterEditModeDistance(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(!_mapSource)
        return false;

    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if(!mouseEvent)
        return false;

    if(event->type() == QEvent::MouseButtonPress)
    {
        if(!_scene)
            return false;

        cleanupSelectionItems();

        QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
        _distanceLine = _scene->addLine(QLineF(scenePos, scenePos));
        _distanceLine->setPen(QPen(QBrush(_mapSource->getDistanceLineColor()),
                                   _mapSource->getDistanceLineWidth(),
                                   static_cast<Qt::PenStyle>(_mapSource->getDistanceLineType())));
        _distanceLine->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);

        _distanceText = _scene->addSimpleText(QString("0"));
        _distanceText->setBrush(QBrush(_mapSource->getDistanceLineColor()));
        QFont textFont = _distanceText->font();
        textFont.setPointSize(DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] / 20);
        _distanceText->setFont(textFont);
        _distanceText->setPos(scenePos);
        _distanceText->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
        _distanceText->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

        _mapItem = new MapDrawLine(QLine(scenePos.toPoint(), scenePos.toPoint()),
                                         false, true,
                                         _mapSource->getDistanceLineColor(),
                                         _mapSource->getDistanceLineWidth(),
                                         static_cast<Qt::PenStyle>(_mapSource->getDistanceLineType()));
        _mapSource->addMapItem(_mapItem);

        mouseEvent->accept();
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        mouseEvent->accept();
    }
    else if(event->type() == QEvent::MouseMove)
    {
        if((!_distanceLine) || (!_distanceText) || (!_scene) || (mouseEvent->buttons() == Qt::NoButton))
            return false;

        QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());

        QLineF line = _distanceLine->line();
        line.setP2(scenePos);
        _distanceLine->setLine(line);
        qreal lineDistance = line.length() * ((_mapSource->getMapScale() > 0.0) ? _mapSource->getMapScale() : _mapSource->getPartyScale()) / 1000.0;
        QString distanceText;
        distanceText = QString::number(lineDistance, 'f', 1);
        _distanceText->setText(distanceText);
        _distanceText->setPos(line.center());

        MapDrawLine* mapLine = dynamic_cast<MapDrawLine*>(_mapItem);
        if(mapLine)
            mapLine->setP2(scenePos.toPoint());

        emit distanceChanged(distanceText);
        mouseEvent->accept();
    }
    return false;
}

bool MapFrame::execEventFilterEditModeFreeDistance(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(!_mapSource)
        return false;

    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if(!mouseEvent)
        return false;

    if(event->type() == QEvent::MouseButtonPress)
    {
        if(!_scene)
            return false;

        cleanupSelectionItems();
        _mouseDownPos = mouseEvent->pos();

        _distanceText = _scene->addSimpleText(QString("0"));
        _distanceText->setBrush(QBrush(_mapSource->getDistanceLineColor()));
        QFont textFont = _distanceText->font();
        textFont.setPointSize(DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] / 20);
        _distanceText->setFont(textFont);
        _distanceText->setPos(ui->graphicsView->mapToScene(mouseEvent->pos() + QPoint(5, 5)));
        _distanceText->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
        _distanceText->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

        _mapItem = new MapDrawPath(1, DMHelper::BrushType_Circle,
                                   false, true,
                                   ui->graphicsView->mapToScene(mouseEvent->pos()).toPoint(),
                                   _mapSource->getDistanceLineColor(),
                                   _mapSource->getDistanceLineWidth(),
                                   static_cast<Qt::PenStyle>(_mapSource->getDistanceLineType()));
        _mapSource->addMapItem(_mapItem);

        mouseEvent->accept();
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        mouseEvent->accept();
    }
    else if(event->type() == QEvent::MouseMove)
    {
        if((!_distanceText) || (!_scene) || (_mapSource->getMapScale() <= 0.0) || (mouseEvent->buttons() == Qt::NoButton))
            return false;

        QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
        QPainterPath currentPath;
        if(_distancePath)
        {
            currentPath = _distancePath->path();
            currentPath.lineTo(scenePos);
            _distancePath->setPath(currentPath);
        }
        else
        {
            currentPath.moveTo(ui->graphicsView->mapToScene(_mouseDownPos));
            currentPath.lineTo(scenePos);
            _distancePath = _scene->addPath(currentPath, QPen(QBrush(_mapSource->getDistanceLineColor()),
                                                              _mapSource->getDistanceLineWidth(),
                                                              static_cast<Qt::PenStyle>(_mapSource->getDistanceLineType())));
            _distancePath->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
        }
        qreal lineDistance = _distancePath->path().length() * ((_mapSource->getMapScale() > 0.0) ? _mapSource->getMapScale() : _mapSource->getPartyScale()) / 1000.0;
        QString distanceText;
        distanceText = QString::number(lineDistance, 'f', 1);
        _distanceText->setText(distanceText);
        _distanceText->setPos(ui->graphicsView->mapToScene(mouseEvent->pos() + QPoint(5, 5)));

        MapDrawPath* mapPath = dynamic_cast<MapDrawPath*>(_mapItem);
        if(mapPath)
            mapPath->addPoint(scenePos.toPoint());

        emit distanceChanged(distanceText);
        mouseEvent->accept();
    }
    return false;
}

bool MapFrame::execEventFilterCameraSelect(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        _mouseDownPos = mouseEvent->pos();
        if(!_rubberBand)
        {
            _rubberBand = new QRubberBand(QRubberBand::Rectangle, ui->graphicsView);
        }
        _rubberBand->setGeometry(QRect(_mouseDownPos, QSize()));
        _rubberBand->show();

        return true;
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        if((_rubberBand) && (_mapSource))
        {
            QRect bandRect(_rubberBand->x(), _rubberBand->y(), _rubberBand->width(), _rubberBand->height());
            QRectF sceneBand = ui->graphicsView->mapToScene(bandRect).boundingRect();
            QRectF targetRect = sceneBand.intersected(_mapSource->getLayerScene().boundingRect());
            targetRect.adjust(1.0, 1.0, -1.0, -1.0);

            if(_cameraRect)
            {
                _cameraRect->setCameraRect(targetRect);
                emit cameraRectChanged(targetRect);
            }

            cleanupSelectionItems();
            ui->graphicsView->update();
            editModeToggled(DMHelper::EditMode_Move);
        }
        return true;
    }
    else if(event->type() == QEvent::MouseMove)
    {
        if(_rubberBand)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            _rubberBand->setGeometry(QRect(_mouseDownPos, mouseEvent->pos()).normalized());
        }
        return true;
    }
    else if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape)
        {
            editModeToggled(DMHelper::EditMode_Move);
            return true;
        }
    }

    return false;
}

bool MapFrame::execEventFilterCameraEdit(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if((event->type() == QEvent::MouseButtonPress) ||
       (event->type() == QEvent::MouseButtonRelease) ||
       (event->type() == QEvent::MouseButtonDblClick))
    {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if(mouseEvent)
        {
            // Ignore any interactions with items other than overlays. The camera rect is set to Z_Overlay when active.
            QGraphicsItem* item = findTopObject(mouseEvent->pos());
            if((item) && (item->zValue() < DMHelper::BattleDialog_Z_Overlay))
                return true;
        }
    }

    // Continue with normal processing
    return false;
}

bool MapFrame::execEventFilterPointer(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(event->type() == QEvent::MouseMove)
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if(!mouseEvent)
            return false;

        emit pointerPositionChanged(ui->graphicsView->mapToScene(mouseEvent->pos()));
    }

    // Ignore any mouse clicks
    return((event->type() == QEvent::MouseButtonPress) ||
           (event->type() == QEvent::MouseButtonRelease) ||
           (event->type() == QEvent::MouseButtonDblClick));
}

void MapFrame::cleanupBuffers()
{
    QGraphicsItem* tempItem;

    if(_partyIcon)
    {
        if(_scene)
            _scene->removeItem(_partyIcon);
        tempItem = _partyIcon;
        _partyIcon = nullptr;
        delete tempItem;
    }

    if(_cameraRect)
    {
        delete _cameraRect;
        _cameraRect = nullptr;
    }

    if(_mapSource)
    {
        _mapSource->getLayerScene().dmUninitialize();
        _mapSource->cleanupMarkers();
    }

    _bwFoWImage = QImage();

    cleanupSelectionItems();
}

void MapFrame::setMapCursor()
{
    if(_mapMoveMouseDown)
    {
        ui->graphicsView->viewport()->setCursor(QCursor(Qt::ClosedHandCursor));
    }
    else if(_spaceDown)
    {
        ui->graphicsView->viewport()->setCursor(QCursor(Qt::OpenHandCursor));
    }
    else
    {
        switch(_editMode)
        {
            case DMHelper::EditMode_Move:
            case DMHelper::EditMode_CameraEdit:
                ui->graphicsView->viewport()->setCursor(QCursor(Qt::ArrowCursor));
                break;
            case DMHelper::EditMode_Distance:
            case DMHelper::EditMode_FreeDistance:
                {
                    QPixmap cursorPixmap(":/img/data/icon_distancecursor.png");
                    ui->graphicsView->viewport()->setCursor(QCursor(cursorPixmap.scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                                                                    32 * DMHelper::CURSOR_SIZE / cursorPixmap.width(),
                                                                    32 * DMHelper::CURSOR_SIZE / cursorPixmap.height()));
                }
                break;
            case DMHelper::EditMode_ZoomSelect:
                ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                break;
            case DMHelper::EditMode_CameraSelect:
                {
                    QPixmap cursorPixmap(":/img/data/icon_cameraselectcursor.png");
                    ui->graphicsView->viewport()->setCursor(QCursor(cursorPixmap.scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                                                                    32 * DMHelper::CURSOR_SIZE / cursorPixmap.width(),
                                                                    32 * DMHelper::CURSOR_SIZE / cursorPixmap.height()));
                }
                break;
            case DMHelper::EditMode_Pointer:
                ui->graphicsView->viewport()->setCursor(QCursor(getPointerPixmap().scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                break;
            case DMHelper::EditMode_FoW:
            case DMHelper::EditMode_Edit:
            default:
                if((_brushMode == DMHelper::BrushType_Circle) || (_brushMode == DMHelper::BrushType_Square))
                    drawEditCursor();
                else
                    ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                break;
        }
    }
}

void MapFrame::drawEditCursor()
{
    if(!_mapSource)
        return;

    int cursorSize = _scale * _mapSource->getPartyScale() * _brushSize / 5;
    QPixmap cursorPixmap(QSize(cursorSize, cursorSize));
    cursorPixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&cursorPixmap);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QBrush(Qt::black), 4));
        if(_brushMode == DMHelper::BrushType_Circle)
            painter.drawEllipse(0, 0, cursorSize, cursorSize);
        else
            painter.drawRect(0, 0, cursorSize, cursorSize);
    painter.end();

    ui->graphicsView->viewport()->setCursor(QCursor(cursorPixmap));
}

void MapFrame::setScale(qreal s)
{
    _scale = s;
    ui->graphicsView->setTransform(QTransform::fromScale(_scale, _scale));
    setMapCursor();
    storeViewRect();
}

void MapFrame::storeViewRect()
{
    if(!_mapSource)
        return;

    _mapSource->setMapRect(ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect().toAlignedRect());
}

void MapFrame::loadViewRect()
{
    if(!_mapSource)
        return;

    if(_mapSource->getMapRect().isValid())
    {
        disconnect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
        disconnect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));

        ui->graphicsView->fitInView(_mapSource->getMapRect(), Qt::KeepAspectRatio);
        QTransform t = ui->graphicsView->transform();
        _scale = t.m11();

        connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
        connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    }
    else
    {
        zoomFit();
    }
}

void MapFrame::checkPartyUpdate()
{
    if((!_mapSource) || (!_scene))
        return;

    if((!_mapSource->getShowParty()) ||
       ((!_mapSource->getParty()) && (_mapSource->getPartyAltIcon().isEmpty())))
    {
        delete _partyIcon;
        _partyIcon = nullptr;
        return;
    }

    QPixmap partyPixmap = _mapSource->getPartyPixmap();
    if(!partyPixmap.isNull())
    {
        if(!_partyIcon)
        {
            _partyIcon = new UnselectedPixmap();
            _scene->addItem(_partyIcon);
            if((_mapSource->getPartyIconPos().x() == -1) && (_mapSource->getPartyIconPos().y() == -1))
                _mapSource->setPartyIconPos(QPoint(_scene->width() / 2, _scene->height() / 2));
            _partyIcon->setFlag(QGraphicsItem::ItemIsMovable, true);
            _partyIcon->setFlag(QGraphicsItem::ItemIsSelectable, true);
            _partyIcon->setPos(_mapSource->getPartyIconPos());
            _partyIcon->setZValue(DMHelper::BattleDialog_Z_Combatant);
        }

        qreal scaleFactor = (static_cast<qreal>(_mapSource->getPartyScale()-2)) / static_cast<qreal>(qMax(partyPixmap.width(), partyPixmap.height()));
        _partyIcon->setScale(scaleFactor);

        _partyIcon->setPixmap(partyPixmap);
    }
}

void MapFrame::gridSizerAccepted()
{
    if(!_gridSizer)
        return;

    setPartyScale(_gridSizer->getSize());
    gridSizerRejected();
}

void MapFrame::gridSizerRejected()
{
    if(!_gridSizer)
        return;

    _gridSizer->deleteLater();
    _gridSizer = nullptr;
}

void MapFrame::rendererActivated(PublishGLMapRenderer* renderer)
{
    if((!renderer) || (!_mapSource) || (renderer->getObject() != _mapSource))
        return;

    connect(this, &MapFrame::distanceChanged, renderer, &PublishGLMapRenderer::distanceChanged);
    connect(this, &MapFrame::fowChanged, renderer, &PublishGLMapRenderer::fowChanged);
    connect(this, &MapFrame::cameraRectChanged, renderer, &PublishGLMapRenderer::setCameraRect);
    connect(this, &MapFrame::markerChanged, renderer, &PublishGLMapRenderer::markerChanged);
    connect(this, &MapFrame::pointerToggled, renderer, &PublishGLRenderer::pointerToggled);
    connect(this, &MapFrame::pointerPositionChanged, renderer, &PublishGLRenderer::setPointerPosition);
    connect(this, &MapFrame::pointerFileNameChanged, renderer, &PublishGLRenderer::setPointerFileName);
    connect(renderer, &PublishGLMapRenderer::deactivated, this, &MapFrame::rendererDeactivated);

    renderer->setPointerFileName(_pointerFile);
    renderer->setRotation(_rotation);
    renderer->fowChanged(_bwFoWImage);

    if(_cameraRect)
        renderer->setCameraRect(_cameraRect->getCameraRect());

    _renderer = renderer;
}

void MapFrame::rendererDeactivated()
{
    if(!_renderer)
        return;

    disconnect(this, &MapFrame::distanceChanged, _renderer, &PublishGLMapRenderer::distanceChanged);
    disconnect(this, &MapFrame::fowChanged, _renderer, &PublishGLMapRenderer::fowChanged);
    disconnect(this, &MapFrame::cameraRectChanged, _renderer, &PublishGLMapRenderer::setCameraRect);
    disconnect(this, &MapFrame::markerChanged, _renderer, &PublishGLMapRenderer::markerChanged);
    disconnect(this, &MapFrame::pointerToggled, _renderer, &PublishGLRenderer::pointerToggled);
    disconnect(this, &MapFrame::pointerPositionChanged, _renderer, &PublishGLRenderer::setPointerPosition);
    disconnect(this, &MapFrame::pointerFileNameChanged, _renderer, &PublishGLRenderer::setPointerFileName);
    disconnect(_renderer, &PublishGLMapRenderer::deactivated, this, &MapFrame::rendererDeactivated);

    _renderer = nullptr;
}

void MapFrame::handleMapMousePress(const QPointF& pos)
{
    _mouseDown = true;
    _mouseDownPos = ui->graphicsView->mapFromScene(pos);
}

void MapFrame::handleMapMouseMove(const QPointF& pos)
{
    if(!_mouseDown)
        return;

    QPoint viewPos = ui->graphicsView->mapFromScene(pos);
    QPoint delta = _mouseDownPos - viewPos;

    _mouseDown = false;
    if(ui->graphicsView->horizontalScrollBar())
        ui->graphicsView->horizontalScrollBar()->setValue(ui->graphicsView->horizontalScrollBar()->value() + delta.x());
    if(ui->graphicsView->verticalScrollBar())
        ui->graphicsView->verticalScrollBar()->setValue(ui->graphicsView->verticalScrollBar()->value() + delta.y());
    _mouseDown = true;

    _mouseDownPos = viewPos;
}

void MapFrame::handleMapMouseRelease(const QPointF& pos)
{
    Q_UNUSED(pos);
    _mouseDown = false;
}

void MapFrame::handleActivateMapMarker()
{
    if(!_activatedId.isNull())
        emit encounterSelected(_activatedId);
}

void MapFrame::handleItemChanged(QGraphicsItem* item)
{
    if((_cameraRect) && (_cameraRect == item))
    {
        emit cameraRectChanged(_cameraRect->getCameraRect());
        ui->graphicsView->update();
    }
}

void MapFrame::handleSceneChanged(const QList<QRectF> &region)
{
    Q_UNUSED(region);

    if((_mapSource) && (_partyIcon))
        _mapSource->setPartyIconPos(_partyIcon->pos().toPoint());

    if((_isPublishing) && (_renderer))
        _renderer->updateRender();
}

void MapFrame::handleMapSceneChanged()
{
    if(!_mapSource)
        return;

    // If the map update was delayed due to loading, fix the map rect
    if(_mapSource->getMapRect().isEmpty())
        zoomFit();

    if(_mapSource->getCameraRect().isEmpty())
        setCameraMap();

    emit setLayers(_mapSource->getLayerScene().getLayers(), _mapSource->getLayerScene().getSelectedLayerIndex());
}

bool MapFrame::convertPublishToScene(const QPointF& publishPosition, QPointF& scenePosition)
{
    qreal publishWidth = 0.0;
    qreal publishX = 0.0;
    qreal publishY = 0.0;

    if(_rotation == 0)
    {
        publishWidth = _targetLabelSize.width();
        publishX = publishPosition.x();
        publishY = publishPosition.y();
    }
    else if(_rotation == 90)
    {
        publishWidth = _targetLabelSize.height();
        publishX = publishPosition.y();
        publishY = 1.0 - publishPosition.x();
    }
    else if(_rotation == 180)
    {
        publishWidth = _targetLabelSize.width();
        publishX = 1.0 - publishPosition.x();
        publishY = 1.0 - publishPosition.y();
    }
    else if(_rotation == 270)
    {
        publishWidth = _targetLabelSize.height();
        publishX = 1.0 - publishPosition.y();
        publishY = publishPosition.x();
    }

    if(publishWidth <= 0)
        return false;

    if((publishX < 0.0) || (publishX > 1.0) || (publishY < 0.0) || (publishY > 1.0))
        return false;

    scenePosition = QPointF((publishX * _publishRect.width()) + _publishRect.x(),
                            (publishY * _publishRect.height()) + _publishRect.y());

    return true;
}

void MapFrame::setCameraToView()
{
    if((!_cameraRect) || (!_mapSource))
        return;

    QRectF viewRect = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect();    
    QRectF targetRect = viewRect.intersected(_mapSource->getLayerScene().boundingRect());
    targetRect.adjust(1.0, 1.0, -1.0, -1.0);
    _cameraRect->setCameraRect(targetRect);
    emit cameraRectChanged(targetRect);
}

QGraphicsItem* MapFrame::findTopObject(const QPoint &pos)
{
    QList<QGraphicsItem *> itemList = ui->graphicsView->items(pos);
    if(itemList.count() <= 0)
        return nullptr;

    // Search for the first selectable item
    for(QGraphicsItem* item : std::as_const(itemList))
    {
        if((item)&&((item->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable))
            return dynamic_cast<QGraphicsItem*>(item);
    }

    // If we get here, nothing selectable was found
    return nullptr;
}

QPixmap MapFrame::getPointerPixmap()
{
    if(!_pointerFile.isEmpty())
    {
        QPixmap result;
        if(result.load(_pointerFile))
            return result;
    }

    return QPixmap(":/img/data/arrow.png");
}
