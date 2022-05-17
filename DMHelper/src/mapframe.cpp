#include "mapframe.h"
#include "ui_mapframe.h"
#include "mapframescene.h"
#include "dmconstants.h"
#include "map.h"
#include "mapmarkergraphicsitem.h"
#include "undofill.h"
#include "undoshape.h"
#include "undomarker.h"
#include "mapmarkerdialog.h"
#include "mapcolorizedialog.h"
#include "selectzoom.h"
#include "campaign.h"
#include "party.h"
#include "unselectedpixmap.h"
#include "camerarect.h"
#include "publishglmapimagerenderer.h"
#include "publishglmapvideorenderer.h"
#include "videoplayerglscreenshot.h"
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
    _backgroundImage(nullptr),
    _fow(nullptr),
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
    connect(_mapSource, &Map::executeUndo, this, &MapFrame::undoPaint);
    connect(_mapSource, &Map::requestFoWUpdate, this, &MapFrame::updateFoW);

    rendererActivated(dynamic_cast<PublishGLMapRenderer*>(currentRenderer));

    _isPublishing = (currentRenderer) && (_mapSource) && (currentRenderer->getObject() == _mapSource);
    if(_cameraRect)
        _cameraRect->setPublishing(_isPublishing);

    emit checkableChanged(_isVideo);
    emit setPublishEnabled(true);
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

    disconnect(this, SIGNAL(dirty()), _mapSource, SIGNAL(dirty()));
    disconnect(_mapSource, &Map::executeUndo, this, &MapFrame::undoPaint);
    disconnect(_mapSource, &Map::requestFoWUpdate, this, &MapFrame::updateFoW);
    disconnect(_mapSource, &Map::requestMapMarker, this, &MapFrame::createMapMarker);
    setMap(nullptr);

    _spaceDown = false;
}

void MapFrame::setMap(Map* map)
{
    if(_mapSource)
    {
        editModeToggled(DMHelper::EditMode_Move);
        uninitializeFoW();
    }

    _mapSource = map;
    if(!_mapSource)
        return;

    connect(_mapSource, &Map::requestMapMarker, this, &MapFrame::createMapMarker);

    initializeFoW();
    setMapCursor();
}

void MapFrame::mapMarkerMoved(int markerId)
{
    if(!_mapSource)
        return;

    UndoMarker* undoMarker = _mapSource->getMapMarker(markerId);
    if(!undoMarker)
        return;

    if(undoMarker->getMarkerItem())
    {
        undoMarker->getMarker().setPosition(undoMarker->getMarkerItem()->pos().toPoint());
        emit dirty();
    }
}

void MapFrame::activateMapMarker(int markerId)
{
    if(!_mapSource)
        return;

    UndoMarker* undoMarker = _mapSource->getMapMarker(markerId);
    if(!undoMarker)
        return;

    if(undoMarker->getMarker().getEncounter().isNull())
        return;

    _activatedId = undoMarker->getMarker().getEncounter();
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

    return QWidget::eventFilter(obj, event);
}

QAction* MapFrame::getUndoAction(QObject* parent)
{
    return _mapSource->getUndoStack()->createUndoAction(parent);
}

QAction* MapFrame::getRedoAction(QObject* parent)
{
    return _mapSource->getUndoStack()->createRedoAction(parent);
}

void MapFrame::updateFoW()
{
    if((_fow)&&(_mapSource))
    {
        _fow->setPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
    }
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

    UndoFill* undoFill = new UndoFill(_mapSource, MapEditFill(QColor(0,0,0,255)));
    _mapSource->getUndoStack()->push(undoFill);
    _mapSource->fillFoW(QColor(0,0,0,255), &_bwFoWImage);
    emit dirty();
    emit fowChanged(_bwFoWImage);
}

void MapFrame::clearFoW()
{
    if(!_mapSource)
        return;

    UndoFill* undoFill = new UndoFill(_mapSource, MapEditFill(QColor(0,0,0,0)));
    _mapSource->getUndoStack()->push(undoFill);
    _mapSource->fillFoW(QColor(0,0,0,0), &_bwFoWImage);
    emit dirty();
    emit fowChanged(_bwFoWImage);
}

