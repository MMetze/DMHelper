#include "mapframe.h"
#include "ui_mapframe.h"
#include "dmconstants.h"
#include "map.h"
#include "mapmarkergraphicsitem.h"
#include "undofill.h"
#include "undomarker.h"
#include "mapmarkerdialog.h"
#include "selectzoom.h"
#include "audiotrack.h"
#include "campaign.h"
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QScrollBar>

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
    _mapSource(nullptr)
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

    // Set up the edit mode button group
    ui->grpEditMode->setId(ui->btnModeFoW, DMHelper::EditMode_FoW);
    ui->grpEditMode->setId(ui->btnModeEdit, DMHelper::EditMode_Edit);
    ui->grpEditMode->setId(ui->btnModeMove, DMHelper::EditMode_Move);
    connect(ui->grpEditMode,SIGNAL(buttonClicked(int)),this,SLOT(editModeToggled(int)));

    // Set up the brush mode button group
    ui->grpBrush->setId(ui->btnBrushCircle, DMHelper::BrushType_Circle);
    ui->grpBrush->setId(ui->btnBrushSquare, DMHelper::BrushType_Square);

    connect(ui->btnPublish,SIGNAL(clicked()),this,SLOT(publishFoWImage()));
    connect(ui->btnClearFoW,SIGNAL(clicked()),this,SLOT(clearFoW()));
    connect(ui->btnResetFoW,SIGNAL(clicked()),this,SLOT(resetFoW()));
    connect(ui->btnShowMarkers,SIGNAL(toggled(bool)),this,SLOT(setViewMarkerVisible(bool)));

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

    setMapCursor();
    setScale(1.0);
}

MapFrame::~MapFrame()
{
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
        _fow->setPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
    }
}

void MapFrame::resetFoW()
{
    if(!_mapSource)
        return;

    UndoFill* undoFill = new UndoFill(*_mapSource, QColor(0,0,0,255));
    _mapSource->getUndoStack()->push(undoFill);
    emit dirty();
}

void MapFrame::clearFoW()
{
    if(!_mapSource)
        return;

    UndoFill* undoFill = new UndoFill(*_mapSource, QColor(0,0,0,0));
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

    emit publishImage(pub);
    emit startTrack(_mapSource->getAudioTrack());
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
    qreal widthFactor = ((qreal)ui->graphicsView->viewport()->width()) / _scene->width();
    qreal heightFactor = ((qreal)ui->graphicsView->viewport()->height()) / _scene->height();
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

    _background = _scene->addPixmap(QPixmap::fromImage(_mapSource->getBackgroundImage()));
    _background->setEnabled(false);
    _background->setZValue(-2);

    _fow = _scene->addPixmap(QPixmap::fromImage(_mapSource->getFoWImage()));
    _fow->setEnabled(false);
    _fow->setZValue(-1);
}

void MapFrame::uninitializeFoW()
{
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

            qreal hScale = ((qreal)(ui->graphicsView->width())) / ((qreal)(target.width()));
            qreal vScale = ((qreal)(ui->graphicsView->height())) / ((qreal)(target.height()));
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

    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        _mouseDownPos = mouseEvent->pos();
        _mouseDown = true;

        _undoPath = new UndoPath(*_mapSource, MapDrawPath(ui->spinBox->value(), ui->grpBrush->checkedId(), ui->btnFoWErase->isChecked(), ui->graphicsView->mapToScene(_mouseDownPos).toPoint()));
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
                else
                {
                    ui->graphicsView->viewport()->setCursor(QCursor(QPixmap(":/img/data/icon_square.png").scaled(ui->spinBox->value()*2*_scale, ui->spinBox->value()*2*_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
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
