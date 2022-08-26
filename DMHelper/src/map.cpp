#include "map.h"
#include "undobase.h"
#include "undofill.h"
#include "undopath.h"
#include "undopoint.h"
#include "undoshape.h"
#include "undomarker.h"
#include "dmconstants.h"
#include "campaign.h"
#include "audiotrack.h"
#include "party.h"
#include "dmhcache.h"
#include <QDomDocument>
#include <QDomElement>
#include <QUndoStack>
#include <QDir>
#include <QPainter>
#include <QImageReader>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

Map::Map(const QString& mapName, const QString& fileName, QObject *parent) :
    CampaignObjectBase(mapName, parent),
    _filename(fileName),
    _undoStack(nullptr),
    _audioTrackId(),
    _playAudio(false),
    _mapRect(),
    _cameraRect(),
    _showPartyIcon(false),
    _partyId(),
    _partyIconPos(-1, -1),
    _partyScale(10),
    _mapScale(100),
    _showMarkers(true),
    _mapItems(),
    _initialized(false),
    _layerScene(),
    //_imgBackground(),
    _imgFow(),
    _imgBWFow(),
    _indexBWFow(0),
    _filterApplied(false),
    _filter(),
    _lineType(Qt::SolidLine),
    _lineColor(Qt::yellow),
    _lineWidth(1),
    _mapColor(Qt::white),
    _mapSize()
{
    _undoStack = new QUndoStack(this);
}

Map::~Map()
{
    qDeleteAll(_mapItems);
    _layerScene.clearLayers();
}

void Map::inputXML(const QDomElement &element, bool isImport)
{
    //TODO: include layers in import/export (with backwards compatibility)

    // For backwards compatibility only, should be part of a layer
    _filename = element.attribute("filename"); // Even if it can't be found, don't want to lose the data
    if(_filename == QString(".")) // In case the map file is this trivial, it can be ignored
        _filename.clear();

    setDistanceLineColor(QColor(element.attribute("lineColor", QColor(Qt::yellow).name())));
    setDistanceLineType(element.attribute("lineType", QString::number(Qt::SolidLine)).toInt());
    setDistanceLineWidth(element.attribute("lineWidth", QString::number(1)).toInt());
    setMapColor(QColor(element.attribute("mapColor")));
    setMapSize(QSize(element.attribute("mapSizeWidth", QString::number(0)).toInt(),
                     element.attribute("mapSizeHeight", QString::number(0)).toInt()));
    _mapRect = QRect(element.attribute("mapRectX", QString::number(0)).toInt(),
                     element.attribute("mapRectY", QString::number(0)).toInt(),
                     element.attribute("mapRectWidth", QString::number(0)).toInt(),
                     element.attribute("mapRectHeight", QString::number(0)).toInt());
    _cameraRect = QRect(element.attribute("cameraRectX", QString::number(0)).toInt(),
                        element.attribute("cameraRectY", QString::number(0)).toInt(),
                        element.attribute("cameraRectWidth", QString::number(0)).toInt(),
                        element.attribute("cameraRectHeight ", QString::number(0)).toInt());

    QDomElement actionsElement = element.firstChildElement(QString("actions"));
    if(!actionsElement.isNull())
    {
        QDomElement actionElement = actionsElement.firstChildElement(QString("action"));
        while(!actionElement.isNull())
        {
            UndoBase* newAction = nullptr;
            switch( actionElement.attribute(QString("type")).toInt())
            {
                case DMHelper::ActionType_Fill:
                    newAction = new UndoFill(this, MapEditFill(QColor()));
                    break;
                case DMHelper::ActionType_Path:
                    newAction = new UndoPath(this, MapDrawPath());
                    break;
                case DMHelper::ActionType_Point:
                    newAction = new UndoPoint(this, MapDrawPoint(0, DMHelper::BrushType_Circle, true, true, QPoint()));
                    break;
                case DMHelper::ActionType_Rect:
                    newAction = new UndoShape(this, MapEditShape(QRect(), true, true));
                    break;
                case DMHelper::ActionType_SetMarker:
                    newAction = new UndoMarker(this, MapMarker());
                    break;
                case DMHelper::ActionType_Base:
                default:
                    break;
            }

            if(newAction)
            {
                newAction->inputXML(actionElement, isImport);
                _undoStack->push(newAction);
            }

            actionElement = actionElement.nextSiblingElement(QString("action"));
        }
    }

    // For backwards compatibility only, should be part of a layer
    QDomElement filterElement = element.firstChildElement(QString("filter"));
    if(!filterElement.isNull())
    {
        _filterApplied = true;

        _filter._r2r = filterElement.attribute("r2r",QString::number(1.0)).toDouble();
        _filter._g2r = filterElement.attribute("g2r",QString::number(0.0)).toDouble();
        _filter._b2r = filterElement.attribute("b2r",QString::number(0.0)).toDouble();
        _filter._r2g = filterElement.attribute("r2g",QString::number(0.0)).toDouble();
        _filter._g2g = filterElement.attribute("g2g",QString::number(1.0)).toDouble();
        _filter._b2g = filterElement.attribute("b2g",QString::number(0.0)).toDouble();
        _filter._r2b = filterElement.attribute("r2b",QString::number(0.0)).toDouble();
        _filter._g2b = filterElement.attribute("g2b",QString::number(0.0)).toDouble();
        _filter._b2b = filterElement.attribute("b2b",QString::number(1.0)).toDouble();
        _filter._sr = filterElement.attribute("sr",QString::number(1.0)).toDouble();
        _filter._sg = filterElement.attribute("sg",QString::number(1.0)).toDouble();
        _filter._sb = filterElement.attribute("sb",QString::number(1.0)).toDouble();

        _filter._isOverlay = static_cast<bool>(filterElement.attribute("isOverlay",QString::number(1)).toInt());
        _filter._overlayColor.setNamedColor(filterElement.attribute("overlayColor",QString("#000000")));
        _filter._overlayAlpha = filterElement.attribute("overlayAlpha",QString::number(128)).toInt();
    }

    CampaignObjectBase::inputXML(element, isImport);
}

