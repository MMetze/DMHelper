#include "map.h"
#include "undofowbase.h"
#include "undofowfill.h"
#include "undofowpath.h"
#include "undofowpoint.h"
#include "undofowshape.h"
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

Map::Map(const QString& mapName, QObject *parent) :
    CampaignObjectBase(mapName, parent),
    _filename(), // for compatibility only
    //_undoStack(nullptr),
    _audioTrackId(),
    _playAudio(false),
    _mapRect(),
    _cameraRect(),
    _showPartyIcon(false),
    _partyId(),
    _partyIconPos(-1, -1),
    //_partyScale(DMHelper::STARTING_GRID_SCALE),
    _mapScale(100),
    _showMarkers(true),
    _mapItems(),
    _initialized(false),
    _layerScene(this),
    //_imgBackground(),
    //_imgFow(),
    _undoItems(),
    //_imgBWFow(),
    //_indexBWFow(0),
    //_filterApplied(false),
    //_filter(),
    _lineType(Qt::SolidLine),
    _lineColor(Qt::yellow),
    _lineWidth(1),
    _mapColor(Qt::white),
    _mapSize()
{
//    _undoStack = new QUndoStack(this);
}

Map::~Map()
{
    qDeleteAll(_mapItems);
    qDeleteAll(_undoItems);
    _layerScene.clearLayers();
}

void Map::inputXML(const QDomElement &element, bool isImport)
{
    // TODO: include layers in import/export (with backwards compatibility)
    // TODO: Layers - need to find a way to handle changing the image and pointing it at the right layer

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
    _playAudio = static_cast<bool>(element.attribute("playaudio", QString::number(1)).toInt());
    _showPartyIcon = static_cast<bool>(element.attribute("showparty", QString::number(1)).toInt());
    _partyAltIcon = element.attribute("partyalticon");
    _partyIconPos = QPoint(element.attribute("partyPosX", QString::number(-1)).toInt(),
                           element.attribute("partyPosY", QString::number(-1)).toInt());
    _mapScale = element.attribute("mapScale", QString::number(100)).toInt();
    _showMarkers = static_cast<bool>(element.attribute("showMarkers", QString::number(1)).toInt());

    QDomElement layersElement = element.firstChildElement(QString("layer-scene"));
    if(!layersElement.isNull())
    {
        _layerScene.inputXML(layersElement, isImport);
    }
    else
    {
        int partyScale = element.attribute("partyScale", QString::number(DMHelper::STARTING_GRID_SCALE)).toInt();
        _layerScene.setScale(partyScale);
        LayerImage* imageLayer = new LayerImage(QString("Map"), _filename);
        imageLayer->inputXML(element, isImport);
        /*
        // For backwards compatibility only, should be part of a layer
        _filename = element.attribute("filename"); // Even if it can't be found, don't want to lose the data
        if(_filename == QString(".")) // In case the map file is this trivial, it can be ignored
            _filename.clear();

        // For backwards compatibility only, should be part of a layer
        QDomElement filterElement = element.firstChildElement(QString("filter"));
        if(!filterElement.isNull())
        {
            imageLayer->setApplyFilter(true);
            //_filterApplied = true;

            MapColorizeFilter filter;
            filter._r2r = filterElement.attribute("r2r",QString::number(1.0)).toDouble();
            filter._g2r = filterElement.attribute("g2r",QString::number(0.0)).toDouble();
            filter._b2r = filterElement.attribute("b2r",QString::number(0.0)).toDouble();
            filter._r2g = filterElement.attribute("r2g",QString::number(0.0)).toDouble();
            filter._g2g = filterElement.attribute("g2g",QString::number(1.0)).toDouble();
            filter._b2g = filterElement.attribute("b2g",QString::number(0.0)).toDouble();
            filter._r2b = filterElement.attribute("r2b",QString::number(0.0)).toDouble();
            filter._g2b = filterElement.attribute("g2b",QString::number(0.0)).toDouble();
            filter._b2b = filterElement.attribute("b2b",QString::number(1.0)).toDouble();
            filter._sr = filterElement.attribute("sr",QString::number(1.0)).toDouble();
            filter._sg = filterElement.attribute("sg",QString::number(1.0)).toDouble();
            filter._sb = filterElement.attribute("sb",QString::number(1.0)).toDouble();

            filter._isOverlay = static_cast<bool>(filterElement.attribute("isOverlay",QString::number(1)).toInt());
            filter._overlayColor.setNamedColor(filterElement.attribute("overlayColor",QString("#000000")));
            filter._overlayAlpha = filterElement.attribute("overlayAlpha",QString::number(128)).toInt();

            imageLayer->setFilter(filter);
        }
        */

        _layerScene.appendLayer(imageLayer);

        LayerFow* fowLayer = new LayerFow(QString("FoW"));
        fowLayer->inputXML(element, isImport);
        _layerScene.appendLayer(fowLayer);
        /*
        QDomElement actionsElement = element.firstChildElement(QString("actions"));
        if(!actionsElement.isNull())
        {
            QDomElement actionElement = actionsElement.firstChildElement(QString("action"));
            while(!actionElement.isNull())
            {
                UndoFowBase* newAction = nullptr;
                switch( actionElement.attribute(QString("type")).toInt())
                {
                    case DMHelper::ActionType_Fill:
                        newAction = new UndoFowFill(nullptr, MapEditFill(QColor()));
                        break;
                    case DMHelper::ActionType_Path:
                        newAction = new UndoFowPath(nullptr, MapDrawPath());
                        break;
                    case DMHelper::ActionType_Point:
                        newAction = new UndoFowPoint(nullptr, MapDrawPoint(0, DMHelper::BrushType_Circle, true, true, QPoint()));
                        break;
                    case DMHelper::ActionType_Rect:
                        newAction = new UndoFowShape(nullptr, MapEditShape(QRect(), true, true));
                        break;
                        // TODO: Layers - where should the markers go?
                    case DMHelper::ActionType_SetMarker:
                        newAction = new UndoMarker(nullptr, MapMarker());
                        break;
                    case DMHelper::ActionType_Base:
                    default:
                        break;
                }

                if(newAction)
                {
                    newAction->inputXML(actionElement, isImport);
                    _undoItems.append(newAction);
                }

                actionElement = actionElement.nextSiblingElement(QString("action"));
            }
        }
        */

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
    //_partyScale = otherMap->getPartyScale();
    _mapScale = otherMap->getMapScale();

    // TODO: Layers - need a markers layer
    _showMarkers = otherMap->getShowMarkers();

    setDistanceLineType(otherMap->getDistanceLineType());
    setDistanceLineColor(otherMap->getDistanceLineColor());
    setDistanceLineWidth(otherMap->getDistanceLineWidth());

    setMapColor(otherMap->getMapColor());
    setMapSize(otherMap->getMapSize());

    _layerScene.copyValues(&otherMap->_layerScene);

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
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getPriority(DMHelper::LayerType_Image));
    return layer ? layer->getImageFile() : QString();
}

