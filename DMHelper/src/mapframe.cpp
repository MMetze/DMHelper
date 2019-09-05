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
#include <QDebug>

#ifdef ANIMATED_MAPS

// libvlc callback static functions
/**
 * Callback prototype to allocate and lock a picture buffer.
 *
 * Whenever a new video frame needs to be decoded, the lock callback is
 * invoked. Depending on the video chroma, one or three pixel planes of
 * adequate dimensions must be returned via the second parameter. Those
 * planes must be aligned on 32-bytes boundaries.
 *
 * \param opaque private pointer as passed to libvlc_video_set_callbacks() [IN]
 * \param planes start address of the pixel planes (LibVLC allocates the array
 *             of void pointers, this callback must initialize the array) [OUT]
 * \return a private pointer for the display and unlock callbacks to identify
 *         the picture buffers
 */
void * mapFrameLockCallback(void *opaque, void **planes)
{
    if(!opaque)
        return nullptr;

    MapFrame* mapFrame = static_cast<MapFrame*>(opaque);
    return mapFrame->lockCallback(planes);
}

/**
 * Callback prototype to unlock a picture buffer.
 *
 * When the video frame decoding is complete, the unlock callback is invoked.
 * This callback might not be needed at all. It is only an indication that the
 * application can now read the pixel values if it needs to.
 *
 * \note A picture buffer is unlocked after the picture is decoded,
 * but before the picture is displayed.
 *
 * \param opaque private pointer as passed to libvlc_video_set_callbacks() [IN]
 * \param picture private pointer returned from the @ref libvlc_video_lock_cb
 *                callback [IN]
 * \param planes pixel planes as defined by the @ref libvlc_video_lock_cb
 *               callback (this parameter is only for convenience) [IN]
 */
void mapFrameUnlockCallback(void *opaque, void *picture, void *const *planes)
{
    if(!opaque)
        return;

    MapFrame* mapFrame = static_cast<MapFrame*>(opaque);
    mapFrame->unlockCallback(picture, planes);
}

/**
 * Callback prototype to display a picture.
 *
 * When the video frame needs to be shown, as determined by the media playback
 * clock, the display callback is invoked.
 *
 * \param opaque private pointer as passed to libvlc_video_set_callbacks() [IN]
 * \param picture private pointer returned from the @ref libvlc_video_lock_cb
 *                callback [IN]
 */
void mapFrameDisplayCallback(void *opaque, void *picture)
{
    if(!opaque)
        return;

    MapFrame* mapFrame = static_cast<MapFrame*>(opaque);
    mapFrame->displayCallback(picture);
}

/**
 * Callback prototype to configure picture buffers format.
 * This callback gets the format of the video as output by the video decoder
 * and the chain of video filters (if any). It can opt to change any parameter
 * as it needs. In that case, LibVLC will attempt to convert the video format
 * (rescaling and chroma conversion) but these operations can be CPU intensive.
 *
 * \param opaque pointer to the private pointer passed to
 *               libvlc_video_set_callbacks() [IN/OUT]
 * \param chroma pointer to the 4 bytes video format identifier [IN/OUT]
 * \param width pointer to the pixel width [IN/OUT]
 * \param height pointer to the pixel height [IN/OUT]
 * \param pitches table of scanline pitches in bytes for each pixel plane
 *                (the table is allocated by LibVLC) [OUT]
 * \param lines table of scanlines count for each plane [OUT]
 * \return the number of picture buffers allocated, 0 indicates failure
 *
 * \note
 * For each pixels plane, the scanline pitch must be bigger than or equal to
 * the number of bytes per pixel multiplied by the pixel width.
 * Similarly, the number of scanlines must be bigger than of equal to
 * the pixel height.
 * Furthermore, we recommend that pitches and lines be multiple of 32
 * to not break assumptions that might be held by optimized code
 * in the video decoders, video filters and/or video converters.
 */
unsigned mapFrameFormatCallback(void **opaque, char *chroma,
                                unsigned *width, unsigned *height,
                                unsigned *pitches,
                                unsigned *lines)
{
    if((!opaque)||(!(*opaque)))
        return 0;

    MapFrame* mapFrame = static_cast<MapFrame*>(*opaque);
    return mapFrame->formatCallback(chroma, width, height, pitches, lines);
}

/**
 * Callback prototype to configure picture buffers format.
 *
 * \param opaque private pointer as passed to libvlc_video_set_callbacks()
 *               (and possibly modified by @ref libvlc_video_format_cb) [IN]
 */