void Map::copyValues(const CampaignObjectBase* other)
{
    const Map* otherMap = dynamic_cast<const Map*>(other);
    if(!otherMap)
        return;

    _filename = otherMap->_filename;
    _audioTrackId = otherMap->getAudioTrackId();
    _playAudio = otherMap->getPlayAudio();
    _mapRect = otherMap->getMapRect();
    _cameraRect = otherMap->getCameraRect();
    _showPartyIcon = otherMap->getShowParty();
    _partyId = otherMap->getPartyId();
    _partyAltIcon = otherMap->getPartyAltIcon();
    _partyIconPos = otherMap->getPartyIconPos();
    _partyScale = otherMap->getPartyScale();
    _mapScale = otherMap->getMapScale();

    _showMarkers = otherMap->getShowMarkers();

    setDistanceLineType(otherMap->getDistanceLineType());
    setDistanceLineColor(otherMap->getDistanceLineColor());
    setDistanceLineWidth(otherMap->getDistanceLineWidth());

    setMapColor(otherMap->getMapColor());
    setMapSize(otherMap->getMapSize());

    _undoStack->clear();
    for(int i = 0; i < otherMap->getUndoStack()->index(); ++i )
    {
        const UndoBase* action = dynamic_cast<const UndoBase*>(otherMap->getUndoStack()->command(i));
        if((action) && (!action->isRemoved()))
        {
            UndoBase* newAction = action->clone();
            newAction->setMap(this);
            _undoStack->push(newAction);
        }
    }

    // Check if we can skip some paint commands because they have been covered up by a fill
    challengeUndoStack();

    _filterApplied = otherMap->_filterApplied;
    _filter = otherMap->_filter;

    CampaignObjectBase::copyValues(other);
}

int Map::getObjectType() const
{
    return DMHelper::CampaignType_Map;
}

/*
const QImage& Map::getImage() const
{
    return _imgBackground;
}
*/

QString Map::getFileName() const
{
    return _filename;
}

bool Map::setFileName(const QString& newFileName)
{
    if((_filename == newFileName) || (newFileName.isEmpty()))
        return true;

    if(!QFile::exists(newFileName))
    {
        QMessageBox::critical(nullptr,
                              QString("DMHelper Map File Not Found"),
                              QString("The new map file could not be found: ") + newFileName + QString(", keeping map file: ") + _filename + QString(" for entry: ") + getName());
        qDebug() << "[Map] setFileName - New map file not found: " << newFileName << " for entry " << getName();
        return false;
    }

    QFileInfo fileInfo(newFileName);
    if(!fileInfo.isFile())
    {
        QMessageBox::critical(nullptr,
                              QString("DMHelper Map File Not Valid"),
                              QString("The new map isn't a file: ") + newFileName + QString(", keeping map file: ") + _filename + QString(" for entry: ") + getName());
        qDebug() << "[Map] setFileName - Map file not a file: " << newFileName << " for entry " << getName();
        return false;
    }

    if(_initialized)
    {
        qDebug() << "[Map] Cannot set new map file, map is initialized and in use! Old: " << _filename << ", New: " << newFileName;
        return true;
    }

    _filename = newFileName;
    emit dirty();

    return true;
}

QColor Map::getMapColor() const
{
    return _mapColor;
}

void Map::setMapColor(const QColor& color)
{
    _mapColor = color;
}

QSize Map::getMapSize() const
{
    return _mapSize;
}

void Map::setMapSize(QSize size)
{
    _mapSize = size;
}

AudioTrack* Map::getAudioTrack()
{
    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return nullptr;

    return campaign->getTrackById(_audioTrackId);
}

QUuid Map::getAudioTrackId() const
{
    return _audioTrackId;
}