bool Map::setFileName(const QString& newFileName)
{
    if(newFileName.isEmpty())
        return true;

    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getPriority(DMHelper::LayerType_Image));
    if((!layer) || (layer->getImageFile() == newFileName))
        return true;

    if(!QFile::exists(newFileName))
    {
        QMessageBox::critical(nullptr,
                              QString("DMHelper Map File Not Found"),
                              QString("The new map file could not be found: ") + newFileName + QString(", keeping map file: ") + layer->getImageFile() + QString(" for entry: ") + getName());
        qDebug() << "[Map] setFileName - New map file not found: " << newFileName << " for entry " << getName();
        return false;
    }

    QFileInfo fileInfo(newFileName);
    if(!fileInfo.isFile())
    {
        QMessageBox::critical(nullptr,
                              QString("DMHelper Map File Not Valid"),
                              QString("The new map isn't a file: ") + newFileName + QString(", keeping map file: ") + layer->getImageFile() + QString(" for entry: ") + getName());
        qDebug() << "[Map] setFileName - Map file not a file: " << newFileName << " for entry " << getName();
        return false;
    }

    if(_initialized)
    {
        qDebug() << "[Map] Cannot set new map file, map is initialized and in use! Old: " << layer->getImageFile() << ", New: " << newFileName;
        return true;
    }

    layer->setFileName(newFileName);
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
    //return _partyScale;
    return _layerScene.getScale();
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

UndoMarker* Map::getMapMarker(int id)
{
    // TODO: Layers
    /*
    // Search the undo stack for new markers
    for(int i = 0; i < _undoStack->count(); ++i)
    {
        // This is a little evil, will need to do it better with a full undo/redo implementation...
        UndoMarker* undoItem = const_cast<UndoMarker*>(dynamic_cast<const UndoMarker*>(_undoStack->command(i)));
        if((undoItem) && (undoItem->getMarker().getID() == id))
            return undoItem;
    }
    */

    return nullptr;
}

bool Map::getShowMarkers() const
{
    return _showMarkers;
}

