#include "mapframe.h"
#include "ui_mapframe.h"
#include "dmconstants.h"
#include "map.h"
#include "mapmarkergraphicsitem.h"
#include "undofill.h"
#include "undoshape.h"
#include "undomarker.h"
#include "mapmarkerdialog.h"
#include "selectzoom.h"
#include "campaign.h"
#include "party.h"
#include "unselectedpixmap.h"
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QMutexLocker>
#include <QFileDialog>
#include <QDebug>

MapFrame::MapFrame(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::MapFrame),
    _scene(nullptr),
    _backgroundImage(nullptr),
    _backgroundVideo(nullptr),
    _fow(nullptr),
    _partyIcon(nullptr),
    _editMode(-1),
    _erase(true),
    _smooth(true),
    _brushMode(DMHelper::BrushType_Circle),
    _brushSize(30),
    _publishZoom(false),
    _publishVisible(false),
    _isPublishing(false),
    _isVideo(false),
    _rotation(0),
    _spaceDown(false),
    _mapMoveMouseDown(false),
    _mouseDown(false),
    _mouseDownPos(),
    _undoPath(nullptr),
    _zoomSelect(false),
    _rubberBand(nullptr),
    _scale(1.0),
    _mapSource(nullptr),
    _timerId(0),
    _videoPlayer(nullptr),
    _targetSize(),
    _bwFoWImage()
{
    ui->setupUi(this);

    // TODO: reactivate markers
    //ui->btnShowMarkers->setVisible(false);

    _scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(_scene);
    ui->graphicsView->viewport()->installEventFilter(this);
    ui->graphicsView->setStyleSheet("background-color: transparent;");

    //TODO Markers: connect(ui->btnShowMarkers,SIGNAL(toggled(bool)),this,SLOT(setViewMarkerVisible(bool)));

    connect(this, SIGNAL(dirty()), this, SLOT(resetPublishFoW()));
    connect(this, &MapFrame::dirty, this, &MapFrame::checkPartyUpdate);

    //_publishTimer = new QTimer(this);
    //_publishTimer->setSingleShot(false);
    //connect(_publishTimer, SIGNAL(timeout()),this,SLOT(executeAnimateImage()));

    //setMapCursor();
    //setScale(1.0);

    editModeToggled(DMHelper::EditMode_Move);
}

MapFrame::~MapFrame()
{
    stopPublishTimer();

    VideoPlayer* deletePlayer = _videoPlayer;
    _videoPlayer = nullptr;
    delete deletePlayer;

    cancelSelect();
    delete ui;
}

void MapFrame::activateObject(CampaignObjectBase* object)
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

    disconnect(this, SIGNAL(dirty()), _mapSource, SIGNAL(dirty()));
    disconnect(_mapSource, &Map::executeUndo, this, &MapFrame::undoPaint);
    disconnect(_mapSource, &Map::requestFoWUpdate, this, &MapFrame::updateFoW);
    setMap(nullptr);
}

void MapFrame::setMap(Map* map)
{
    if(_mapSource)
    {
        cancelSelect();
        uninitializeFoW();
    }

    _mapSource = map;
    if(!_mapSource)
        return;

    initializeFoW();
    setMapCursor();
}

MapMarkerGraphicsItem* MapFrame::addMapMarker(MapMarker& marker)
{
    // TODO: add marker and layer support...
    Q_UNUSED(marker);
    return nullptr;
}

void MapFrame::mapMarkerMoved(int markerId)
{
    Q_UNUSED(markerId);
    // TODO: add marker and layer support...

}

void MapFrame::editMapMarker(int markerId)
{
    Q_UNUSED(markerId);
    // TODO: add marker and layer support...

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
        if(_zoomSelect)
        {
            if(execEventFilterSelectZoom(obj, event))
                return true;
        }
        else if(_editMode == DMHelper::EditMode_FoW)
        {
            if(execEventFilterEditModeFoW(obj, event))
                return true;
        }
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

    UndoFill* undoFill = new UndoFill(*_mapSource, MapEditFill(QColor(0,0,0,255)));
    _mapSource->getUndoStack()->push(undoFill);
    emit dirty();
}