void Map::setAudioTrack(AudioTrack* track)
{
    QUuid newTrackId = (track == nullptr) ? QUuid() : track->getID();
    if(_audioTrackId != newTrackId)
    {
        _audioTrackId = newTrackId;
        emit dirty();
    }
}

bool Map::getPlayAudio() const
{
    return _playAudio;
}

void Map::setPlayAudio(bool playAudio)
{
    if(_playAudio != playAudio)
    {
        _playAudio = playAudio;
        emit dirty();
    }
}

Party* Map::getParty()
{
    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return nullptr;

    return dynamic_cast<Party*>(campaign->getObjectById(_partyId));
}

QString Map::getPartyAltIcon() const
{
    return _partyAltIcon;
}

QUuid Map::getPartyId() const
{
    return _partyId;
}

bool Map::getShowParty() const
{
    return _showPartyIcon;
}

const QPoint& Map::getPartyIconPos() const
{
    return _partyIconPos;
}

int Map::getPartyScale() const
{
    return _partyScale;
}

QPixmap Map::getPartyPixmap()
{
    QPixmap partyPixmap;

    if(getParty())
    {
        partyPixmap = getParty()->getIconPixmap(DMHelper::PixmapSize_Battle);
    }
    else
    {
        if(partyPixmap.load(getPartyAltIcon()))
            partyPixmap = partyPixmap.scaled(DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0],
                                             DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][1],
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
    }

    return partyPixmap;
}

int Map::getDistanceLineType() const
{
    return _lineType;
}

QColor Map::getDistanceLineColor() const
{
    return _lineColor;
}

int Map::getDistanceLineWidth() const
{
    return _lineWidth;
}

int Map::getMapScale() const
{
    return _mapScale;
}

const QRect& Map::getMapRect() const
{
    return _mapRect;
}

void Map::setMapRect(const QRect& mapRect)
{
    if(_mapRect != mapRect)
    {
        _mapRect = mapRect;
        emit dirty();
    }
}

const QRect& Map::getCameraRect() const
{
    return _cameraRect;
}

QUndoStack* Map::getUndoStack() const
{
    return _undoStack;
}

void Map::applyPaintTo(QImage* target, const QColor& clearColor, int index, bool preview, int startIndex)
{
    if(!target)
    {
        if(!_imgFow.isNull())
            internalApplyPaintTo(&_imgFow, clearColor, index, true, startIndex);
        if(!_imgBWFow.isNull())
            internalApplyPaintTo(&_imgBWFow, clearColor, index, true, startIndex);
    }
    else
    {
        internalApplyPaintTo(target, clearColor, index, preview, startIndex);
    }
}

void Map::internalApplyPaintTo(QImage* target, const QColor& clearColor, int index, bool preview, int startIndex)
{
    if((!target) || (index < startIndex))
        return;

    if(index > _undoStack->count())
        index = _undoStack->count();

    if(startIndex == 0)
        target->fill(clearColor);

    for( int i = startIndex; i < index; ++i )
    {
        const UndoBase* action = dynamic_cast<const UndoBase*>(_undoStack->command(i));
        if(action)
            action->apply(preview, target);
    }
}

UndoMarker* Map::getMapMarker(int id)
{
    // Search the undo stack for new markers
    for(int i = 0; i < _undoStack->count(); ++i)
    {
        // This is a little evil, will need to do it better with a full undo/redo implementation...
        UndoMarker* undoItem = const_cast<UndoMarker*>(dynamic_cast<const UndoMarker*>(_undoStack->command(i)));
        if((undoItem) && (undoItem->getMarker().getID() == id))
            return undoItem;
    }

    return nullptr;
}

bool Map::getShowMarkers() const
{
    return _showMarkers;
}

int Map::getMarkerCount() const
{
    return _undoStack->count();
}

void Map::addMapItem(MapDraw* mapItem)
{
    _mapItems.append(mapItem);
}

void Map::removeMapItem(MapDraw* mapItem)
{
    _mapItems.removeOne(mapItem);
}

int Map::getMapItemCount() const
{
    return _mapItems.count();
}

MapDraw* Map::getMapItem(int index)
{
    if((index < 0) || (index >= _mapItems.count()))
        return nullptr;
    else
        return _mapItems.at(index);
}

bool Map::isInitialized()
{
    return _initialized;
}

bool Map::isValid()
{
    // If it has been initialized, it's valid
    if(isInitialized())
        return true;

    // If the filename is empty, it's invalid
    if(_filename.isEmpty())
        return false;

    // If the file does not exist, it's invalid
    if(!QFile::exists(_filename))
        return false;

    // If the file is not a file (ie it's a directory), it's invalid
    QFileInfo fileInfo(_filename);
    if(!fileInfo.isFile())
        return false;

#if !defined(Q_OS_MAC)
    // If the file is otherwise OK and the format is a known image, it's invalid because it should have been loaded!
    QImageReader reader(_filename);
    if(!reader.format().isEmpty())
        return false;
#endif

    return true;
}

