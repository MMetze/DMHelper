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
#include "audiotrack.h"
#include "campaign.h"
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QMutexLocker>
#include <QDebug>

// MapFrame definitions

MapFrame::MapFrame(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::MapFrame),
    _scene(nullptr),
    _backgroundImage(nullptr),
    _backgroundVideo(nullptr),
    _fow(nullptr),
    _erase(true),
    _smooth(false),
    _brushMode(DMHelper::BrushType_Circle),
    _brushSize(30),
    _publishZoom(false),
    _publishVisible(false),
    _isPublishing(false),
    _isVideo(false),
    _rotation(0),
    //_color(Qt::black),
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
    ui->grpMode->setVisible(false);
    ui->btnModeFoW->setVisible(false);
    ui->btnModeEdit->setVisible(false);
    ui->btnModeMove->setVisible(false);
    ui->btnShowMarkers->setVisible(false);

    _scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(_scene);
    ui->graphicsView->viewport()->installEventFilter(this);
    ui->graphicsView->setStyleSheet("background-color: transparent;");

    // Set up the edit mode button group
    ui->grpEditMode->setId(ui->btnModeFoW, DMHelper::EditMode_FoW);
    ui->grpEditMode->setId(ui->btnModeEdit, DMHelper::EditMode_Edit);
    ui->grpEditMode->setId(ui->btnModeMove, DMHelper::EditMode_Move);
    connect(ui->grpEditMode,SIGNAL(buttonClicked(int)),this,SLOT(editModeToggled(int)));

    // Set up the brush mode button group
    ui->grpBrush->setId(ui->btnBrushCircle, DMHelper::BrushType_Circle);
    ui->grpBrush->setId(ui->btnBrushSquare, DMHelper::BrushType_Square);
    ui->grpBrush->setId(ui->btnBrushSelect, DMHelper::BrushType_Select);

    //connect(ui->framePublish,SIGNAL(clicked()),this,SLOT(publishFoWImage()));
    connect(ui->framePublish, SIGNAL(rotateCW()), this, SLOT(rotatePublish()));
    connect(ui->framePublish, SIGNAL(rotateCCW()), this, SLOT(rotatePublish()));
    connect(ui->btnClearFoW,SIGNAL(clicked()),this,SLOT(clearFoW()));
    connect(ui->btnResetFoW,SIGNAL(clicked()),this,SLOT(resetFoW()));
    //TODO Markers: connect(ui->btnShowMarkers,SIGNAL(toggled(bool)),this,SLOT(setViewMarkerVisible(bool)));

    connect(ui->btnZoomIn,SIGNAL(clicked()),this,SLOT(zoomIn()));
    connect(ui->btnZoomIn,SIGNAL(clicked()),this,SLOT(cancelSelect()));
    connect(ui->btnZoomOut,SIGNAL(clicked()),this,SLOT(zoomOut()));
    connect(ui->btnZoomOut,SIGNAL(clicked()),this,SLOT(cancelSelect()));
    connect(ui->btnZoomOne,SIGNAL(clicked()),this,SLOT(zoomOne()));
    connect(ui->btnZoomOne,SIGNAL(clicked()),this,SLOT(cancelSelect()));
    connect(ui->btnZoomFit,SIGNAL(clicked()),this,SLOT(zoomFit()));
    connect(ui->btnZoomFit,SIGNAL(clicked()),this,SLOT(cancelSelect()));
    //connect(ui->btnZoomSelect,SIGNAL(clicked()),this,SLOT(zoomSelect()));

    connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(storeViewRect()));

    //connect(ui->btnPublishVisible,SIGNAL(clicked(bool)),this,SLOT(publishModeVisibleClicked()));
    //connect(ui->btnPublishZoom,SIGNAL(clicked(bool)),this,SLOT(publishModeZoomClicked()));

    connect(ui->grpBrush,SIGNAL(buttonClicked(int)),this,SLOT(setMapCursor()));
    connect(ui->spinBox,SIGNAL(valueChanged(int)),this,SLOT(setMapCursor()));

    connect(this, SIGNAL(dirty()), this, SLOT(resetPublishFoW()));

    connect(ui->chkAudio, SIGNAL(clicked()), this, SLOT(audioPlaybackChecked()));

    //_publishTimer = new QTimer(this);
    //_publishTimer->setSingleShot(false);
    //connect(_publishTimer, SIGNAL(timeout()),this,SLOT(executeAnimateImage()));

    setMapCursor();
    //setScale(1.0);

    /***** TODO: Remove this when finalized ****/
    ui->grpMode->hide();
    ui->groupBox_5->hide();
    ui->groupBox_2->hide();
    ui->btnShowMarkers->hide();
    ui->groupBox_4->hide();
    ui->chkAudio->hide();
    ui->label_2->hide();
    ui->cmbTracks->hide();
    ui->groupBox_3->hide();
    ui->framePublish->hide();
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
    loadTracks();
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
    //if(ui->btnZoomSelect->isChecked())
    if(_zoomSelect)
    {
        if(execEventFilterSelectZoom(obj, event))
            return true;
    }
    else
    {
        switch(ui->grpEditMode->checkedId())
        {
            case DMHelper::EditMode_FoW:
                if(execEventFilterEditModeFoW(obj, event))
                    return true;
                break;
            case DMHelper::EditMode_Edit:
                if(execEventFilterEditModeEdit(obj, event))
                    return true;
                break;
            case DMHelper::EditMode_Move:
                if(execEventFilterEditModeMove(obj, event))
                    return true;
                break;
            default:
                break;
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

/*
void MapFrame::publishFoWImage(bool publishing)
{
    if(!_mapSource)
        return;

    _isPublishing = publishing;

    //if(!ui->framePublish->isCheckable())
    if(!_isVideo)
    {
        // Still Image
        QImage pub;
        //if(ui->btnPublishZoom->isChecked())
        if(_publishZoom)
        {
            QRect imgRect(static_cast<int>(static_cast<qreal>(ui->graphicsView->horizontalScrollBar()->value()) / _scale),
                          static_cast<int>(static_cast<qreal>(ui->graphicsView->verticalScrollBar()->value()) / _scale),
                          static_cast<int>(static_cast<qreal>(ui->graphicsView->viewport()->width()) / _scale),
                          static_cast<int>(static_cast<qreal>(ui->graphicsView->viewport()->height()) / _scale));

            // TODO: Consider zoom factor...

            pub = _mapSource->getPublishImage(imgRect);
        }
        else
        {
            //if(ui->btnPublishVisible->isChecked())
            if(_publishVisible)
            {
                pub = _mapSource->getShrunkPublishImage();
            }
            else
            {
                pub = _mapSource->getPublishImage();
            }
        }

        //if(ui->framePublish->getRotation() != 0)
        if(_rotation != 0)
        {
            //pub = pub.transformed(QTransform().rotate(ui->framePublish->getRotation()), Qt::SmoothTransformation);
            pub = pub.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
        }

        //emit publishImage(pub, ui->framePublish->getColor());
        emit publishImage(pub, _color);
        emit showPublishWindow();
        startAudioTrack();
    }
    else
    {
        //Video
        stopPublishTimer();
        //createVideoPlayer(!ui->framePublish->isChecked());
        createVideoPlayer(!_isPublishing);
        //if((ui->framePublish->isChecked()) && (_videoPlayer) && (!_videoPlayer->isError()))
        if((_isPublishing) && (_videoPlayer) && (!_videoPlayer->isError()))
        {
            //emit animationStarted(ui->framePublish->getColor());
            //emit animationStarted(_color);
            emit animationStarted();
            emit showPublishWindow();
            startAudioTrack();
        }

        startPublishTimer();
    }
}
*/

void MapFrame::clear()
{
    _mapSource = nullptr;
    delete _backgroundImage; _backgroundImage = nullptr;
    delete _backgroundVideo; _backgroundVideo = nullptr;
    delete _fow; _fow = nullptr;
    delete _undoPath; _undoPath = nullptr;
}

void MapFrame::cancelPublish()
{
    //ui->framePublish->cancelPublish();
    emit publishCancelled();
    _isPublishing = false;
}

void MapFrame::editModeToggled(int editMode)
{
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
    //ui->btnZoomSelect->setChecked(true);
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
    //ui->btnBrushSelect->setChecked(false);
    //ui->btnZoomSelect->setChecked(false);
    //_zoomSelect = false;
    //setMapCursor();
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
    //if((_videoPlayer) && (ui->framePublish->isChecked()))
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

    //if(!ui->framePublish->isCheckable())
    if(!_isVideo)
    {
        // Still Image
        QImage pub;
        //if(ui->btnPublishZoom->isChecked())
        if(_publishZoom)
        {
            QRect imgRect(static_cast<int>(static_cast<qreal>(ui->graphicsView->horizontalScrollBar()->value()) / _scale),
                          static_cast<int>(static_cast<qreal>(ui->graphicsView->verticalScrollBar()->value()) / _scale),
                          static_cast<int>(static_cast<qreal>(ui->graphicsView->viewport()->width()) / _scale),
                          static_cast<int>(static_cast<qreal>(ui->graphicsView->viewport()->height()) / _scale));

            // TODO: Consider zoom factor...

            pub = _mapSource->getPublishImage(imgRect);
        }
        else
        {
            //if(ui->btnPublishVisible->isChecked())
            if(_publishVisible)
            {
                pub = _mapSource->getShrunkPublishImage();
            }
            else
            {
                pub = _mapSource->getPublishImage();
            }
        }

        //if(ui->framePublish->getRotation() != 0)
        if(_rotation != 0)
        {
            //pub = pub.transformed(QTransform().rotate(ui->framePublish->getRotation()), Qt::SmoothTransformation);
            pub = pub.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
        }

        //emit publishImage(pub, ui->framePublish->getColor());
        //emit publishImage(pub, _color);
        emit publishImage(pub);
        emit showPublishWindow();
        startAudioTrack();
    }
    else
    {
        //Video
        stopPublishTimer();
        //createVideoPlayer(!ui->framePublish->isChecked());
        createVideoPlayer(!_isPublishing);
        //if((ui->framePublish->isChecked()) && (_videoPlayer) && (!_videoPlayer->isError()))
        if((_isPublishing) && (_videoPlayer) && (!_videoPlayer->isError()))
        {
            //emit animationStarted(ui->framePublish->getColor());
            //emit animationStarted(_color);
            emit animationStarted();
            emit showPublishWindow();
            startAudioTrack();
        }

        startPublishTimer();
    }
}

void MapFrame::setRotation(int rotation)
{
    _rotation = rotation;
    rotatePublish();
}

/*
void MapFrame::setBackgroundColor(QColor color)
{
    _color = color;
}
*/

void MapFrame::initializeFoW()
{
    delete _backgroundImage;
    _backgroundImage = nullptr;
    delete _backgroundVideo;
    _backgroundVideo = nullptr;
    delete _fow;
    _fow = nullptr;

    delete _scene;
    _scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(_scene);

    cancelSelect();
    //ui->framePublish->cancelPublish();

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

        //ui->graphicsView->fitInView(_mapSource->getMapRect(), Qt::KeepAspectRatio);
        loadViewRect();
    }
    else
    {
        qDebug() << "[MapFrame] Initializing map frame video";
        createVideoPlayer(true);
        startPublishTimer();
    }

    _isVideo = !_mapSource->isInitialized();
}

void MapFrame::uninitializeFoW()
{
    qDebug() << "[MapFrame] Uninitializing MapFrame...";

    stopPublishTimer();
    cleanupBuffers();
    if(_videoPlayer)
    {
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
    }
}

void MapFrame::loadTracks()
{
    if(!_mapSource)
        return;

    disconnect(ui->cmbTracks, SIGNAL(currentIndexChanged(int)), this, SLOT(trackSelected(int)));

    ui->cmbTracks->clear();
    ui->cmbTracks->addItem(QString("---"), QVariant::fromValue(0));

    //Campaign* campaign = _mapSource->getCampaign();
    Campaign* campaign = dynamic_cast<Campaign*>(_mapSource->getParentByType(DMHelper::CampaignType_Campaign));
    int currentIndex = 0;
    QList<AudioTrack*> trackList = campaign->findChildren<AudioTrack*>();
    //for(int i = 0; i < campaign->getTrackCount(); ++i)
    for(AudioTrack* track : trackList)
    {
        //AudioTrack* track = campaign->getTrackByIndex(i);
        ui->cmbTracks->addItem(track->getName(), QVariant::fromValue(track));
        if(_mapSource->getAudioTrackId() == track->getID())
            currentIndex = ui->cmbTracks->count() - 1;
    }
    ui->cmbTracks->setCurrentIndex(currentIndex);

    connect(ui->cmbTracks, SIGNAL(currentIndexChanged(int)), this, SLOT(trackSelected(int)));
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

        //if(ui->framePublish->isChecked())
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
                if(!_bwFoWImage.isNull())
                {
                    QPainter p;
                    p.begin(&result);
                        p.drawImage(0, 0, _bwFoWImage);
                    p.end();
                }

                //if(ui->framePublish->getRotation() != 0)
                if(_rotation != 0)
                {
                    //result = result.transformed(QTransform().rotate(ui->framePublish->getRotation()), Qt::SmoothTransformation);
                    result = result.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
                }

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
                    {
                        _backgroundImage->hide();
                    }

                    QPixmap pmpCopy = QPixmap::fromImage(*(_videoPlayer->getImage())).copy();
                    if(_backgroundVideo)
                    {
                        //_backgroundVideo->setPixmap(QPixmap::fromImage(*(_videoPlayer->getImage())));
                        // TODO: update the graphics view/scene to work with the mutex on drawforeground/background
                        _backgroundVideo->setPixmap(pmpCopy);
                        _backgroundVideo->show();
                    }
                    else
                    {
                        //_backgroundVideo = _scene->addPixmap(QPixmap::fromImage(*(_videoPlayer->getImage())));
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

                    //ui->graphicsView->fitInView(_mapSource->getMapRect(), Qt::KeepAspectRatio);
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

    //if(ui->grpBrush->checkedId() == DMHelper::BrushType_Select)
    if(_brushMode == DMHelper::BrushType_Select)
    {
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
                QRect bandRect = _rubberBand->rect();
                bandRect.moveTo(_rubberBand->pos());
                QRect shapeRect(ui->graphicsView->mapToScene(bandRect.topLeft()).toPoint(),
                                ui->graphicsView->mapToScene(bandRect.bottomRight()).toPoint());
                UndoShape* undoShape = new UndoShape(*_mapSource, MapEditShape(shapeRect, _erase, _smooth));
                _mapSource->getUndoStack()->push(undoShape);
                emit dirty();
            }
            //cancelSelect();
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

            //_undoPath = new UndoPath(*_mapSource, MapDrawPath(ui->spinBox->value(), ui->grpBrush->checkedId(), _erase, _smooth, ui->graphicsView->mapToScene(_mouseDownPos).toPoint()));
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
        /*
        if((_videoPlayer) && (!_videoPlayer->isError()))
        {
            startPublishTimer();
        }
        */
    }
    else
    {
        qDebug() << "[MapFrame] Publish FoW Player animation started";
        //QSize rotatedSize = (ui->framePublish->getRotation() % 180 == 0) ? _targetSize :  _targetSize.transposed();
        QSize rotatedSize = (_rotation % 180 == 0) ? _targetSize :  _targetSize.transposed();
        _videoPlayer = new VideoPlayer(_mapSource->getFileName(), rotatedSize, true, _mapSource->getPlayAudio());
        //_videoPlayer->targetResized(rotatedSize);
        if(!_videoPlayer->isError())
        {
            _bwFoWImage = QImage();
        }
    }
}

void MapFrame::cleanupBuffers()
{
    QGraphicsItem* tempItem;

    if(_backgroundImage)
    {
        _scene->removeItem(_backgroundImage);
        tempItem = _backgroundImage;
        _backgroundImage = nullptr;
        delete tempItem;
    }

    if(_backgroundVideo)
    {
        _scene->removeItem(_backgroundVideo);
        tempItem = _backgroundVideo;
        _backgroundVideo = nullptr;
        delete tempItem;
    }

    if(_fow)
    {
        _scene->removeItem(_fow);
        tempItem = _fow;
        _fow = nullptr;
        delete tempItem;
    }

    _scene->clear();
    _scene->update();
}

void MapFrame::startAudioTrack()
{
    // TODO: FIX THIS!
//    if((_mapSource) && (_mapSource->getPlayAudio()))
//        emit startTrack(_mapSource->getAudioTrack());
}

void MapFrame::setMapCursor()
{
    //if(ui->btnZoomSelect->isChecked())
    if(_zoomSelect)
    {
        ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    }
    else
    {
        switch(ui->grpEditMode->checkedId())
        {
            case DMHelper::EditMode_FoW:
                //if(ui->grpBrush->checkedId() == DMHelper::BrushType_Circle)
                if(_brushMode == DMHelper::BrushType_Circle)
                {
                    //ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/icon_circle.png").scaled(ui->spinBox->value()*2*_scale, ui->spinBox->value()*2*_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                    ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/icon_circle.png").scaled(_brushSize*2*_scale, _brushSize*2*_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                }
                //else if(ui->grpBrush->checkedId() == DMHelper::BrushType_Square)
                else if(_brushMode == DMHelper::BrushType_Square)
                {
                    //ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/icon_square.png").scaled(ui->spinBox->value()*2*_scale, ui->spinBox->value()*2*_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                    ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/icon_square.png").scaled(_brushSize*2*_scale, _brushSize*2*_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                }
                else
                {
                    ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                }
                break;
            case DMHelper::EditMode_Edit:
                ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                break;
            case DMHelper::EditMode_Move:
            default:
                ui->graphicsView->viewport()->unsetCursor();
                break;
        }
    }
}

/*
void MapFrame::publishModeVisibleClicked()
{
    if(ui->btnPublishVisible->isChecked())
    {
        ui->btnPublishZoom->setChecked(false);
    }
}

void MapFrame::publishModeZoomClicked()
{
    if(ui->btnPublishZoom->isChecked())
    {
        ui->btnPublishVisible->setChecked(false);
    }
}
*/

void MapFrame::rotatePublish()
{
    resetPublishFoW();
    //if((_videoPlayer) && (ui->framePublish->isChecked()))
    if((_videoPlayer) && (_isPublishing))
    {
        //createVideoPlayer(!ui->framePublish->isChecked());
        createVideoPlayer(!_isPublishing);
    }
}

void MapFrame::trackSelected(int index)
{
    if(!_mapSource)
        return;

    AudioTrack* track = ui->cmbTracks->itemData(index).value<AudioTrack*>();
    _mapSource->setAudioTrack(track);
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
        ui->graphicsView->fitInView(_mapSource->getMapRect(), Qt::KeepAspectRatio);
    }
    else
    {
        zoomFit();
    }
}

void MapFrame::resetPublishFoW()
{
    if(!_bwFoWImage.isNull())
    {
        _bwFoWImage = QImage();
    }
}

void MapFrame::audioPlaybackChecked()
{
    if(_mapSource)
        _mapSource->setPlayAudio(ui->chkAudio->isChecked());

    //if((ui->framePublish->isChecked()) && (_videoPlayer))
    if((_isPublishing) && (_videoPlayer))
        _videoPlayer->setPlayingAudio(ui->chkAudio->isChecked());
}