int Map::getMarkerCount() const
{
    // TODO: Layers
    //return _undoStack->count();
    return 0;
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

// TODO: Layers - does this need to be flushed out
bool Map::isValid()
{
    // If it has been initialized, it's valid
    if(isInitialized())
        return true;

    /*
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
*/

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
    /*
    _imgFow = externalImage;
    applyPaintTo(nullptr, QColor(0,0,0,128), _undoStack->index());
    */
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
    LayerFow* layer = dynamic_cast<LayerFow*>(_layerScene.getFirst(DMHelper::LayerType_Fow));
    return layer ? layer->getImage() : QImage();
    //return _imgFow;
}

bool Map::isCleared()
{
    // TODO: Layers
    /*
    if((_undoStack) && (_undoStack->count() > 0))
    {
        const QUndoCommand* latestCommand = _undoStack->command(_undoStack->index());
        if(latestCommand)
        {
            const UndoFowFill* fillObj = dynamic_cast<const UndoFowFill*>(latestCommand);
            if((fillObj) && (fillObj->mapEditFill().color().alpha() == 0))
            {
                return true;
            }
        }
    }
    */

    return false;
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

    // TODO: Layers
    /*
    QImage grayFoWImage(result.size(), QImage::Format_ARGB32);
    applyPaintTo(&grayFoWImage, QColor(0,0,0,128), _undoStack->index(), true);

    QPainter p;
    p.begin(&result);
        p.drawImage(0, 0, grayFoWImage);
    p.end();
    */

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
    // TODO: Layers
    //QImage bwFoWImage = getBWFoWImage(_imgBackground);
    QImage bwFoWImage;// = getBWFoWImage();

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

    // TODO: build a preview image
    /*
    if((_filename.isNull()) || (_filename.isEmpty()))
        return QImage();

    if(!previewImage.load(_filename))
    {
        // Last attempt, check the cache for a video version
        QString cacheFilePath = DMHCache().getCacheFilePath(_filename, QString("png"));
        previewImage.load(cacheFilePath);
    }
    */

    return isFilterApplied() ? getFilter().apply(previewImage) : previewImage;
}

bool Map::initialize()
{
    if(_initialized)
        return true;

    //QImage imgBackground;

    /*
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
    */

    //emitSignal(_imgBackground);

    _layerScene.initializeLayers();
/*
    Todo: create a basic color layer, move image loading into the layer

    LayerImage* backgroundLayer = new LayerImage(QString("Background"), imgBackground, -2);
    // TODO: These are needed in the layers...
    backgroundLayer->setApplyFilter(_filterApplied);
    backgroundLayer->setFilter(_filter);
    */
//    connect(this, &Map::mapImageChanged, backgroundLayer, &LayerImage::updateImage);
    //_layerScene.appendLayer(backgroundLayer);

    /*
    QImage cloudsImage("C:/Users/turne/Documents/DnD/DM Helper/testdata/CloudsSquare.png");
    LayerImage* cloudsLayer = new LayerImage(cloudsImage, 20);
    _layerScene.appendLayer(cloudsLayer);
    */

    //_imgFow = QImage(_imgBackground.size(), QImage::Format_ARGB32);
    //applyPaintTo(nullptr, QColor(0,0,0,128), _undoStack->index());

    //LayerFow* fowLayer = new LayerFow(QString("FoW"), imgBackground.size(), -1);
    /*
    for(int i = 0; i < _undoItems.count(); ++i)
    {
        UndoFowBase* undoItem = _undoItems[i];
        undoItem->setLayer(fowLayer);
        fowLayer->getUndoStack()->push(undoItem);
    }
    */
    //_layerScene.appendLayer(fowLayer);

    if(!_cameraRect.isValid())
    {
        _cameraRect.setTopLeft(QPoint(0, 0));
        _cameraRect.setSize(_layerScene.sceneSize().toSize());
    }

    _initialized = true;
    return true;
}

void Map::uninitialize()
{
//    _imgBackground = QImage();
//    _imgBWFow = QImage();
//    _imgFow = QImage();
    _layerScene.clearLayers();
    _initialized = false;
}

void Map::undoPaint()
{
    //emit executeUndo();
}

void Map::updateFoW()
{
    //emit requestFoWUpdate();
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
    if(_layerScene.getScale() != partyScale)
    //if(_partyScale != partyScale)
    {
        //_partyScale = partyScale;
        _layerScene.setScale(partyScale);
        emit partyScaleChanged(partyScale);
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
    //element.setAttribute("filename", targetDirectory.relativeFilePath(getFileName()));
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
    //element.setAttribute("partyScale", _partyScale);
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

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool Map::belongsToObject(QDomElement& element)
{
    if((element.tagName() == QString("actions")) || (element.tagName() == QString("layer-scene")))
        return true;
    else
        return CampaignObjectBase::belongsToObject(element);
}

void Map::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    _audioTrackId = parseIdString(element.attribute("audiotrack"));
    _partyId = parseIdString(element.attribute("party"));

    _layerScene.postProcessXML(element, isImport);

    CampaignObjectBase::internalPostProcessXML(element, isImport);
}
