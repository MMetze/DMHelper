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

    connect(ui->btnPublishVisible,SIGNAL(clicked(bool)),this,SLOT(publishModeVisibleClicked()));
    connect(ui->btnPublishZoom,SIGNAL(clicked(bool)),this,SLOT(publishModeZoomClicked()));

    connect(ui->grpBrush,SIGNAL(buttonClicked(int)),this,SLOT(setMapCursor()));
    connect(ui->spinBox,SIGNAL(valueChanged(int)),this,SLOT(setMapCursor()));

    //_publishTimer = new QTimer(this);
    //_publishTimer->setSingleShot(false);
    //connect(_publishTimer, SIGNAL(timeout()),this,SLOT(executeAnimateImage()));

    setMapCursor();
    setScale(1.0);
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

void MapFrame::updateFoW()
{
    if((_fow)&&(_mapSource))
    {
        qDebug() << "[MapFrame] Updating ";
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
    else
    {
        stopPublishTimer();
        createVideoPlayer(!ui->framePublish->isChecked());
        startPublishTimer();
    }
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

void MapFrame::targetResized(const QSize& newSize)
{
    _targetSize = newSize;
    if((_videoPlayer) && (ui->framePublish->isChecked()))
    {
        _videoPlayer->targetResized(newSize);
    }
}

void MapFrame::initializeFoW()
{
    delete _background;
    _background = nullptr;
    delete _fow;
    _fow = nullptr;

    delete _scene;
    _scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(_scene);

    cancelSelect();

    if(!_mapSource)
        return;

    _mapSource->initialize();
    if(_mapSource->isInitialized())
    {
        qDebug() << "[MapFrame] Initializing map frame image";
        _background = _scene->addPixmap(QPixmap::fromImage(_mapSource->getBackgroundImage()));
        _background->setEnabled(false);
        _background->setZValue(-2);

        _fow = _scene->addPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
        _fow->setEnabled(false);
        _fow->setZValue(-1);
    }
    else
    {
        qDebug() << "[MapFrame] Initializing map frame video";
        createVideoPlayer(true);
        /*
        if((_videoPlayer) &&!_videoPlayer->isError())
        {
            startPublishTimer();
        }
        */
    }

    ui->framePublish->setCheckable(!_mapSource->isInitialized());
}

void MapFrame::uninitializeFoW()
{
    qDebug() << "[MapFrame] Uninitializing MapFrame...";

    stopPublishTimer();
    if(_videoPlayer)
    {
        delete _videoPlayer;
        _videoPlayer = nullptr;
    }

    cleanupBuffers();
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

    if((!_mapSource) || (!_videoPlayer)|| (!_videoPlayer->getImage()) || (_videoPlayer->isError()) || (!_videoPlayer->getMutex()))
        return;

    if(_videoPlayer->isNewImage())
    {
        QMutexLocker locker(_videoPlayer->getMutex());

        if(ui->framePublish->isChecked())
        {
            if((_bwFoWImage.isNull()) && (!_videoPlayer->getImage()->isNull()))
                _bwFoWImage = _mapSource->getBWFoWImage(*(_videoPlayer->getImage()));

            QImage result = _videoPlayer->getImage()->copy();
            QPainter p;
            p.begin(&result);
                p.drawImage(0, 0, _bwFoWImage);
            p.end();

            if(ui->framePublish->getRotation() != 0)
            {
                result = result.transformed(QTransform().rotate(ui->framePublish->getRotation()), Qt::SmoothTransformation);
            }

            emit animateImage(result);
        }
        else
        {
            if((!_background) && (!_videoPlayer->getImage()->isNull()))
            {
                //QImage testImage = _videoPlayer->getImage()->copy();
                //_background = _scene->addPixmap(QPixmap::fromImage(testImage));
                _background = _scene->addPixmap(QPixmap::fromImage(*(_videoPlayer->getImage())));
                _background->setEnabled(false);
                _background->setZValue(-2);

                QImage fowImage = QImage(_videoPlayer->getImage()->size(), QImage::Format_ARGB32);
                fowImage.fill(QColor(0,0,0,128));
                _mapSource->setExternalFoWImage(fowImage);
                _fow = _scene->addPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
                _fow->setEnabled(false);
                _fow->setZValue(1);
            }

            update();
        }
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
    cleanupBuffers();

    delete _videoPlayer;
    _videoPlayer = nullptr;

    if(dmPlayer)
    {
        qDebug() << "[MapFrame] Publish FoW DM animation started";
        _videoPlayer = new VideoPlayer(QSize(0, 0));
        if((_videoPlayer) && (!_videoPlayer->isError()))
        {
            startPublishTimer();
        }
        //QTimer::singleShot(30, this, SLOT(createDMPlayer()));
    }
    else
    {
        qDebug() << "[MapFrame] Publish FoW Player animation started";
        _videoPlayer = new VideoPlayer(_targetSize);
        _videoPlayer->targetResized(_targetSize);
        if(!_videoPlayer->isError())
        {
            _bwFoWImage = QImage();

            emit animationStarted(ui->framePublish->getColor());
            emit showPublishWindow();
            emit startTrack(_mapSource->getAudioTrack());
        }
        //QTimer::singleShot(30, this, SLOT(createPlayerPlayer()));
    }
}

void MapFrame::cleanupBuffers()
{
    QGraphicsItem* tempItem;

    if(_background)
    {
        _scene->removeItem(_background);
        tempItem = _background;
        _background = nullptr;
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

void MapFrame::trackSelected(int index)
{
    if(!_mapSource)
        return;

    AudioTrack* track = ui->cmbTracks->itemData(index).value<AudioTrack*>();
    _mapSource->setAudioTrack(track);
}

void MapFrame::createDMPlayer()
{
    qDebug() << "[MapFrame] Publish FoW DM animation started";
    _videoPlayer = new VideoPlayer(QSize(0, 0));
    if((_videoPlayer) && (!_videoPlayer->isError()))
    {
        startPublishTimer();
    }
}

void MapFrame::createPlayerPlayer()
{
    qDebug() << "[MapFrame] Publish FoW Player animation started";
    _videoPlayer = new VideoPlayer(_targetSize);
    _videoPlayer->targetResized(_targetSize);
    if(!_videoPlayer->isError())
    {
        _bwFoWImage = QImage();

        emit animationStarted(ui->framePublish->getColor());
        emit showPublishWindow();
        emit startTrack(_mapSource->getAudioTrack());
    }
}
