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
#include "layerimage.h"
#include "layervideo.h"
#include "layerfow.h"
#include "undomarker.h"
#include "mapmarkergraphicsitem.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDir>
#include <QPainter>
#include <QImageReader>
#include <QMessageBox>
#include <QFileDialog>
#include <QIcon>
#include <QDebug>

Map::Map(const QString& mapName, QObject *parent) :
    CampaignObjectBase(mapName, parent),
    _audioTrackId(),
    _playAudio(false),
    _mapRect(),
    _cameraRect(),
    _showPartyIcon(false),
    _partyId(),
    _partyIconPos(-1, -1),
    _mapScale(0),
    _gridCount(0),
    _showMarkers(true),
    _mapItems(),
    _initialized(false),
    _layerScene(this),
    _undoItems(),
    _lineType(Qt::SolidLine),
    _lineColor(Qt::yellow),
    _lineWidth(1),
    _backgroundColor(Qt::black),
    _mapColor(Qt::white),
    _mapSize(),
    _markerList()
{
    connect(&_layerScene, &LayerScene::dirty, this, &Map::dirty);
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
    _mapScale = element.attribute("mapScale").toInt();
    _showMarkers = static_cast<bool>(element.attribute("showMarkers", QString::number(1)).toInt());

    setBackgroundColor(QColor(element.attribute("backgroundColor", "#000000")));

    // Load the markers
    QDomElement markersElement = element.firstChildElement(QString("markers"));
    if(!markersElement.isNull())
    {
        QDomElement markerElement = markersElement.firstChildElement(QString("marker"));
        while(!markerElement.isNull())
        {
            UndoMarker* newMarker = new UndoMarker(MapMarker());
            newMarker->inputXML(markerElement, isImport);
            addMarker(newMarker);

            markerElement = markerElement.nextSiblingElement(QString("marker"));
        }
    }

    QDomElement layersElement = element.firstChildElement(QString("layer-scene"));
    if(!layersElement.isNull())
    {
        _layerScene.inputXML(layersElement, isImport);
    }
    else
    {
        // Backwards compatibility mode
        int partyScale = element.attribute("partyScale", QString::number(DMHelper::STARTING_GRID_SCALE)).toInt();
        _layerScene.setScale(partyScale);

        Layer* imageLayer = nullptr;
        QString filename = element.attribute("filename");
        if((filename.isEmpty()) || (filename == QString(".")))
        {
            qDebug() << "[Map] inputXML - empty map file: " << filename << ", creating default image layer.";
            imageLayer = new LayerImage(QString("Map Image"), QString());
        }
        else if(!QFileInfo::exists(filename))
        {
            qDebug() << "[Map] inputXML - map file not found: " << filename << ", creating default image layer.";
            imageLayer = new LayerImage(QString("Map Image"), QString());
        }
        else
        {
            QImageReader reader(filename);
            if(reader.canRead())
            {
                qDebug() << "[Map] inputXML - QImageReader can read the file: " << filename << ", creating an image layer.";
                imageLayer = new LayerImage(QString("Map Image: ") + QFileInfo(filename).fileName(), filename);
            }
            else
            {
                qDebug() << "[Map] inputXML - QImageReader *cannot* read the file: " << filename << ", creating a video layer.";
                imageLayer = new LayerVideo(QString("Map Video: ") + QFileInfo(filename).fileName(), filename);
            }
        }

        if(imageLayer)
        {
            imageLayer->inputXML(element, isImport);
            _layerScene.appendLayer(imageLayer);

            LayerFow* fowLayer = new LayerFow(QString("FoW"));
            fowLayer->inputXML(element, isImport);
            _layerScene.appendLayer(fowLayer);

            // Step through the FoW stack and pick out any markers for the Map's marker stack
            QDomElement actionsElement = element.firstChildElement(QString("actions"));
            if(!actionsElement.isNull())
            {
                QDomElement actionElement = actionsElement.firstChildElement(QString("action"));
                while(!actionElement.isNull())
                {
                    if(actionElement.attribute(QString("type")).toInt() == DMHelper::ActionType_SetMarker)
                    {
                        UndoMarker* newMarker = new UndoMarker(MapMarker());
                        newMarker->inputXML(actionElement, isImport);
                        addMarker(newMarker);
                    }

                    actionElement = actionElement.nextSiblingElement(QString("action"));
                }
            }
        }
    }

    CampaignObjectBase::inputXML(element, isImport);
}