void MapFrame::clearFoW()
{
    if(!_mapSource)
        return;

    UndoFill* undoFill = new UndoFill(*_mapSource, MapEditFill(QColor(0,0,0,0)));
    _mapSource->getUndoStack()->push(undoFill);
    emit dirty();
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
    delete _backgroundVideo; _backgroundVideo = nullptr;
    delete _fow; _fow = nullptr;
    delete _undoPath; _undoPath = nullptr;
}

void MapFrame::cancelPublish()
{
    emit publishCancelled();
    _isPublishing = false;
}

void MapFrame::setParty(Party* party)
{
    if(!_mapSource)
        return;

    _mapSource->setParty(party);
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

void MapFrame::editModeToggled(int editMode)
{
    if(_editMode == editMode)
        return;

    _editMode = editMode;
    _undoPath = nullptr;
    switch(editMode)
    {
        case DMHelper::EditMode_FoW:
            ui->graphicsView->viewport()->installEventFilter(this);
            ui->graphicsView->removeEventFilter(this);
            break;
        case DMHelper::EditMode_Edit:
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

    QString filename = QFileDialog::getOpenFileName(this, QString("Select Map Image..."), QString(), QString("Image files (*.png *.jpg)"));
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
    qreal widthFactor = static_cast<qreal>(ui->graphicsView->viewport()->width()) / _scene->width();
    qreal heightFactor = static_cast<qreal>(ui->graphicsView->viewport()->height()) / _scene->height();
    setScale(qMin(widthFactor, heightFactor));
}

void MapFrame::zoomSelect(bool enabled)
{
    if(_zoomSelect != enabled)
    {
        _zoomSelect = enabled;
        setMapCursor();
        emit zoomSelectChanged(_zoomSelect);
    }
}

void MapFrame::cancelSelect()
{
    delete _rubberBand;
    _rubberBand = nullptr;
    zoomSelect(false);
    setBrushMode(DMHelper::BrushType_Circle);
}

void MapFrame::setErase(bool enabled)
{
    _erase = enabled;
}

void MapFrame::setSmooth(bool enabled)
{
    _smooth = enabled;
}

void MapFrame::setPublishZoom(bool enabled)
{
    _publishZoom = enabled;
}

void MapFrame::setPublishVisible(bool enabled)
{
    _publishVisible = enabled;
}

void MapFrame::targetResized(const QSize& newSize)
{
    _targetSize = newSize;
    if((_videoPlayer) && (_isPublishing))
    {
        _videoPlayer->targetResized(newSize);
    }

    resetPublishFoW();
}

void MapFrame::publishClicked(bool checked)
{
    if(!_mapSource)
        return;

    _isPublishing = checked;

    if(!_isVideo)
    {
        // Still Image
        QImage pub;
        QPointF partyTopLeft;
        if(_publishZoom)
        {
            QRect imgRect(static_cast<int>(static_cast<qreal>(ui->graphicsView->horizontalScrollBar()->value()) / _scale),
                          static_cast<int>(static_cast<qreal>(ui->graphicsView->verticalScrollBar()->value()) / _scale),
                          static_cast<int>(static_cast<qreal>(ui->graphicsView->viewport()->width()) / _scale),
                          static_cast<int>(static_cast<qreal>(ui->graphicsView->viewport()->height()) / _scale));

            // TODO: Consider zoom factor...

            pub = _mapSource->getPublishImage(imgRect);
            partyTopLeft = _partyIcon->pos() - imgRect.topLeft();
        }
        else
        {
            if(_publishVisible)
            {
                QRect targetRect;
                pub = _mapSource->getShrunkPublishImage(&targetRect);
                partyTopLeft = _partyIcon->pos() - targetRect.topLeft();
            }
            else
            {
                pub = _mapSource->getPublishImage();
                partyTopLeft = _partyIcon->pos();
            }
        }

        if((_mapSource->getShowParty()) && (_mapSource->getParty()))
        {
            QPainter p(&pub);
            QPixmap partyPixmap = _mapSource->getParty()->getIconPixmap(DMHelper::PixmapSize_Battle);
            qreal partyScale = 0.04 * static_cast<qreal>(_mapSource->getPartyScale());
            p.drawPixmap(partyTopLeft, partyPixmap.scaled(partyPixmap.width() * partyScale, partyPixmap.height() * partyScale));
        }

        if(_rotation != 0)
        {
            pub = pub.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
        }

        emit publishImage(pub);
        emit showPublishWindow();
    }
    else
    {
        //Video
        stopPublishTimer();
        createVideoPlayer(!_isPublishing);
        if((_isPublishing) && (_videoPlayer) && (!_videoPlayer->isError()))
        {
            emit animationStarted();
            emit showPublishWindow();
        }

        startPublishTimer();
    }
}

void MapFrame::setRotation(int rotation)
{
    _rotation = rotation;
    rotatePublish();
}

void MapFrame::initializeFoW()
{
    if((_backgroundImage) || (_backgroundVideo) || (_fow) || (_scene))
        qDebug() << "[MapFrame] ERROR: Cleanup of previous map frame contents NOT done. Undefined behavior!";

    _scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(_scene);

    if(!_mapSource)
        return;

    _mapSource->initialize();
    if(_mapSource->isInitialized())
    {
        qDebug() << "[MapFrame] Initializing map frame image";
        _backgroundImage = _scene->addPixmap(QPixmap::fromImage(_mapSource->getBackgroundImage()));
        _backgroundImage->setEnabled(false);
        _backgroundImage->setZValue(-2);

        _fow = _scene->addPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
        _fow->setEnabled(false);
        _fow->setZValue(-1);

        loadViewRect();
    }
    else
    {
        qDebug() << "[MapFrame] Initializing map frame video";
        createVideoPlayer(true);
        startPublishTimer();
    }

    checkPartyUpdate();

    connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    connect(_mapSource, &Map::partyChanged, this, &MapFrame::partyChanged);
    connect(_mapSource, &Map::showPartyChanged, this, &MapFrame::showPartyChanged);
    connect(_mapSource, &Map::partyScaleChanged, this, &MapFrame::partyScaleChanged);
    connect(_mapSource, &Map::partyChanged, this, &MapFrame::dirty);
    connect(_mapSource, &Map::showPartyChanged, this, &MapFrame::dirty);
    connect(_mapSource, &Map::partyScaleChanged, this, &MapFrame::dirty);

    emit partyChanged(_mapSource->getParty());
    emit showPartyChanged(_mapSource->getShowParty());
    emit partyScaleChanged(_mapSource->getPartyScale());

    _isVideo = !_mapSource->isInitialized();
}

void MapFrame::uninitializeFoW()
{
    qDebug() << "[MapFrame] Uninitializing MapFrame...";

    if((_mapSource) && (_partyIcon))
        _mapSource->setPartyIconPos(_partyIcon->pos().toPoint());

    disconnect(_mapSource, &Map::partyChanged, this, &MapFrame::dirty);
    disconnect(_mapSource, &Map::showPartyChanged, this, &MapFrame::dirty);
    disconnect(_mapSource, &Map::partyScaleChanged, this, &MapFrame::dirty);
    disconnect(_mapSource, &Map::partyChanged, this, &MapFrame::partyChanged);
    disconnect(_mapSource, &Map::showPartyChanged, this, &MapFrame::showPartyChanged);
    disconnect(_mapSource, &Map::partyScaleChanged, this, &MapFrame::partyScaleChanged);
    disconnect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    disconnect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));

    stopPublishTimer();
    cleanupBuffers();
    if(_videoPlayer)
    {
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
    }

    delete _partyIcon; _partyIcon = nullptr;
    delete _backgroundImage; _backgroundImage = nullptr;
    delete _backgroundVideo; _backgroundVideo = nullptr;
    delete _fow; _fow = nullptr;

    delete _scene;
    _scene = nullptr;
}