LayerScene& Map::getLayerScene()
{
    return _layerScene;
}

const LayerScene& Map::getLayerScene() const
{
    return _layerScene;
}

void Map::setExternalFoWImage(QImage externalImage)
{
    _imgFow = externalImage;
    applyPaintTo(nullptr, QColor(0,0,0,128), _undoStack->index());
}

QImage Map::getUnfilteredBackgroundImage()
{
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    return layer ? layer->getImageUnfiltered() : QImage();
}

QImage Map::getBackgroundImage()
{
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    return layer ? layer->getImage() : QImage();
}

QImage Map::getFoWImage()
{
    return _imgFow;
}

bool Map::isCleared()
{
    if((_undoStack) && (_undoStack->count() > 0))
    {
        const QUndoCommand* latestCommand = _undoStack->command(_undoStack->index());
        if(latestCommand)
        {
            const UndoFill* fillObj = dynamic_cast<const UndoFill*>(latestCommand);
            if((fillObj) && (fillObj->mapEditFill().color().alpha() == 0))
            {
                return true;
            }
        }
    }

    return false;
}

void Map::paintFoWPoint(QPoint point, const MapDraw& mapDraw, QPaintDevice* target, bool preview)
{
    if(!target)
    {
        if(_imgFow.isNull())
            return;

        target = &_imgFow;
    }

    QPainter p(target);
    p.setPen(Qt::NoPen);

    if(mapDraw.brushType() == DMHelper::BrushType_Circle)
    {
        if(mapDraw.erase())
        {
            if(mapDraw.smooth())
            {
                QRadialGradient grad(point, mapDraw.radius());
                grad.setColorAt(0, QColor(0,0,0,0));
                grad.setColorAt(1.0 - (5.0/static_cast<qreal>(mapDraw.radius())), QColor(0,0,0,0));
                grad.setColorAt(1, QColor(255,255,255));
                p.setBrush(grad);
            }
            else
            {
                p.setBrush(QColor(0,0,0,0));
            }
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        }
        else
        {
            int alpha = preview ? 128 : 255;
            p.setBrush(QColor(0,0,0,alpha));
            p.setCompositionMode(QPainter::CompositionMode_Source);
        }

        p.drawEllipse( point, mapDraw.radius(), mapDraw.radius() );
    }
    else
    {
        if(mapDraw.erase())
        {
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            if(mapDraw.smooth())
            {
                qreal border = static_cast<qreal>(mapDraw.radius()) / 20.0;
                qreal radius = static_cast<qreal>(mapDraw.radius()) - (border * 4);
                p.setBrush(QColor(0,0,0,0));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(0,0,0,50));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(0,0,0,100));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(0,0,0,150));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(0,0,0,200));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
            }
            else
            {
                p.setBrush(QColor(0,0,0,0));
                p.drawRect(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), mapDraw.radius() * 2, mapDraw.radius() * 2);
            }
        }
        else
        {
            int alpha = preview ? 128 : 255;
            p.setBrush(QColor(0,0,0,alpha));
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.drawRect(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), mapDraw.radius() * 2, mapDraw.radius() * 2);
        }
    }
}

void Map::paintFoWRect(QRect rect, const MapEditShape& mapEditShape, QPaintDevice* target, bool preview)
{
    if(!target)
    {
        if(_imgFow.isNull())
            return;

        target = &_imgFow;
    }

    QPainter p(target);
    p.setPen(Qt::NoPen);

    if(mapEditShape.erase())
    {
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        if(mapEditShape.smooth())
        {
            qreal rectWidth = rect.width() / 80;
            qreal rectHeight = rect.height() / 80;
            QRectF baseRect(static_cast<qreal>(rect.x()) + rectWidth * 4,
                            static_cast<qreal>(rect.y()) + rectHeight * 4,
                            static_cast<qreal>(rect.width()) - rectWidth * 4 * 2,
                            static_cast<qreal>(rect.height()) - rectHeight * 4 * 2 );
            p.setBrush(QColor(0,0,0,0));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(0,0,0,50));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(0,0,0,100));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(0,0,0,150));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(0,0,0,200));
            p.drawRect(baseRect);
        }
        else
        {
            p.setBrush(QColor(0,0,0,0));
            p.drawRect(rect);
        }
    }
    else
    {
        int alpha = preview ? 128 : 255;
        p.setBrush(QColor(0,0,0,alpha));
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawRect(rect);
    }
}

void Map::fillFoW(const QColor& color, QPaintDevice* target)
{
    if(!target)
    {
        if(_imgFow.isNull())
            return;

        target = &_imgFow;
    }

    QPainter p(target);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(0,0,target->width(),target->height(),color);
}

QImage Map::getBWFoWImage()
{
    // TODO: get layer and extract BW FOW image
    return getBWFoWImage(_layerScene.sceneSize().toSize());
}