void MapFrame::undoPaint()
{
    if(!_mapSource)
        return;

    _mapSource->applyPaintTo(nullptr, QColor(0,0,0,128), _mapSource->getUndoStack()->index() - 1);

    updateFoW();
}

void MapFrame::clear()
{
    _mapSource = nullptr;
    delete _partyIcon; _partyIcon = nullptr;
    delete _backgroundImage; _backgroundImage = nullptr;
//    delete _backgroundVideo; _backgroundVideo = nullptr;
    delete _fow; _fow = nullptr;
    delete _undoPath; _undoPath = nullptr;
}

void MapFrame::colorize()
{
    if(!_mapSource)
        return;

    if(!_backgroundImage)
        return;

    bool previousApplied = _mapSource->isFilterApplied();
    if(previousApplied)
        _mapSource->setApplyFilter(false);
    MapColorizeDialog dlg(_mapSource->getBackgroundImage(), _mapSource->getFilter());
    dlg.resize(width() / 2, height() / 2);
    if(dlg.exec() == QDialog::Accepted)
    {
        _mapSource->setFilter(dlg.getFilter());
        _mapSource->setApplyFilter(dlg.getFilter().isValid());
        _backgroundImage->setPixmap(QPixmap::fromImage(_mapSource->getBackgroundImage()));
    }
    else
    {
        if(previousApplied)
            _mapSource->setApplyFilter(true);
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

void MapFrame::setShowMarkers(bool show)
{
    if((!_mapSource) || (!_mapSource->getUndoStack()))
        return;

    for(int i = 0; i < _mapSource->getUndoStack()->index(); ++i )
    {
        const UndoMarker* marker = dynamic_cast<const UndoMarker*>(_mapSource->getUndoStack()->command(i));
        if((marker) && (marker->getMarkerItem()) && (!marker->isRemoved()))
            marker->getMarkerItem()->setVisible(show);
    }

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
    MapMarkerDialog dlg(MapMarker(), *_mapSource, this);
    dlg.resize(width() / 3, height() / 3);
    dlg.move(ui->graphicsView->mapFromScene(markerPosition) + mapToGlobal(ui->graphicsView->pos()));
    if(dlg.exec() == QDialog::Accepted)
    {
        MapMarker marker = dlg.getMarker();
        marker.setPosition(markerPosition.toPoint());
        UndoMarker* undoMarker = new UndoMarker(_mapSource, marker);
        _mapSource->getUndoStack()->push(undoMarker);
        emit dirty();
        setShowMarkers(true);
    }
}

void MapFrame::createMapMarker(UndoMarker* undoEntry, MapMarker* marker)
{
    if((!undoEntry) || (!marker) || (!_scene))
        return;

    if(!_mapSource)
        return;

    MapMarkerGraphicsItem* markerItem = new MapMarkerGraphicsItem(_scene, *marker, 0.04 * static_cast<qreal>(_mapSource->getPartyScale()), *this);
    markerItem->setPos(marker->getPosition());
    markerItem->setZValue(DMHelper::BattleDialog_Z_BackHighlight);

    markerItem->setVisible(_mapSource->getShowMarkers());
    undoEntry->setMarkerItem(markerItem);

    emit markerChanged();
}

void MapFrame::editMapMarker(int markerId)
{
    if(!_mapSource)
        return;

    UndoMarker* undoMarker = _mapSource->getMapMarker(markerId);
    if(!undoMarker)
        return;

    MapMarkerDialog dlg(undoMarker->getMarker(), *_mapSource, this);
    dlg.resize(width() / 3, height() / 3);
    dlg.move(ui->graphicsView->mapFromScene(undoMarker->getMarker().getPosition()) + mapToGlobal(ui->graphicsView->pos()));
    int result = dlg.exec();
    if(result == QDialog::Accepted)
    {
        undoMarker->setMarker(dlg.getMarker());
        emit dirty();
        emit markerChanged();
    }
    else if(result == MapMarkerDialog::MAPMARKERDIALOG_DELETE)
    {
        deleteMapMarker(undoMarker->getMarker().getID());
    }
}

void MapFrame::deleteMapMarker(int markerId)
{
    if(!_mapSource)
        return;

    UndoMarker* undoMarker = _mapSource->getMapMarker(markerId);
    if(!undoMarker)
        return;

    QMessageBox::StandardButton deleteConfirm = QMessageBox::question(this,
                                                                      QString("Delete Marker"),
                                                                      QString("Are you sure that you want to delete this marker?"));

    if(deleteConfirm == QMessageBox::Yes)
    {
        undoMarker->setRemoved(false);
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
        uninitializeFoW();
        _mapSource->uninitialize();
        _mapSource->setFileName(filename);
        initializeFoW();
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
    if((!_cameraRect) || (!_backgroundImage))
        return;

    QRectF newRect = _backgroundImage->boundingRect();
    _cameraRect->setCameraRect(newRect);
    emit cameraRectChanged(newRect);
}

void MapFrame::setCameraVisible()
{
    if((!_cameraRect) || (!_mapSource))
        return;

    QRectF newRect = _mapSource->getShrunkPublishRect();
    _cameraRect->setCameraRect(newRect);
    emit cameraRectChanged(newRect);
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

        PublishGLMapRenderer* newRenderer;
        if(_isVideo)
            newRenderer = new PublishGLMapVideoRenderer(_mapSource);
        else
            newRenderer = new PublishGLMapImageRenderer(_mapSource);

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

void MapFrame::initializeFoW()
{
    if((_backgroundImage) || (_fow) || (_scene))
        qDebug() << "[MapFrame] ERROR: Cleanup of previous map frame contents NOT done. Undefined behavior!";

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
        setBackgroundPixmap(QPixmap::fromImage(_mapSource->getBackgroundImage()));

        _fow = _scene->addPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
        _fow->setEnabled(false);
        _fow->setZValue(-1);
        _bwFoWImage = _mapSource->getBWFoWImage();

        loadViewRect();
        checkPartyUpdate();
        createMarkerItems();

        if(_cameraRect)
            _cameraRect->setCameraRect(_mapSource->getCameraRect());
        else
            _cameraRect = new CameraRect(_mapSource->getCameraRect(), *_scene, ui->graphicsView->viewport());

        emit cameraRectChanged(_mapSource->getCameraRect());
        emit fowChanged(_bwFoWImage);
    }
    else
    {
        if(_mapSource->isValid())
            extractDMScreenshot();
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
    connect(this, &MapFrame::cameraRectChanged, _mapSource, QOverload<const QRectF&>::of(&Map::setCameraRect));

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

void MapFrame::uninitializeFoW()
{
    qDebug() << "[MapFrame] Uninitializing MapFrame...";

    if((_mapSource) && (_partyIcon))
        _mapSource->setPartyIconPos(_partyIcon->pos().toPoint());

    if(_mapSource)
    {
        disconnect(this, &MapFrame::cameraRectChanged, _mapSource, QOverload<const QRectF&>::of(&Map::setCameraRect));
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
    }

    cleanupBuffers();
    cleanupSelectionItems();

    delete _scene;
    _scene = nullptr;
}

void MapFrame::createMarkerItems()
{
    if((!_mapSource) || (!_mapSource->getUndoStack()))
        return;

    for(int i = 0; i < _mapSource->getUndoStack()->index(); ++i )
    {
        const UndoMarker* constMarker = dynamic_cast<const UndoMarker*>(_mapSource->getUndoStack()->command(i));
        if(constMarker)
        {
            UndoMarker* marker = const_cast<UndoMarker*>(constMarker);
            marker->redo();
        }
    }
}

void MapFrame::cleanupMarkerItems()
{
    if((!_mapSource) || (!_mapSource->getUndoStack()))
        return;

    for(int i = 0; i < _mapSource->getUndoStack()->index(); ++i )
    {
        const UndoMarker* constMarker = dynamic_cast<const UndoMarker*>(_mapSource->getUndoStack()->command(i));
        if(constMarker)
        {
            UndoMarker* marker = const_cast<UndoMarker*>(constMarker);
            marker->undo();
        }
    }
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
    uninitializeFoW();
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
    //qDebug() << "[MapFrame] shown (" << isVisible() << ")";
    loadViewRect();
}

void MapFrame::keyPressEvent(QKeyEvent *event)
{
    if(event)
    {
        if((event->key() == Qt::Key_Space) || (event->key() == Qt::Key_Control))
        {
            _spaceDown = true;
            setMapCursor();
            event->accept();
            return;
        }
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
        else if(event->key() == Qt::Key_Escape)
        {
            editModeToggled(DMHelper::EditMode_Move);
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
            qreal minScale = qMin(hScale,vScale);

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
                UndoShape* undoShape = new UndoShape(_mapSource, MapEditShape(shapeRect, _erase, false));
                _mapSource->getUndoStack()->push(undoShape);
                _mapSource->paintFoWRect(shapeRect, undoShape->mapEditShape(), &_bwFoWImage, false);
                emit dirty();
                emit fowChanged(_bwFoWImage);
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
            _undoPath = new UndoPath(_mapSource, MapDrawPath(_brushSize, _brushMode, _erase, _smooth, drawPoint));
            _mapSource->getUndoStack()->push(_undoPath);
            _mapSource->paintFoWPoint(drawPoint, _undoPath->mapDrawPath(), &_bwFoWImage, false);

            emit fowChanged(_bwFoWImage);
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
                _undoPath->addPoint(drawPoint);
                _mapSource->paintFoWPoint(drawPoint, _undoPath->mapDrawPath(), &_bwFoWImage, false);
                emit fowChanged(_bwFoWImage);
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

        _distanceText = _scene->addSimpleText(QString("0"));
        _distanceText->setBrush(QBrush(_mapSource->getDistanceLineColor()));
        QFont textFont = _distanceText->font();
        textFont.setPointSize(DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] / 20);
        _distanceText->setFont(textFont);
        _distanceText->setPos(scenePos);

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
        if((!_distanceLine) || (!_distanceText) || (!_scene) || (_mapSource->getMapScale() <= 0.0) || (mouseEvent->buttons() == Qt::NoButton))
            return false;

        QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());

        QLineF line = _distanceLine->line();
        line.setP2(scenePos);
        _distanceLine->setLine(line);
        qreal lineDistance = line.length() * _mapSource->getMapScale() / 1000.0;
        QString distanceText;
        distanceText = QString::number(lineDistance, 'f', 1);
        _distanceText->setText(distanceText);
        _distanceText->setPos(line.center());

        MapDrawLine* mapLine = dynamic_cast<MapDrawLine*>(_mapItem);
        if(mapLine)
            mapLine->setP2(scenePos.toPoint());

        #ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
            qDebug() << "[Battle Dialog Scene] line set to " << line << ", text to " << _distanceText->text();
        #endif
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
        _distanceText->setPos(ui->graphicsView->mapToScene(mouseEvent->pos() + QPoint(5,5)));

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
            qDebug() << "[MapFrame] Move adding path lineto: " << scenePos;

        }
        else
        {
            currentPath.moveTo(ui->graphicsView->mapToScene(_mouseDownPos));
            currentPath.lineTo(scenePos);
            _distancePath = _scene->addPath(currentPath, QPen(QBrush(_mapSource->getDistanceLineColor()),
                                                              _mapSource->getDistanceLineWidth(),
                                                              static_cast<Qt::PenStyle>(_mapSource->getDistanceLineType())));
            qDebug() << "[MapFrame] Move creating path at position: " << scenePos;
        }
        qreal lineDistance = _distancePath->path().length() * _mapSource->getMapScale() / 1000.0;
        QString distanceText;
        distanceText = QString::number(lineDistance, 'f', 1);
        _distanceText->setText(distanceText);
        _distanceText->setPos(ui->graphicsView->mapToScene(mouseEvent->pos() + QPoint(5,5)));

        MapDrawPath* mapPath = dynamic_cast<MapDrawPath*>(_mapItem);
        if(mapPath)
            mapPath->addPoint(scenePos.toPoint());

        #ifdef BATTLE_DIALOG_GRAPHICS_SCENE_LOG_MOUSEMOVE
            qDebug() << "[Battle Dialog Scene] line set to " << line << ", text to " << _distanceText->text();
        #endif

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
        if(_rubberBand)
        {
            QRect bandRect(_rubberBand->x(), _rubberBand->y(), _rubberBand->width(), _rubberBand->height());
            QRectF sceneBand = ui->graphicsView->mapToScene(bandRect).boundingRect();
            QRectF targetRect = sceneBand.intersected(_backgroundImage->boundingRect());
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

void MapFrame::extractDMScreenshot()
{
    if(!_mapSource)
        return;

    VideoPlayerGLScreenshot* screenshot = new VideoPlayerGLScreenshot(_mapSource->getFileName());
    connect(screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &MapFrame::handleScreenshotReady);
    screenshot->retrieveScreenshot();
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

    if(_backgroundImage)
    {
        if(_scene)
            _scene->removeItem(_backgroundImage);
        tempItem = _backgroundImage;
        _backgroundImage = nullptr;
        delete tempItem;
    }

    if(_fow)
    {
        if(_scene)
            _scene->removeItem(_fow);
        tempItem = _fow;
        _fow = nullptr;
        delete tempItem;
    }

    _bwFoWImage = QImage();

    cleanupMarkerItems();
    cleanupSelectionItems();

    if(_scene)
    {
        _scene->clear();
        _scene->update();
    }
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
                if(_brushMode == DMHelper::BrushType_Circle)
                    drawEditCursor();
                else if(_brushMode == DMHelper::BrushType_Square)
                    drawEditCursor();
                else
                    ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                break;
        }
    }
}

void MapFrame::drawEditCursor()
{
    int cursorSize = _scale * _brushSize * 2;
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
    ui->graphicsView->setTransform(QTransform::fromScale(_scale,_scale));
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
    _partyIcon->setScale(0.04 * static_cast<qreal>(_mapSource->getPartyScale())); //250 * 25 / 625  = 10;
    QPixmap partyPixmap = _mapSource->getPartyPixmap();
    if(!partyPixmap.isNull())
        _partyIcon->setPixmap(partyPixmap);
}

void MapFrame::handleScreenshotReady(const QImage& image)
{
    if((image.isNull()) || (!_mapSource))
        return;

    setBackgroundPixmap(QPixmap::fromImage(image));
    QImage fowImage = QImage(image.size(), QImage::Format_ARGB32);
    fowImage.fill(QColor(0,0,0,0));
    _mapSource->setExternalFoWImage(fowImage);
    if(!_fow)
    {
        _fow = _scene->addPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
        _fow->setEnabled(false);
        _fow->setZValue(-1);
    }
    else
    {
        _fow->setPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
    }

    _bwFoWImage = _mapSource->getBWFoWImage();

    checkPartyUpdate();
    createMarkerItems();

    if(!_mapSource->getCameraRect().isValid())
        _mapSource->setCameraRect(_backgroundImage->boundingRect().toRect());

    if(_cameraRect)
        _cameraRect->setCameraRect(_mapSource->getCameraRect());
    else
        _cameraRect = new CameraRect(_mapSource->getCameraRect(), *_scene, ui->graphicsView->viewport());
    emit cameraRectChanged(_mapSource->getCameraRect());
    emit fowChanged(_bwFoWImage);
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
    renderer->fowChanged(QImage());

    if(_cameraRect)
        renderer->setCameraRect(_cameraRect->getCameraRect());

    _renderer = renderer;
}

void MapFrame::rendererDeactivated()
{
    if(!_renderer)
        return;

    PublishGLMapVideoRenderer* videoRenderer = dynamic_cast<PublishGLMapVideoRenderer*>(_renderer);
    if(videoRenderer)
    {
        QImage screenshot = videoRenderer->getLastScreenshot();
        if(!screenshot.isNull())
            setBackgroundPixmap(QPixmap::fromImage(screenshot));
    }

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

void MapFrame::setBackgroundPixmap(const QPixmap& pixmap)
{
    if(!_backgroundImage)
    {
        _backgroundImage = _scene->addPixmap(pixmap);
        _backgroundImage->setEnabled(false);
        _backgroundImage->setZValue(-2);
    }
    else
    {
        _backgroundImage->setPixmap(pixmap);
        _backgroundImage->show();
    }
}

void MapFrame::setCameraToView()
{
    if((!_cameraRect) || (!_backgroundImage))
        return;

    QRectF viewRect = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect();
    QRectF targetRect = viewRect.intersected(_backgroundImage->boundingRect());
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
    for(QGraphicsItem* item : qAsConst(itemList))
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
