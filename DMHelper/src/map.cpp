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
#include <QDomDocument>
#include <QDomElement>
#include <QUndoStack>
#include <QDir>
#include <QPainter>
#include <QImageReader>
#include <QMessageBox>
#include <QDebug>

Map::Map(const QString& mapName, const QString& fileName, QObject *parent) :
    CampaignObjectBase(mapName, parent),
    _filename(fileName),
    _undoStack(nullptr),
    _audioTrackId(),
    _playAudio(false),
    _mapRect(),
    _showPartyIcon(false),
    _partyId(),
    _partyIconPos(-1, -1),
    _partyScale(10),
    _showMarkers(true),
    _initialized(false),
    _imgBackground(),
    _imgFow(),
    _mapColor(Qt::white),
    _mapSize()
{
    _undoStack = new QUndoStack(this);
}

void Map::inputXML(const QDomElement &element, bool isImport)
{
    setFileName(element.attribute("filename"));
    setMapColor(QColor(element.attribute("mapColor")));
    setMapSize(QSize(element.attribute("mapSizeWidth", QString::number(0)).toInt(),
                     element.attribute("mapSizeHeight", QString::number(0)).toInt()));
    _mapRect = QRect(element.attribute("mapRectX", QString::number(0)).toInt(),
                     element.attribute("mapRectY", QString::number(0)).toInt(),
                     element.attribute("mapRectWidth", QString::number(0)).toInt(),
                     element.attribute("mapRectHeight", QString::number(0)).toInt());

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
                    newAction = new UndoFill(*this, MapEditFill(QColor()));
                    break;
                case DMHelper::ActionType_Path:
                    newAction = new UndoPath(*this, MapDrawPath());
                    break;
                case DMHelper::ActionType_Point:
                    newAction = new UndoPoint(*this, MapDrawPoint(0, DMHelper::BrushType_Circle, true, true, QPoint()));
                    break;
                case DMHelper::ActionType_Rect:
                    newAction = new UndoShape(*this, MapEditShape(QRect(), true, true));
                    break;
                case DMHelper::ActionType_SetMarker:
                    newAction = new UndoMarker(*this, MapMarker(QPoint(0,0), QString(""), QString(""), QUuid()));
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

    CampaignObjectBase::inputXML(element, isImport);
}

int Map::getObjectType() const
{
    return DMHelper::CampaignType_Map;
}

QString Map::getFileName() const
{
    return _filename;
}

void Map::setFileName(const QString& newFileName)
{
    if((_filename == newFileName) || (newFileName.isEmpty()))
        return;

    if(!QFile::exists(newFileName))
    {
        QMessageBox::critical(nullptr,
                              QString("DMHelper Map File Not Found"),
                              QString("The selected map file could not be found: ") + newFileName);
        qDebug() << "[Map] New map file not found: " << newFileName;
        return;
    }

    if(_initialized)
    {
        qDebug() << "[Map] Cannot set new map file, map is initialized and in use! Old: " << _filename << ", New: " << newFileName;
        return;
    }

    _filename = newFileName;
    emit dirty();
}

QColor Map::getMapColor() const
{
    return _mapColor;
}

void Map::setMapColor(QColor color)
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

QUuid Map::getAudioTrackId()
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

QString Map::getPartyAltIcon()
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

QUndoStack* Map::getUndoStack() const
{
    return _undoStack;
}

void Map::applyPaintTo(QImage* target, QColor clearColor, int index, bool preview)
{
    bool previewNeed = preview;

    if(!target)
    {
        if(_imgFow.isNull())
            return;

        target = &_imgFow;
        previewNeed = true;
    }

    if(index < 0)
        return;

    if(index > _undoStack->count())
        index = _undoStack->count();

    target->fill(clearColor);

    for( int i = 0; i < index; ++i )
    {
        const UndoBase* action = dynamic_cast<const UndoBase*>(_undoStack->command(i));
        if(action)
        {
            action->apply(previewNeed, target);
        }
    }
}

UndoMarker* Map::getMapMarker(int id)
{
    // Search the undo stack for new markers
    for(int i = 0; i < _undoStack->count(); ++i)
    {
        // This is a little evil, will need to do it better with a full undo/redo implementation...
        UndoMarker* undoItem = const_cast<UndoMarker*>(dynamic_cast<const UndoMarker*>(_undoStack->command(i)));
        if((undoItem) && (undoItem->marker().getID() == id))
            return undoItem;
    }

    return nullptr;
}

bool Map::getShowMarkers() const
{
    return _showMarkers;
}

bool Map::isInitialized()
{
    return _initialized;
}

void Map::setExternalFoWImage(QImage externalImage)
{
    _imgFow = externalImage;
    applyPaintTo(nullptr, QColor(0,0,0,128), _undoStack->index());
}

QImage Map::getBackgroundImage()
{
    return _imgBackground;
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
                grad.setColorAt(0,QColor(0,0,0,0));
                grad.setColorAt(1.0 - (5.0/static_cast<qreal>(mapDraw.radius())),QColor(0,0,0,0));
                grad.setColorAt(1,QColor(255,255,255));
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

void Map::fillFoW( QColor color, QPaintDevice* target )
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
    return getBWFoWImage(_imgBackground.size());
}