void MapFrame::hideEvent(QHideEvent * event)
{
    cancelSelect();
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
    qDebug() << "[MapFrame] shown (" << isVisible() << ")";
    loadViewRect();
}

void MapFrame::timerEvent(QTimerEvent *event)
{
    if((event) && (_timerId == 0) && (_videoPlayer))
    {
        killTimer(event->timerId());
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
        return;
    }

    if((!_mapSource) || (!_videoPlayer)|| (!_videoPlayer->getImage()) || (_videoPlayer->isError()) || (!_videoPlayer->getMutex()))
        return;

    if(_videoPlayer->isNewImage())
    {
        QMutexLocker locker(_videoPlayer->getMutex());

        if(_isPublishing)
        {
            if(!_videoPlayer->getImage()->isNull())
            {
                if((_bwFoWImage.isNull()) && (!_mapSource->isCleared()))
                {
                    QSize originalSize = _videoPlayer->getOriginalSize();
                    if(!originalSize.isEmpty())
                    {
                        QImage bwImg = _mapSource->getBWFoWImage(originalSize);
                        QSize imgSize = _videoPlayer->getImage()->size();
                        _bwFoWImage = bwImg.scaled(imgSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    }
                }

                QImage result = _videoPlayer->getImage()->copy();
                uchar* b = result.bits();
                memset(b, 0, 100);
                if(!_bwFoWImage.isNull())
                {
                    QPainter p;
                    p.begin(&result);
                        p.drawImage(0, 0, _bwFoWImage);
                    p.end();
                }

                if(_rotation != 0)
                    result = result.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);

                emit animateImage(result);
            }
        }
        else
        {
            if(!_videoPlayer->getImage()->isNull())
            {
                if((!_backgroundVideo) ||(!_backgroundVideo->isVisible()))
                {
                    if(_backgroundImage)
                        _backgroundImage->hide();

                    QPixmap pmpCopy = QPixmap::fromImage(*(_videoPlayer->getImage())).copy();
                    if(_backgroundVideo)
                    {
                        // TODO: update the graphics view/scene to work with the mutex on drawforeground/background
                        _backgroundVideo->setPixmap(pmpCopy);
                        _backgroundVideo->show();
                    }
                    else
                    {
                        _backgroundVideo = _scene->addPixmap(pmpCopy);
                        _backgroundVideo->setEnabled(false);
                        _backgroundVideo->setZValue(-2);
                    }

                    if(_fow)
                    {
                        _fow->setPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
                    }
                    else
                    {
                        QImage fowImage = QImage(_videoPlayer->getImage()->size(), QImage::Format_ARGB32);
                        fowImage.fill(QColor(0,0,0,128));
                        _mapSource->setExternalFoWImage(fowImage);
                        _fow = _scene->addPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
                        _fow->setEnabled(false);
                        _fow->setZValue(1);
                    }

                    loadViewRect();

                    stopPublishTimer();
                    startTimer(500); // Clean up the initial image after a brief period (determined by trial and error)
                }
            }

            update();
        }

        if(_videoPlayer)
            _videoPlayer->clearNewImage();
    }
}