void mapFrameCleanupCallback(void *opaque)
{
    if(!opaque)
        return;

    MapFrame* mapFrame = static_cast<MapFrame*>(opaque);
    mapFrame->cleanupCallback();
}

#endif

// MapFrame definitions

MapFrame::MapFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapFrame),
    _scene(nullptr),
    _background(nullptr),
    _fow(nullptr),
    _mouseDown(false),
    _mouseDownPos(),
    _undoPath(nullptr),
    _rubberBand(nullptr),
    _scale(1.0),
    //_rotation(0),
    _mapSource(nullptr)
#ifdef ANIMATED_MAPS
    ,
    vlcInstance(nullptr),
    vlcListPlayer(nullptr),
    _nativeWidth(0),
    _nativeHeight(0),
    _nativeBufferNotAligned(nullptr),
    _nativeBuffer(nullptr),
    _loadImage(),
    _newImage(false),
    _timerId(0),
    _publishTimer(nullptr)
#endif
{
    ui->setupUi(this);

    /*
    QLabel* frame = new QLabel(QString("TEST!!!"),ui->frame);
    frame->move(50,50);
    frame->resize(200,200);
    ui->graphicsView->setStyleSheet("background-image: none; background-color: transparent;");
    frame->setPixmap(QPixmap(":/img/data/active.png"));
    //frame->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    frame->setWindowFlags(Qt::Window);
    frame->setAttribute(Qt::WA_TranslucentBackground);
    frame->setAttribute(Qt::WA_NoSystemBackground);
    frame->raise();
    */

    // TODO: reactivate markers
    ui->grpMode->setVisible(false);
    ui->btnModeFoW->setVisible(false);
    ui->btnModeEdit->setVisible(false);
    ui->btnModeMove->setVisible(false);
    ui->btnShowMarkers->setVisible(false);

    _scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(_scene);
    ui->graphicsView->viewport()->installEventFilter(this);
    //ui->graphicsView->setStyleSheet("background: white");
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

    //connect(ui->btnPublish,SIGNAL(clicked()),this,SLOT(publishFoWImage()));
    connect(ui->framePublish,SIGNAL(clicked()),this,SLOT(publishFoWImage()));
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
    connect(ui->btnZoomSelect,SIGNAL(clicked()),this,SLOT(zoomSelect()));

    //connect(ui->btnRotateCCW,SIGNAL(clicked()),this,SLOT(rotateCCW()));
    //connect(ui->btnRotateCW,SIGNAL(clicked()),this,SLOT(rotateCW()));

    connect(ui->btnPublishVisible,SIGNAL(clicked(bool)),this,SLOT(publishModeVisibleClicked()));
    connect(ui->btnPublishZoom,SIGNAL(clicked(bool)),this,SLOT(publishModeZoomClicked()));

    connect(ui->grpBrush,SIGNAL(buttonClicked(int)),this,SLOT(setMapCursor()));
    connect(ui->spinBox,SIGNAL(valueChanged(int)),this,SLOT(setMapCursor()));

#ifdef ANIMATED_MAPS
    _publishTimer = new QTimer(this);
    _publishTimer->setSingleShot(false);
    connect(_publishTimer, SIGNAL(timeout()),this,SLOT(executeAnimateImage()));
#endif

    setMapCursor();
    setScale(1.0);
}