QImage Map::getBWFoWImage(const QImage &img)
{
    return getBWFoWImage(img.size());
}

QImage Map::getBWFoWImage(const QSize &size)
{
    if((_imgBWFow.isNull()) || (size != _imgBWFow.size()) || (_indexBWFow > _undoStack->index()))
    {
        _imgBWFow = QImage(size, QImage::Format_ARGB32);
        _indexBWFow = 0;
    }

    applyPaintTo(&_imgBWFow, QColor(0,0,0,255), _undoStack->index(), false, _indexBWFow);
    _indexBWFow = qMax(_undoStack->index() - 1, 0);

    return _imgBWFow;
}

/*
QImage Map::getPublishImage()
{
    QImage result(getBackgroundImage());

    QImage bwFoWImage = getBWFoWImage(_imgBackground);
    QPainter p;
    p.begin(&result);
        p.drawImage(0, 0, bwFoWImage);
    p.end();

    return result;
}

QImage Map::getPublishImage(const QRect& rect)
{
    QRect targetRect(rect);
    if(targetRect.left() < 0)
        targetRect.setLeft(0);
    if(targetRect.top() < 0)
        targetRect.setTop(0);
    if(targetRect.right() > _imgBackground.rect().right())
        targetRect.setRight(_imgBackground.rect().right());
    if(targetRect.bottom() > _imgBackground.rect().bottom())
        targetRect.setBottom(_imgBackground.rect().bottom());

    QImage result = QImage(targetRect.size(), _imgBackground.format());
    QImage bwFoWImage = getBWFoWImage(_imgBackground);
    QPainter p;
    p.begin(&result);
        p.drawImage(0, 0, getBackgroundImage(), targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height());
        p.drawImage(0, 0, bwFoWImage, targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height());
    p.end();

    return result;
}
*/

QImage Map::getGrayImage()
{
    QImage result(getPreviewImage());

    QImage grayFoWImage(result.size(), QImage::Format_ARGB32);
    applyPaintTo(&grayFoWImage, QColor(0,0,0,128), _undoStack->index(), true);

    QPainter p;
    p.begin(&result);
        p.drawImage(0, 0, grayFoWImage);
    p.end();

    return result;
}

/*
QImage Map::getShrunkPublishImage(QRect* targetRect)
{
    QImage bwFoWImage = getBWFoWImage(_imgBackground);

    int top, bottom, left, right;
    top = bottom = left = right = -1;
    int i, j;
    for(j = 0; (j < bwFoWImage.height()) && (top == -1); ++j)
    {
        for(i = 0; (i < bwFoWImage.width()) && (top == -1); ++i)
        {
            if(bwFoWImage.pixelColor(i,j) != Qt::black)
            {
                top = j;
            }
        }
    }

    for(j = bwFoWImage.height() - 1; (j > top) && (bottom == -1); --j)
    {
        for(i = 0; (i < bwFoWImage.width()) && (bottom == -1); ++i)
        {
            if(bwFoWImage.pixelColor(i,j) != Qt::black)
            {
                bottom = j;
            }
        }
    }

    for(i = 0; (i < bwFoWImage.width()) && (left == -1); ++i)
    {
        for(j = top; (j < bottom) && (left == -1); ++j)
        {
            if(bwFoWImage.pixelColor(i,j) != Qt::black)
            {
                left = i;
            }
        }
    }

    for(i = bwFoWImage.width() - 1; (i > left) && (right == -1); --i)
    {
        for(j = top; (j < bottom) && (right == -1); ++j)
        {
            if(bwFoWImage.pixelColor(i,j) != Qt::black)
            {
                right = i;
            }
        }
    }

    QImage result;
    if((right == left) || (top == bottom))
    {
        result = bwFoWImage;
    }
    else
    {
        result = QImage(right - left, bottom - top, _imgBackground.format());
        QPainter p;
        p.begin(&result);
            p.drawImage(0, 0, getBackgroundImage(), left, top, right - left, bottom - top);
            p.drawImage(0, 0, bwFoWImage, left, top, right - left, bottom - top);
        p.end();
    }

    if(targetRect)
        *targetRect = QRect(left, top, right - left, bottom - top);

    return result;
}
*/

QRect Map::getShrunkPublishRect()
{
    //QImage bwFoWImage = getBWFoWImage(_imgBackground);
    QImage bwFoWImage = getBWFoWImage();

    int top, bottom, left, right;
    top = bottom = left = right = -1;
    int i, j;
    for(j = 0; (j < bwFoWImage.height()) && (top == -1); ++j)
    {
        for(i = 0; (i < bwFoWImage.width()) && (top == -1); ++i)
        {
            if(bwFoWImage.pixelColor(i,j) != Qt::black)
            {
                top = j;
            }
        }
    }

    for(j = bwFoWImage.height() - 1; (j > top) && (bottom == -1); --j)
    {
        for(i = 0; (i < bwFoWImage.width()) && (bottom == -1); ++i)
        {
            if(bwFoWImage.pixelColor(i,j) != Qt::black)
            {
                bottom = j;
            }
        }
    }

    for(i = 0; (i < bwFoWImage.width()) && (left == -1); ++i)
    {
        for(j = top; (j < bottom) && (left == -1); ++j)
        {
            if(bwFoWImage.pixelColor(i,j) != Qt::black)
            {
                left = i;
            }
        }
    }

    for(i = bwFoWImage.width() - 1; (i > left) && (right == -1); --i)
    {
        for(j = top; (j < bottom) && (right == -1); ++j)
        {
            if(bwFoWImage.pixelColor(i,j) != Qt::black)
            {
                right = i;
            }
        }
    }

    return QRect(left, top, right - left, bottom - top);
}