void MapFrame::keyPressEvent(QKeyEvent *event)
{
    if((event) && (((event->key() == Qt::Key_Space)) || ((event->key() == Qt::Key_Control))))
    {
        _spaceDown = true;
        setMapCursor();
        event->accept();
        return;
    }

    CampaignObjectFrame::keyPressEvent(event);
}

void MapFrame::keyReleaseEvent(QKeyEvent *event)
{
    if((event) && (((event->key() == Qt::Key_Space)) || ((event->key() == Qt::Key_Control))))
    {
        _spaceDown = false;
        setMapCursor();
        event->accept();
        return;
    }

    CampaignObjectFrame::keyReleaseEvent(event);
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
        }
        cancelSelect();
        return true;
    }
    else if(event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        _rubberBand->setGeometry(QRect(_mouseDownPos, mouseEvent->pos()).normalized());
        return true;
    }
    else if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape)
        {
            cancelSelect();
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
                UndoShape* undoShape = new UndoShape(*_mapSource, MapEditShape(shapeRect, _erase, false)); //_smooth));
                _mapSource->getUndoStack()->push(undoShape);
                emit dirty();
            }
            cancelSelect();
            return true;
        }
        else if(event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            _rubberBand->setGeometry(QRect(_mouseDownPos, mouseEvent->pos()).normalized());
            return true;
        }
        else if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->key() == Qt::Key_Escape)
            {
                cancelSelect();
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

            _undoPath = new UndoPath(*_mapSource, MapDrawPath(_brushSize, _brushMode, _erase, _smooth, ui->graphicsView->mapToScene(_mouseDownPos).toPoint()));
            _mapSource->getUndoStack()->push(_undoPath);

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
                QPoint localPos =  ui->graphicsView->mapToScene(mouseEvent->pos()).toPoint();
                _undoPath->addPoint(localPos);
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
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if(_mouseDownPos == mouseEvent->pos())
        {
            MapMarkerDialog dlg(QString(""), QString(""), this);
            if(dlg.exec() == QDialog::Accepted)
            {
                UndoMarker* undoMarker = new UndoMarker(*_mapSource, MapMarker(_mouseDownPos, dlg.getTitle(), dlg.getDescription()));
                _mapSource->getUndoStack()->push(undoMarker);
            }
        }
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

void MapFrame::startPublishTimer()
{
    if(!_timerId)
    {
        _timerId = startTimer(DMHelper::ANIMATION_TIMER_DURATION);
    }
}

void MapFrame::stopPublishTimer()
{
    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
    }
}