QImage Map::getBWFoWImage(const QImage &img)
{
    return getBWFoWImage(img.size());
}

QImage Map::getBWFoWImage(const QSize &size)
{
    QImage result(size, QImage::Format_ARGB32);

    applyPaintTo(&result, QColor(0,0,0,255), _undoStack->index());

    return result;
}

QImage Map::getPublishImage()
{
    QImage result(_imgBackground);

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
        p.drawImage(0, 0, _imgBackground, targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height());
        p.drawImage(0, 0, bwFoWImage, targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height());
    p.end();

    return result;
}

QImage Map::getGrayImage()
{
    QImage result(_imgBackground);

    QImage grayFoWImage(result.size(), QImage::Format_ARGB32);
    applyPaintTo(&grayFoWImage, QColor(0,0,0,128), _undoStack->index(), true);

    QPainter p;
    p.begin(&result);
        p.drawImage(0, 0, grayFoWImage);
    p.end();

    return result;
}

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
            p.drawImage(0, 0, _imgBackground, left, top, right - left, bottom - top);
            p.drawImage(0, 0, bwFoWImage, left, top, right - left, bottom - top);
        p.end();
    }

    if(targetRect)
        *targetRect = QRect(left, top, right - left, bottom - top);

    return result;
}

QImage Map::getPreviewImage()
{
    if(!_imgBackground.isNull())
        return _imgBackground;

    if((_filename.isNull()) || (_filename.isEmpty()))
        return QImage();

    QImage previewImage;
    previewImage.load(_filename);
    return previewImage;
}

void Map::initialize()
{
    if(_initialized)
        return;

    if(!_filename.isEmpty())
    {
        if(!QFile::exists(_filename))
        {
            QMessageBox::critical(nullptr,
                                  QString("DMHelper Map File Not Found"),
                                  QString("The map file could not be found: ") + _filename);
            qDebug() << "[Map] Map file not found: " << _filename;
            return;
        }

        QImageReader reader(_filename);
        _imgBackground = reader.read();

        if(_imgBackground.isNull())
        {
            qDebug() << "[Map] Error reading map file " << _filename;
            qDebug() << "[Map] Error " << reader.error() << ": " << reader.errorString();
            return;
        }

        if(_imgBackground.format() != QImage::Format_ARGB32_Premultiplied)
            _imgBackground.convertTo(QImage::Format_ARGB32_Premultiplied);
    }
    else if(_mapColor.isValid() && _mapSize.isValid())
    {
        _imgBackground = QImage(_mapSize, QImage::Format_ARGB32_Premultiplied);
        _imgBackground.fill(_mapColor);
    }
    else
    {
        return;
    }

    _imgFow = QImage(_imgBackground.size(), QImage::Format_ARGB32);
    applyPaintTo(nullptr, QColor(0,0,0,128), _undoStack->index());

    _initialized = true;
}

void Map::uninitialize()
{
    _imgBackground = QImage();
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
    _partyIconPos = pos;
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

void Map::setShowMarkers(bool showMarkers)
{
    if(_showMarkers != showMarkers)
    {
        _showMarkers = showMarkers;
        emit showMarkersChanged(_showMarkers);
        emit dirty();
    }
}

QDomElement Map::createOutputXML(QDomDocument &doc)
{
   return doc.createElement("map");
}

void Map::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("filename", targetDirectory.relativeFilePath(getFileName()));
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
    element.setAttribute("showMarkers", _showMarkers);
    element.setAttribute("mapRectX", _mapRect.x());
    element.setAttribute("mapRectY", _mapRect.y());
    element.setAttribute("mapRectWidth", _mapRect.width());
    element.setAttribute("mapRectHeight", _mapRect.height());

    QDomElement actionsElement = doc.createElement("actions");
    /*
    int i;
    for(i = 0; i < _markerList.count(); ++i)
    {
        QDomElement actionElement = doc.createElement("action");
        actionElement.setAttribute("type", DMHelper::ActionType_SetMarker);
        _markerList.at(i).outputXML(actionElement, isExport);
        actionsElement.appendChild(actionElement);
    }
    */

    for(int i = 0; i < _undoStack->index(); ++i )
    {
        const UndoBase* action = dynamic_cast<const UndoBase*>(_undoStack->command(i));
        if(action)
        {
            QDomElement actionElement = doc.createElement("action");
            actionElement.setAttribute("type", action->getType());
            action->outputXML(doc, actionElement, targetDirectory, isExport);
            actionsElement.appendChild(actionElement);
        }
    }
    element.appendChild(actionsElement);

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool Map::belongsToObject(QDomElement& element)
{
    if(element.tagName() == QString("actions"))
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
    _showMarkers = static_cast<bool>(element.attribute("showMarkers", QString::number(1)).toInt());

    CampaignObjectBase::internalPostProcessXML(element, isImport);
}
