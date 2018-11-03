#include "map.h"
#include "undobase.h"
#include "undofill.h"
#include "undopath.h"
#include "undopoint.h"
#include "undomarker.h"
#include "mapframe.h"
#include "dmconstants.h"
#include "campaign.h"
#include "audiotrack.h"
#include <QDomDocument>
#include <QDomElement>
#include <QUndoStack>
#include <QDir>
#include <QPainter>

Map::Map(const QString& mapName, const QString& fileName, QObject *parent) :
    AdventureItem(parent),
    _name(mapName),
    _filename(fileName),
    _undoStack(nullptr),
    _mapFrame(nullptr),
    _markerList(),
    _audioTrackId(-1),
    _initialized(false),
    _imgBackground(),
    _imgFow()
{
    _undoStack = new QUndoStack(this);
}

Map::Map(const QDomElement& element, QObject *parent) :
    AdventureItem(parent),
    _name(),
    _filename(),
    _undoStack(nullptr),
    _mapFrame(nullptr),
    _markerList(),
    _audioTrackId(-1),
    _initialized(false),
    _imgBackground(),
    _imgFow()
{
    _undoStack = new QUndoStack(this);
    inputXML(element);
}

void Map::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    QDomElement element = doc.createElement( "map" );

    AdventureItem::outputXML(doc, element, targetDirectory);

    element.setAttribute( "name", getName() );
    element.setAttribute( "filename", targetDirectory.relativeFilePath(getFileName()) );
    element.setAttribute( "audiotrack", _audioTrackId );

    QDomElement actionsElement = doc.createElement( "actions" );
    int i;
    for(int i = 0; i < _markerList.count(); ++i)
    {
        QDomElement actionElement = doc.createElement( "action" );
        actionElement.setAttribute( "type", DMHelper::ActionType_SetMarker );
        _markerList.at(i).outputXML(actionElement);
        actionsElement.appendChild(actionElement);
    }

    for( i = 0; i < _undoStack->index(); ++i )
    {
        const UndoBase* action = dynamic_cast<const UndoBase*>(_undoStack->command(i));
        if(action)
        {
            QDomElement actionElement = doc.createElement( "action" );
            actionElement.setAttribute( "type", action->getType() );
            action->outputXML(doc, actionElement, targetDirectory);
            actionsElement.appendChild(actionElement);
        }
    }
    element.appendChild(actionsElement);

    parent.appendChild(element);
}

void Map::inputXML(const QDomElement &element)
{
    AdventureItem::inputXML(element);

    setName(element.attribute("name"));
    setFileName(element.attribute("filename"));
    _audioTrackId = element.attribute("audiotrack").toInt();

    QDomElement actionsElement = element.firstChildElement( QString("actions") );
    if( !actionsElement.isNull() )
    {
        QDomElement actionElement = actionsElement.firstChildElement( QString("action") );
        while( !actionElement.isNull() )
        {
            UndoBase* newAction = NULL;
            switch( actionElement.attribute( QString("type") ).toInt() )
            {
                case DMHelper::ActionType_Fill:
                    newAction = new UndoFill(*this, MapEditFill(QColor()));
                    break;
                case DMHelper::ActionType_Path:
                    newAction = new UndoPath(*this, MapDrawPath());
                    break;
                case DMHelper::ActionType_Point:
                    newAction = new UndoPoint(*this, MapDrawPoint(0, DMHelper::BrushType_Circle, true, QPoint()));
                    break;
                case DMHelper::ActionType_SetMarker:
                    {
                        MapMarker m(QPoint(0,0), QString(""), QString(""));
                        m.inputXML(actionElement);
                        _markerList.append(m);
                    }
                    break;
                case DMHelper::ActionType_Base:
                default:
                    break;
            }

            if(newAction)
            {
                newAction->inputXML(actionElement);
                _undoStack->push(newAction);
            }

            actionElement = actionElement.nextSiblingElement( QString("action") );
        }
    }

}

QString Map::getName() const
{
    return _name;
}

void Map::setName(const QString& newName)
{
    _name = newName;
    emit changed();
}

QString Map::getFileName() const
{
    return _filename;
}

void Map::setFileName(const QString& newFileName)
{
    _filename = newFileName;
    emit dirty();
}