void MapFrame::createVideoPlayer(bool dmPlayer)
{
    if(!_mapSource)
        return;

    if((!dmPlayer) && (_backgroundVideo))
    {
        QPixmap pmpCopy = _backgroundVideo->pixmap().copy();
        if(!_backgroundImage)
        {
            _backgroundImage = _scene->addPixmap(pmpCopy);
            _backgroundImage->setEnabled(false);
            _backgroundImage->setZValue(-2);
        }
        else
        {
            _backgroundImage->setPixmap(pmpCopy);
            _backgroundImage->show();
        }

        _backgroundVideo->hide();
    }

    if(_videoPlayer)
    {
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
    }

    if(dmPlayer)
    {
        qDebug() << "[MapFrame] Publish FoW DM animation started";
        _videoPlayer = new VideoPlayer(_mapSource->getFileName(), QSize(0, 0), true, false);
    }
    else
    {
        qDebug() << "[MapFrame] Publish FoW Player animation started";
        QSize rotatedSize = (_rotation % 180 == 0) ? _targetSize :  _targetSize.transposed();
        _videoPlayer = new VideoPlayer(_mapSource->getFileName(), rotatedSize, true, _mapSource->getPlayAudio());
        if(!_videoPlayer->isError())
            _bwFoWImage = QImage();
    }
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

    if(_backgroundImage)
    {
        if(_scene)
            _scene->removeItem(_backgroundImage);
        tempItem = _backgroundImage;
        _backgroundImage = nullptr;
        delete tempItem;
    }

    if(_backgroundVideo)
    {
        if(_scene)
            _scene->removeItem(_backgroundVideo);
        tempItem = _backgroundVideo;
        _backgroundVideo = nullptr;
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
    else if(_zoomSelect)
    {
        ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    }
    else
    {
        if(_editMode == DMHelper::EditMode_Move)
        {
            ui->graphicsView->viewport()->setCursor(QCursor(Qt::ArrowCursor));
        }
        else
        {
            if(_brushMode == DMHelper::BrushType_Circle)
                drawEditCursor();
            else if(_brushMode == DMHelper::BrushType_Square)
                drawEditCursor();
            else
                ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
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

void MapFrame::rotatePublish()
{
    resetPublishFoW();
    if((_videoPlayer) && (_isPublishing))
        createVideoPlayer(!_isPublishing);
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

void MapFrame::resetPublishFoW()
{
    if(!_bwFoWImage.isNull())
        _bwFoWImage = QImage();
}

void MapFrame::checkPartyUpdate()
{
    if((!_mapSource) || (!_scene))
        return;

    if((!_mapSource->getShowParty()) || (_mapSource->getPartyId().isNull()))
    {
        delete _partyIcon;
        _partyIcon = nullptr;
        return;
    }

    Party* party = _mapSource->getParty();
    if(!party)
        return;

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
    _partyIcon->setPixmap(party->getIconPixmap(DMHelper::PixmapSize_Battle));
}