void Map::copyValues(const CampaignObjectBase* other)
{
    const Map* otherMap = dynamic_cast<const Map*>(other);
    if(!otherMap)
        return;

    _audioTrackId = otherMap->getAudioTrackId();
    _playAudio = otherMap->getPlayAudio();
    _mapRect = otherMap->getMapRect();
    _cameraRect = otherMap->getCameraRect();
    _showPartyIcon = otherMap->getShowParty();
    _partyId = otherMap->getPartyId();
    _partyAltIcon = otherMap->getPartyAltIcon();
    _partyIconPos = otherMap->getPartyIconPos();
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

QIcon Map::getDefaultIcon()
{
    return QIcon(":/img/data/icon_contentmap.png");
}

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

int Map::getGridCount() const
{
    return _gridCount;
}

void Map::setGridCount(int gridCount)
{
    _gridCount = gridCount;
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

QColor Map::getBackgroundColor() const
{
    return _backgroundColor;
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

void Map::initializeMarkers(QGraphicsScene* scene)
{
    if(!scene)
        return;

    foreach(UndoMarker* marker, _markerList)
    {
        if(marker)
            marker->createMarkerItem(scene, 0.04 * static_cast<qreal>(getPartyScale()));
    }
}

void Map::cleanupMarkers()
{
    foreach(UndoMarker* marker, _markerList)
    {
        if(marker)
            marker->cleanupMarkerItem();
    }
}

bool Map::getShowMarkers() const
{
    return _showMarkers;
}

QList<UndoMarker*> Map::getMarkers()
{
    return _markerList;
}

int Map::getMarkerCount() const
{
    return _markerList.count();
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

/*
void Map::setExternalFoWImage(QImage externalImage)
{
    // TODO: Layers
}
*/

QImage Map::getUnfilteredBackgroundImage()
{
    LayerImage* imageLayer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    LayerVideo* videoLayer = dynamic_cast<LayerVideo*>(_layerScene.getFirst(DMHelper::LayerType_Video));

    if(!videoLayer)
        return imageLayer ? imageLayer->getImageUnfiltered() : QImage();

    if(!imageLayer)
        return videoLayer ? videoLayer->getScreenshot() : QImage();

    return imageLayer->getOrder() <= videoLayer->getOrder() ? imageLayer->getImageUnfiltered() : videoLayer->getScreenshot();
}

QImage Map::getBackgroundImage()
{
    LayerImage* imageLayer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    LayerVideo* videoLayer = dynamic_cast<LayerVideo*>(_layerScene.getFirst(DMHelper::LayerType_Video));

    if(!videoLayer)
        return imageLayer ? imageLayer->getImage() : QImage();

    if(!imageLayer)
        return videoLayer ? videoLayer->getScreenshot() : QImage();

    return imageLayer->getOrder() <= videoLayer->getOrder() ? imageLayer->getImage() : videoLayer->getScreenshot();
}

QImage Map::getFoWImage()
{
    LayerFow* layer = dynamic_cast<LayerFow*>(_layerScene.getFirst(DMHelper::LayerType_Fow));
    return layer ? layer->getImage() : QImage();
}

/*
bool Map::isCleared()
{
    return false;
}
*/

/*
QImage Map::getGrayImage()
{
    return getPreviewImage();
}
*/

/*
bool Map::isFilterApplied() const
{
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    return layer ? layer->isFilterApplied() : false;
}
*/

MapColorizeFilter Map::getFilter() const
{
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    return layer ? layer->getFilter() : MapColorizeFilter();
}

QImage Map::getPreviewImage()
{
    return getBackgroundImage();
}

void Map::addMarker(UndoMarker* marker)
{
    if(!marker)
        return;

    if(marker->getMarkerItem())
        marker->getMarkerItem()->setVisible(getShowMarkers());

    connect(marker, &UndoMarker::mapMarkerMoved, this, &Map::mapMarkerMoved);
    connect(marker, &UndoMarker::mapMarkerEdited, this, &Map::mapMarkerEdited);
    connect(marker, &UndoMarker::unselectParty, this, &Map::unselectParty);
    connect(marker, &UndoMarker::mapMarkerActivated, this, &Map::mapMarkerActivated);

    _markerList.append(marker);
}

void Map::removeMarker(UndoMarker* marker)
{
    if(!marker)
        return;

    if(_markerList.contains(marker))
    {
        disconnect(marker, &UndoMarker::mapMarkerMoved, this, &Map::mapMarkerMoved);
        disconnect(marker, &UndoMarker::mapMarkerEdited, this, &Map::mapMarkerEdited);
        disconnect(marker, &UndoMarker::unselectParty, this, &Map::unselectParty);
        disconnect(marker, &UndoMarker::mapMarkerActivated, this, &Map::mapMarkerActivated);

        _markerList.removeAll(marker);
    }
}

bool Map::initialize()
{
    if(_initialized)
        return true;

    if(_layerScene.getScale() <= 0)
        connect(&_layerScene, &LayerScene::sceneSizeChanged, this, &Map::initializePartyScale);

    _layerScene.initializeLayers();

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
    _layerScene.uninitializeLayers();
    _initialized = false;
}

void Map::undoPaint()
{
    //emit executeUndo();
}

/*
void Map::updateFoW()
{
    //emit requestFoWUpdate();
}
*/

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
    {
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

void Map::setBackgroundColor(const QColor& color)
{
    if(_backgroundColor != color)
    {
        _backgroundColor = color;
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
}

void Map::setFilter(const MapColorizeFilter& filter)
{
    LayerImage* layer = dynamic_cast<LayerImage*>(_layerScene.getFirst(DMHelper::LayerType_Image));
    if(layer)
        layer->setFilter(filter);
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

void Map::initializePartyScale()
{
    disconnect(&_layerScene, &LayerScene::sceneSizeChanged, this, &Map::initializePartyScale);

    if(_layerScene.getScale() > 0)
        return;

    int newScale = DMHelper::STARTING_GRID_SCALE;
    if(_gridCount > 0)
    {
        newScale = _layerScene.sceneSize().width() / _gridCount;
        if(newScale < 1)
            newScale = DMHelper::STARTING_GRID_SCALE;
    }

    _layerScene.setScale(newScale);
}

QDomElement Map::createOutputXML(QDomDocument &doc)
{
   return doc.createElement("map");
}

void Map::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
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

    if((_backgroundColor.isValid()) && (_backgroundColor != Qt::black))
        element.setAttribute("backgroundColor", _backgroundColor.name());

    if(_markerList.count() > 0)
    {
        QDomElement markersElement = doc.createElement("markers");
        foreach(UndoMarker* marker, _markerList)
        {
            if(marker)
            {
                QDomElement markerElement = doc.createElement("marker");
                marker->outputXML(doc, markerElement, targetDirectory, isExport);
                markersElement.appendChild(markerElement);
            }
        }
        element.appendChild(markersElement);
    }

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool Map::belongsToObject(QDomElement& element)
{
    if((element.tagName() == QString("actions")) || (element.tagName() == QString("layer-scene")) || (element.tagName() == QString("markers")))
        return true;
    else
        return CampaignObjectBase::belongsToObject(element);
}

void Map::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    _audioTrackId = parseIdString(element.attribute("audiotrack"));
    _partyId = parseIdString(element.attribute("party"));

    QDomElement layersElement = element.firstChildElement(QString("layer-scene"));
    if(!layersElement.isNull())
        _layerScene.postProcessXML(element, isImport);

    CampaignObjectBase::internalPostProcessXML(element, isImport);
}