MapFrame::~MapFrame()
{
#ifdef ANIMATED_MAPS
    killTimer(_timerId);

    if(vlcListPlayer)
    {
        libvlc_media_list_player_stop(vlcListPlayer);
        libvlc_media_list_player_release(vlcListPlayer);
    }

    if(vlcInstance)
        libvlc_release(vlcInstance);

    cleanupBuffers();
#endif

    cancelSelect();
    delete ui;
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
    if(ui->btnZoomSelect->isChecked())
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

#ifdef ANIMATED_MAPS

void* MapFrame::lockCallback(void **planes)
{
    if((planes) && (_nativeBuffer))
    {
        *planes = _nativeBuffer;
    }

    return nullptr;
}

void MapFrame::unlockCallback(void *picture, void *const *planes)
{
    return;
}

void MapFrame::displayCallback(void *picture)
{
    if((!_nativeBuffer)  || (_nativeWidth == 0) || (_nativeHeight == 0))
        return;

    _loadImage = QImage(_nativeBuffer, _nativeWidth, _nativeHeight, QImage::Format_RGB32); // QImage::Format_RGB32 works fine as well and it's much faster

    //QImage image(_nativeBuffer, _nativeWidth, _nativeHeight, QImage::Format_RGB32); // QImage::Format_RGB32 works fine as well and it's much faster
    //_background->setPixmap(QPixmap::fromImage(image));
    //update();
    _newImage = true;
}

unsigned MapFrame::formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
    if((!chroma)||(!width)||(!height)||(!pitches)||(!lines))
        return 0;

    if(_nativeBufferNotAligned)
        return 0;

    qDebug() << "[MapFrame] Format Callback with chroma: " << QString(chroma) << ", width: " << *width << ", height: " << *height << ", pitches: " << *pitches << ", lines: " << *lines;

    memcpy(chroma, "RV32", sizeof("RV32") - 1);

    _nativeWidth = *width;
    _nativeHeight = *height;
    *pitches = (*width) * 4;
    *lines = *height;

    _nativeBufferNotAligned = (uchar*)malloc((_nativeWidth * _nativeHeight * 4) + 31);
    _nativeBuffer = (uchar*)((size_t(_nativeBufferNotAligned)+31) & (~31));
    //loadImage = QImage(QSize(_nativeWidth, _nativeHeight), QImage::Format_ARGB32);

    /*
    _loadImage = QImage(QSize(_nativeWidth, _nativeHeight), QImage::Format_RGB32);
    _background = _scene->addPixmap(QPixmap::fromImage(_loadImage));
    _background->setEnabled(false);
    _background->setZValue(-2);
    _newImage = false;

    QImage imgFow = QImage(QSize(_nativeWidth, _nativeHeight), QImage::Format_ARGB32);
    imgFow.fill(QColor(0,0,0,128));
    _fow = _scene->addPixmap(QPixmap::fromImage(imgFow));
    _fow->setEnabled(false);
    _fow->setZValue(1);
    */

    return 1;
}

void MapFrame::cleanupCallback()
{
    qDebug() << "[MapFrame] Cleanup Callback";
    cleanupBuffers();
}

#endif