bool Map::isFilterApplied() const
{
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    return layer ? layer->isFilterApplied() : false;
}

MapColorizeFilter Map::getFilter() const
{
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    return layer ? layer->getFilter() : MapColorizeFilter();
}

QImage Map::getPreviewImage()
{
    QImage previewImage = getBackgroundImage();
    if(!previewImage.isNull())
        return previewImage;

    if((_filename.isNull()) || (_filename.isEmpty()))
        return QImage();

    if(!previewImage.load(_filename))
    {
        // Last attempt, check the cache for a video version
        QString cacheFilePath = DMHCache().getCacheFilePath(_filename, QString("png"));
        previewImage.load(cacheFilePath);
    }

    return isFilterApplied() ? getFilter().apply(previewImage) : previewImage;
}

bool Map::initialize()
{
    if(_initialized)
        return true;

    QImage imgBackground;

    // Everything other than direct layer initialization has to be for backwards compatibility exclusively
    if(!_filename.isEmpty())
    {
        if(!QFile::exists(_filename))
        {
            qDebug() << "[Map] Map file not found: " << _filename << " for entry " << getName();
            QMessageBox::StandardButton result = QMessageBox::critical(nullptr,
                                                                       QString("DMHelper Map File Not Found"),
                                                                       QString("For the map entry """) + getName() + QString(""", the map file could not be found: ") + _filename + QString("\n Do you want to select a new map file?"),
                                                                       QMessageBox::Yes | QMessageBox::No,
                                                                       QMessageBox::Yes);

            QString newFileName;
            if(result == QMessageBox::Yes)
                newFileName = QFileDialog::getOpenFileName(nullptr, QString("DMHelper New Map File"));

            setFileName(newFileName);

            if(newFileName.isEmpty())
                return true;
        }

        QFileInfo fileInfo(_filename);
        if(!fileInfo.isFile())
        {
            qDebug() << "[Map] Map file not a file: " << _filename << " for entry " << getName();
            QMessageBox::StandardButton result = QMessageBox::critical(nullptr,
                                                                       QString("DMHelper Map File Not Valid"),
                                                                       QString("For the map entry """) + getName() + QString(""", the map isn't a file: ") + _filename + QString("\n Do you want to select a new map file?"),
                                                                       QMessageBox::Yes | QMessageBox::No,
                                                                       QMessageBox::Yes);

            QString newFileName;
            if(result == QMessageBox::Yes)
                newFileName = QFileDialog::getOpenFileName(nullptr, QString("DMHelper New Map File"));

            setFileName(newFileName);

            if(newFileName.isEmpty())
                return true;
        }

        QImageReader reader(_filename);
        if(reader.format().isEmpty())
        {
            // The image is not a known format, so it could be a video
            qDebug() << "[Map] Image format is not known, so it cannot be read";
            return false;
        }

        imgBackground = reader.read();

        if(imgBackground.isNull())
        {
            // Could not read the file as an image - this is likely a file error...
            qDebug() << "[Map] Not able to read map file: " << reader.error() <<", " << reader.errorString();
            qDebug() << "[Map] Image Format: " << QString::fromUtf8(reader.format());
#if !defined(Q_OS_MAC)
            QMessageBox::critical(nullptr,
                                  QString("DMHelper Map File Read Error"),
                                  QString("For the map entry """) + getName() + QString(""", the map could not be read. It may be too high resolution for DMHelper!"));
#endif
            return false;
        }

        if(imgBackground.format() != QImage::Format_ARGB32_Premultiplied)
            imgBackground.convertTo(QImage::Format_ARGB32_Premultiplied);
    }
    else if(_mapColor.isValid() && _mapSize.isValid())
    {
        imgBackground = QImage(_mapSize, QImage::Format_ARGB32_Premultiplied);
        imgBackground.fill(_mapColor);
    }
    else
    {
        qDebug() << "[Map] ERROR: Unable to initialize map with neither a file nor a color & size";
        return true;
    }

    //emitSignal(_imgBackground);
    LayerImage* backgroundLayer = new LayerImage(imgBackground, -2);
    backgroundLayer->setApplyFilter(_filterApplied);
    backgroundLayer->setFilter(_filter);
    connect(this, &Map::mapImageChanged, backgroundLayer, &LayerImage::updateImage);
    _layerScene.appendLayer(backgroundLayer);

    QImage cloudsImage("C:/Users/turne/Documents/DnD/DM Helper/testdata/CloudsSquare.png");
    LayerImage* cloudsLayer = new LayerImage(cloudsImage, 20);
    _layerScene.appendLayer(cloudsLayer);

    //_imgFow = QImage(_imgBackground.size(), QImage::Format_ARGB32);
    //applyPaintTo(nullptr, QColor(0,0,0,128), _undoStack->index());

    LayerFow* fowLayer = new LayerFow(-1);
    _layerScene.appendLayer(fowLayer);

    if(!_cameraRect.isValid())
    {
        _cameraRect.setTopLeft(QPoint(0, 0));
        _cameraRect.setSize(imgBackground.size());
    }

    _initialized = true;
    return true;
}

void Map::uninitialize()
{
//    _imgBackground = QImage();
    _imgBWFow = QImage();
    _imgFow = QImage();
    _initialized = false;
}

void Map::undoPaint()
{
    emit executeUndo();
}

void Map::updateFoW()
{
    emit requestFoWUpdate();
}

void Map::addMapMarker(UndoMarker* undoEntry, MapMarker* marker)
{
    emit requestMapMarker(undoEntry, marker);
}

void Map::setParty(Party* party)
{
    QUuid newPartyId = (party == nullptr) ? QUuid() : party->getID();
    if(_partyId != newPartyId)
    {
        _partyAltIcon = QString();
        _partyId = newPartyId;
        emit partyChanged(party);
        emit dirty();
    }
}

void Map::setPartyIcon(const QString& partyIcon)
{
    if(_partyAltIcon != partyIcon)
    {
        _partyId = QUuid();
        _partyAltIcon = partyIcon;
        emit partyIconChanged(_partyAltIcon);
        emit dirty();
    }
}

void Map::setShowParty(bool showParty)
{
    if(_showPartyIcon != showParty)
    {
        _showPartyIcon = showParty;
        emit showPartyChanged(showParty);
        emit dirty();
    }
}

void Map::setPartyIconPos(const QPoint& pos)
{
    if(_partyIconPos != pos)
    {
        _partyIconPos = pos;
        emit partyIconPosChanged(pos);
        emit dirty();
    }
}

void Map::setPartyScale(int partyScale)
{
    if(_partyScale != partyScale)
    {
        _partyScale = partyScale;
        emit partyScaleChanged(_partyScale);
        emit dirty();
    }
}

void Map::setMapScale(int mapScale)
{
    if(_mapScale != mapScale)
    {
        _mapScale = mapScale;
        emit mapScaleChanged(_mapScale);
        emit dirty();
    }
}

void Map::setDistanceLineColor(const QColor& color)
{
    if(_lineColor != color)
    {
        _lineColor = color;
        emit distanceLineColorChanged(_lineColor);
        emit dirty();
    }
}

void Map::setDistanceLineType(int lineType)
{
    if(_lineType != lineType)
    {
        _lineType = lineType;
        emit distanceLineTypeChanged(_lineType);
        emit dirty();
    }
}

void Map::setDistanceLineWidth(int lineWidth)
{
    if(_lineWidth != lineWidth)
    {
        _lineWidth = lineWidth;
        emit distanceLineWidthChanged(_lineWidth);
        emit dirty();
    }
}

void Map::setShowMarkers(bool showMarkers)
{
    if(_showMarkers != showMarkers)
    {
        _showMarkers = showMarkers;
        emit showMarkersChanged(_showMarkers);
        emit dirty();
    }
}

void Map::setApplyFilter(bool applyFilter)
{
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    if(layer)
        layer->setApplyFilter(applyFilter);

/*
    if(_filterApplied != applyFilter)
    {
        _filterApplied = applyFilter;
        emit dirty();
        emit mapImageChanged(getBackgroundImage());
    }
    */
}

void Map::setFilter(const MapColorizeFilter& filter)
{
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    if(layer)
        layer->setFilter(filter);

    /*
    _filter = filter;
    emit dirty();
    emit mapImageChanged(getBackgroundImage());
    */
}

void Map::setCameraRect(const QRect& cameraRect)
{
    if(_cameraRect != cameraRect)
    {
        _cameraRect = cameraRect;
        emit dirty();
    }
}

void Map::setCameraRect(const QRectF& cameraRect)
{
    setCameraRect(cameraRect.toRect());
}

QDomElement Map::createOutputXML(QDomDocument &doc)
{
   return doc.createElement("map");
}

void Map::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("filename", targetDirectory.relativeFilePath(getFileName()));
    element.setAttribute("lineColor", _lineColor.name());
    element.setAttribute("lineType", _lineType);
    element.setAttribute("lineWidth", _lineWidth);
    element.setAttribute("mapColor", _mapColor.name());
    element.setAttribute("mapSizeWidth", _mapSize.width());
    element.setAttribute("mapSizeHeight", _mapSize.height());
    element.setAttribute("audiotrack", _audioTrackId.toString());
    element.setAttribute("playaudio", _playAudio);
    element.setAttribute("showparty", _showPartyIcon);
    element.setAttribute("party", _partyId.toString());
    element.setAttribute("partyalticon", _partyAltIcon);
    element.setAttribute("partyPosX", _partyIconPos.x());
    element.setAttribute("partyPosY", _partyIconPos.y());
    element.setAttribute("partyScale", _partyScale);
    element.setAttribute("mapScale", _mapScale);
    element.setAttribute("showMarkers", _showMarkers);
    element.setAttribute("mapRectX", _mapRect.x());
    element.setAttribute("mapRectY", _mapRect.y());
    element.setAttribute("mapRectWidth", _mapRect.width());
    element.setAttribute("mapRectHeight", _mapRect.height());
    element.setAttribute("cameraRectX", _cameraRect.x());
    element.setAttribute("cameraRectY", _cameraRect.y());
    element.setAttribute("cameraRectWidth", _cameraRect.width());
    element.setAttribute("cameraRectHeight", _cameraRect.height());

    // Check if we can skip some paint commands because they have been covered up by a fill
    challengeUndoStack();

    QDomElement actionsElement = doc.createElement("actions");
    for(int i = 0; i < _undoStack->index(); ++i )
    {
        const UndoBase* action = dynamic_cast<const UndoBase*>(_undoStack->command(i));
        if((action) && (!action->isRemoved()))
        {
            QDomElement actionElement = doc.createElement("action");
            actionElement.setAttribute("type", action->getType());
            action->outputXML(doc, actionElement, targetDirectory, isExport);
            actionsElement.appendChild(actionElement);
        }
    }
    element.appendChild(actionsElement);

    if(_filterApplied)
    {
        QDomElement filterElement = doc.createElement("filter");
        filterElement.setAttribute("r2r", _filter._r2r);
        filterElement.setAttribute("g2r", _filter._g2r);
        filterElement.setAttribute("b2r", _filter._b2r);
        filterElement.setAttribute("r2g", _filter._r2g);
        filterElement.setAttribute("g2g", _filter._g2g);
        filterElement.setAttribute("b2g", _filter._b2g);
        filterElement.setAttribute("r2b", _filter._r2b);
        filterElement.setAttribute("g2b", _filter._g2b);
        filterElement.setAttribute("b2b", _filter._b2b);
        filterElement.setAttribute("sr", _filter._sr);
        filterElement.setAttribute("sg", _filter._sg);
        filterElement.setAttribute("sb", _filter._sb);
        filterElement.setAttribute("isOverlay", _filter._isOverlay);
        filterElement.setAttribute("overlayColor", _filter._overlayColor.name());
        filterElement.setAttribute("overlayAlpha", _filter._overlayAlpha);
        element.appendChild(filterElement);
    }

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool Map::belongsToObject(QDomElement& element)
{
    if((element.tagName() == QString("actions")) || (element.tagName() == QString("filter")))
        return true;
    else
        return CampaignObjectBase::belongsToObject(element);
}

void Map::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    _audioTrackId = parseIdString(element.attribute("audiotrack"));
    _playAudio = static_cast<bool>(element.attribute("playaudio", QString::number(1)).toInt());
    _showPartyIcon = static_cast<bool>(element.attribute("showparty", QString::number(1)).toInt());
    _partyId = parseIdString(element.attribute("party"));
    _partyAltIcon = element.attribute("partyalticon");
    _partyIconPos = QPoint(element.attribute("partyPosX", QString::number(-1)).toInt(),
                           element.attribute("partyPosY", QString::number(-1)).toInt());
    _partyScale = element.attribute("partyScale", QString::number(10)).toInt();
    _mapScale = element.attribute("mapScale", QString::number(100)).toInt();
    _showMarkers = static_cast<bool>(element.attribute("showMarkers", QString::number(1)).toInt());

    CampaignObjectBase::internalPostProcessXML(element, isImport);
}

void Map::challengeUndoStack()
{
    bool filled = false;
    for(int i = _undoStack->index(); i >= 0; --i)
    {
        const UndoBase* constAction = dynamic_cast<const UndoBase*>(_undoStack->command(i));
        if(constAction)
        {
            if(filled)
            {
                if((constAction->getType() == DMHelper::ActionType_Fill) ||
                   (constAction->getType() == DMHelper::ActionType_Path) ||
                   (constAction->getType() == DMHelper::ActionType_Point) ||
                   (constAction->getType() == DMHelper::ActionType_Rect))
                {
                    UndoBase* action = const_cast<UndoBase*>(constAction);
                    action->setRemoved(true);
                }
            }
            else if(constAction->getType() == DMHelper::ActionType_Fill)
            {
                filled = true;
            }
        }
    }
}