AudioTrack* Map::getAudioTrack()
{
    Campaign* campaign = getCampaign();
    if(!campaign)
        return nullptr;

    return campaign->getTrackById(_audioTrackId);
}

int Map::getAudioTrackId()
{
    return _audioTrackId;
}

void Map::setAudioTrack(AudioTrack* track)
{
    int newTrackId = (track == nullptr) ? -1 : track->getID();
    if(_audioTrackId != newTrackId)
    {
        _audioTrackId = newTrackId;
        emit dirty();
    }
}

QUndoStack* Map::getUndoStack() const
{
    return _undoStack;
}

void Map::applyPaintTo(QImage* target, QColor clearColor, int count)
{
    bool preview = false;

    if(!target)
    {
        target = &_imgFow;
        preview = true;
    }

    if(count < 0)
        return;

    if(count > _undoStack->count())
        count = _undoStack->count();

    target->fill(clearColor);

    for( int i = 0; i < count; ++i )
    {
        const UndoBase* action = dynamic_cast<const UndoBase*>(_undoStack->command(i));
        if(action)
        {
            action->apply(preview, target);
        }
    }
}

MapFrame* Map::getRegisteredWindow() const
{
    return _mapFrame;
}

MapMarker* Map::getMapMarker(int id)
{
    // Search the map's marker list for existing markers
    int i;
    for(i = 0; i < _markerList.count(); ++i)
    {
        if(_markerList.at(i).getID() == id)
        {
            return &(_markerList[i]);
        }
    }

    /*
     * TODO FIX
    // Search the undo stack for new markers
    for(i = 0; i < _undoStack->count(); ++i)
    {
        const UndoMarker* undoItem = dynamic_cast<const UndoMarker*>(_undoStack->command(i));
        if(undoItem)
        {
            if(undoItem->marker().getID() == id)
            {
                return &(undoItem->marker());
            }
        }
    }
    */

    // Marker not found.
    return 0;
}

QImage Map::getBackgroundImage()
{
    return _imgBackground;
}

QImage Map::getFoWImage()
{
    return _imgFow;
}

void Map::registerWindow(MapFrame* mapFrame)
{
    _mapFrame = mapFrame;
    if(mapFrame != NULL)
    {
        for(int i = 0; i < _markerList.count(); ++i)
        {
            _mapFrame->addMapMarker(_markerList[i]);
        }
        connect(_mapFrame, SIGNAL(dirty()), this, SIGNAL(dirty()));
    }
}

void Map::unregisterWindow(MapFrame* mapFrame)
{
    if(mapFrame == _mapFrame)
    {
        disconnect(_mapFrame);
        registerWindow(NULL);
    }
}

void Map::initialize()
{
    if(_initialized)
        return;

    if((_filename.isNull()) || (_filename.isEmpty()))
        return;

    _imgBackground.load(_filename);

    if(_imgBackground.isNull())
        return;

    _imgFow = QImage(_imgBackground.size(), QImage::Format_ARGB32);
    applyPaintTo(0, QColor(0,0,0,128), _undoStack->index());

    _initialized = true;
}

void Map::uninitialize()
{
    _imgBackground = QImage();
    _imgFow = QImage();
    _initialized = false;
}

void Map::paintFoWPoint( QPoint point, const MapDraw& mapDraw, QPaintDevice* target, bool preview )
{
    if(!target)
    {
        target = &_imgFow;
    }

    QPainter p(target);
    p.setPen(Qt::NoPen);

    if(mapDraw.brushType() == DMHelper::BrushType_Circle)
    {
        if(mapDraw.erase())
        {
            QRadialGradient grad(point, mapDraw.radius());
            grad.setColorAt(0,QColor(0,0,0,0));
            grad.setColorAt(1.0 - (5.0/(qreal)mapDraw.radius()),QColor(0,0,0,0));
            grad.setColorAt(1,QColor(255,255,255));
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.setBrush(grad);
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
            p.setBrush(QColor(0,0,0,0));
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        }
        else
        {
            int alpha = preview ? 128 : 255;
            p.setBrush(QColor(0,0,0,alpha));
            p.setCompositionMode(QPainter::CompositionMode_Source);
        }

        p.drawRect( point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), mapDraw.radius() * 2, mapDraw.radius() * 2 );
    }
}

void Map::fillFoW( QColor color, QPaintDevice* target )
{
    if(!target)
    {
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

QImage Map::getShrunkPublishImage()
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