void MapFrame::updateFoW()
{
    if((_fow)&&(_mapSource))
    {
       _fow->setPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
    }
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

void MapFrame::publishFoWImage()
{
    if(!_mapSource)
        return;

    //if(!ui->btnPublish->isCheckable())
    if(!ui->framePublish->isCheckable())
    {
        QImage pub;
        if(ui->btnPublishZoom->isChecked())
        {
            QRect imgRect(ui->graphicsView->horizontalScrollBar()->value() / _scale,
                          ui->graphicsView->verticalScrollBar()->value() / _scale,
                          ui->graphicsView->viewport()->width() / _scale,
                          ui->graphicsView->viewport()->height() / _scale);

            // TODO: Consider zoom factor...

            pub = _mapSource->getPublishImage(imgRect);
        }
        else
        {
            if(ui->btnPublishVisible->isChecked())
            {
                pub = _mapSource->getShrunkPublishImage();
            }
            else
            {
                pub = _mapSource->getPublishImage();
            }
        }

        if(ui->framePublish->getRotation() != 0)
        {
            pub = pub.transformed(QTransform().rotate(ui->framePublish->getRotation()), Qt::SmoothTransformation);
        }

        emit publishImage(pub, ui->framePublish->getColor());
        emit showPublishWindow();
        emit startTrack(_mapSource->getAudioTrack());
    }
#ifdef ANIMATED_MAPS
    else
    {
        //if(ui->btnPublish->isChecked())
        if(ui->framePublish->isChecked())
        {
            killTimer(_timerId);
            _timerId = 0;
            _bwFoWImage = _mapSource->getBWFoWImage(_loadImage);
            _publishTimer->start(DMHelper::ANIMATION_TIMER_DURATION);
            emit animationStarted(ui->framePublish->getColor());
            emit showPublishWindow();
            emit startTrack(_mapSource->getAudioTrack());
        }
        else
        {
            _publishTimer->stop();
            _timerId = startTimer(0);
        }
    }
#endif
}

void MapFrame::clear()
{
    _mapSource = nullptr;
    delete _background; _background = nullptr;
    delete _fow; _fow = nullptr;
    delete _undoPath; _undoPath = nullptr;
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

void MapFrame::zoomSelect()
{
    ui->btnZoomSelect->setChecked(true);
    setMapCursor();
}

void MapFrame::cancelSelect()
{
    delete _rubberBand;
    _rubberBand = nullptr;
    ui->btnBrushSelect->setChecked(false);
    ui->btnZoomSelect->setChecked(false);
    setMapCursor();
}

void MapFrame::initializeFoW()
{
    if(_background)
    {
        delete _background;
        _background = nullptr;
    }

    if(_fow)
    {
        delete _fow;
        _fow = nullptr;
    }

    if(_scene)
    {
        delete _scene;
        _scene = new QGraphicsScene(this);
        ui->graphicsView->setScene(_scene);
    }

    cancelSelect();

    if(!_mapSource)
        return;

    _mapSource->initialize();
    if(_mapSource->isInitialized())
    {
        _background = _scene->addPixmap(QPixmap::fromImage(_mapSource->getBackgroundImage()));
        _background->setEnabled(false);
        _background->setZValue(-2);

        _fow = _scene->addPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
        _fow->setEnabled(false);
        _fow->setZValue(-1);

        //ui->btnPublish->setCheckable(false);
        ui->framePublish->setCheckable(false);
    }

#ifdef ANIMATED_MAPS

    else
    {
        if(!vlcInstance)
            vlcInstance = libvlc_new(0, nullptr);

        if(!vlcInstance)
            return;

        QString fileOpen = QString("C:\\Users\\Craig\\Documents\\Dnd\\Animated Maps\\Airship_gridLN.m4v");

        // Stop if something is playing
        if (vlcListPlayer && libvlc_media_list_player_is_playing(vlcListPlayer))
            return;

        // Create a new Media List and add the media to it
        libvlc_media_list_t *vlcMediaList = libvlc_media_list_new(vlcInstance);
        if (!vlcMediaList)
            return;

        // Create a new Media
        libvlc_media_t *vlcMedia = libvlc_media_new_path(vlcInstance, fileOpen.toUtf8().constData());
        if (!vlcMedia)
            return;

        libvlc_media_list_add_media(vlcMediaList, vlcMedia);
        libvlc_media_release(vlcMedia);

        // Create a new libvlc player
        vlcListPlayer = libvlc_media_list_player_new(vlcInstance);
        libvlc_media_player_t *player = libvlc_media_player_new(vlcInstance);

        libvlc_media_list_player_set_media_list(vlcListPlayer, vlcMediaList);
        libvlc_media_list_player_set_playback_mode(vlcListPlayer, libvlc_playback_mode_loop);

        libvlc_media_list_player_set_media_player(vlcListPlayer, player);

        unsigned x = 0;
        unsigned y = 0;
        int result = libvlc_video_get_size(player, 0, &x, &y);
        qDebug() << "[MapFrame] Video size (result: " << result << "): " << x << " x " << y << ". File: " << fileOpen;

        // Integrate the video in the interface
        //libvlc_media_player_set_hwnd(player, (void *)ui->graphicsView->winId());
        //libvlc_media_player_set_hwnd(player, (void *)ui->frame->winId());
        //libvlc_media_player_set_hwnd(player, (void *)vlcFrame->winId());

        /*
        // Draw using callbacks
        _nativeWidth = 300;
        _nativeHeight = 300;
        _nativeBufferNotAligned = (uchar*)malloc((_nativeWidth * _nativeHeight * 4) + 31);
        _nativeBuffer = (uchar*)((size_t(_nativeBufferNotAligned)+31) & (~31));
        //loadImage = QImage(QSize(_nativeWidth, _nativeHeight), QImage::Format_ARGB32);

        _loadImage = QImage(QSize(_nativeWidth, _nativeHeight), QImage::Format_RGB32);
        _background = _scene->addPixmap(QPixmap::fromImage(_loadImage));
        _background->setEnabled(false);
        _background->setZValue(-2);
        _newImage = false;

        QImage imgFow = QImage(QSize(_nativeWidth, _nativeHeight), QImage::Format_ARGB32);
        imgFow.fill(QColor(0,0,0,128));
        _fow = _scene->addPixmap(QPixmap::fromImage(imgFow));
        _fow->setEnabled(false);
        _fow->setZValue(1);
        */

        libvlc_video_set_callbacks(player, mapFrameLockCallback, mapFrameUnlockCallback, mapFrameDisplayCallback, this);
        libvlc_video_set_format_callbacks(player, mapFrameFormatCallback, mapFrameCleanupCallback);
        //libvlc_video_set_format(player, "RV32", _nativeWidth, _nativeHeight, _nativeWidth*4);

        // And start playback
        libvlc_media_list_player_play(vlcListPlayer);
        _timerId = startTimer(0);

        //ui->btnPublish->setCheckable(true);
        ui->framePublish->setCheckable(true);
    }
#endif
}

void MapFrame::uninitializeFoW()
{
#ifdef ANIMATED_MAPS
    killTimer(_timerId);
    _timerId = 0;

    if(vlcListPlayer)
    {
        libvlc_media_list_player_stop(vlcListPlayer);
        libvlc_media_list_player_release(vlcListPlayer);
        vlcListPlayer = nullptr;
    }

    cleanupBuffers();
#endif
}

void MapFrame::loadTracks()
{
    if(!_mapSource)
        return;

    disconnect(ui->cmbTracks, SIGNAL(currentIndexChanged(int)), this, SLOT(trackSelected(int)));

    ui->cmbTracks->clear();
    ui->cmbTracks->addItem(QString("---"), QVariant::fromValue(0));

    Campaign* campaign = _mapSource->getCampaign();
    int currentIndex = 0;
    for(int i = 0; i < campaign->getTrackCount(); ++i)
    {
        AudioTrack* track = campaign->getTrackByIndex(i);
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
    QWidget::resizeEvent(event);
}

void MapFrame::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

#ifdef ANIMATED_MAPS
    if(!_mapSource)
        return;

    if(_newImage)
    {
        if(!_background)
        {
            _background = _scene->addPixmap(QPixmap::fromImage(_loadImage));
            _background->setEnabled(false);
            _background->setZValue(-2);

            QImage fowImage = QImage(_loadImage.size(), QImage::Format_ARGB32);
            fowImage.fill(QColor(0,0,0,128));
            _mapSource->setExternalFoWImage(fowImage);
            _fow = _scene->addPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
            _fow->setEnabled(false);
            _fow->setZValue(1);
        }
        else
        {
            _background->setPixmap(QPixmap::fromImage(_loadImage));
        }

        update();
        _newImage = false;
    }
#endif
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

    if(ui->grpBrush->checkedId() == DMHelper::BrushType_Select)
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
                UndoShape* undoShape = new UndoShape(*_mapSource, MapEditShape(shapeRect, ui->btnFoWErase->isChecked(), ui->chkSmooth->isChecked()));
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

            _undoPath = new UndoPath(*_mapSource, MapDrawPath(ui->spinBox->value(), ui->grpBrush->checkedId(), ui->btnFoWErase->isChecked(), ui->chkSmooth->isChecked(), ui->graphicsView->mapToScene(_mouseDownPos).toPoint()));
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

void MapFrame::cleanupBuffers()
{
    delete _background;
    _background = nullptr;
    delete _fow;
    _fow = nullptr;

#ifdef ANIMATED_MAPS
    if(_nativeBufferNotAligned)
    {
        free(_nativeBufferNotAligned);
        _nativeBufferNotAligned = nullptr;
        _nativeBuffer = nullptr;
    }
#endif
}

void MapFrame::setMapCursor()
{
    if(ui->btnZoomSelect->isChecked())
    {
        ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    }
    else
    {
        switch(ui->grpEditMode->checkedId())
        {
            case DMHelper::EditMode_FoW:
                if(ui->grpBrush->checkedId() == DMHelper::BrushType_Circle)
                {
                    ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/icon_circle.png").scaled(ui->spinBox->value()*2*_scale, ui->spinBox->value()*2*_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                }
                else if(ui->grpBrush->checkedId() == DMHelper::BrushType_Square)
                {
                    ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/icon_square.png").scaled(ui->spinBox->value()*2*_scale, ui->spinBox->value()*2*_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
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

void MapFrame::setScale(qreal s)
{
    _scale = s;
    ui->graphicsView->setTransform(QTransform::fromScale(_scale,_scale));
    setMapCursor();
}

/*
void MapFrame::rotateCCW()
{
    _rotation -= 90;
}

void MapFrame::rotateCW()
{
    _rotation += 90;
}
*/

#ifdef ANIMATED_MAPS
void MapFrame::executeAnimateImage()
{
    if(!_mapSource)
        return;

    QImage result(_loadImage);
    QPainter p;
    p.begin(&result);
        p.drawImage(0, 0, _bwFoWImage);
    p.end();

    emit animateImage(result);
}
#endif

void MapFrame::trackSelected(int index)
{
    if(!_mapSource)
        return;

    AudioTrack* track = ui->cmbTracks->itemData(index).value<AudioTrack*>();
    _mapSource->setAudioTrack(track);
}
