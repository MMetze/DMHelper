#include "layerfow.h"
#include "publishglbattlebackground.h"
#include "undofowbase.h"
#include "undofowfill.h"
#include "undofowpath.h"
#include "undofowpoint.h"
#include "undofowshape.h"
#include "undomarker.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QOpenGLFunctions>
#include <QImage>
#include <QUndoStack>
#include <QPainter>
#include <QDebug>

LayerFow::LayerFow(const QString& name, const QSize& imageSize, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
    _backgroundObject(nullptr),
    _imageSize(imageSize),
    _imgFow(),
    //_pixmapFow(),
    _undoStack(nullptr),
    _undoItems()
{
    _undoStack = new QUndoStack(); // TODO: why does not leaking this avoid a crash at shutdown?

    connect(this, &LayerFow::dirty, this, &LayerFow::updateFowInternal);
}

LayerFow::~LayerFow()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerFow::inputXML(const QDomElement &element, bool isImport)
{
    qDeleteAll(_undoItems);
    _undoItems.clear();

    // Load the actions
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
                case DMHelper::ActionType_SetMarker: // TODO: Layers - should markers be on the FOW layer? I think not...
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

    Layer::inputXML(element, isImport);
}

QRectF LayerFow::boundingRect() const
{
    return QRectF(QPointF(0,0), _imgFow.size());
}

QImage LayerFow::getLayerIcon() const
{
    return getImage();
}

DMHelper::LayerType LayerFow::getType() const
{
    return DMHelper::LayerType_Fow;
}

Layer* LayerFow::clone() const
{
    LayerFow* newLayer = new LayerFow(_name, _imgFow.size(), _order);

    newLayer->_layerVisible = _layerVisible;
    newLayer->_imgFow = _imgFow;
    //newLayer->_pixmapFow = _pixmapFow;

    if(_undoStack->count() > 0)
    {
        for(int i = 0; i < _undoStack->index(); ++i)
        {
            const UndoFowBase* action = dynamic_cast<const UndoFowBase*>(_undoStack->command(i));
            if((action) && (!action->isRemoved()))
            {
                UndoFowBase* newAction = action->clone();
                newAction->setLayer(newLayer);
                newLayer->_undoStack->push(newAction);
            }
        }

        newLayer->challengeUndoStack();
    }
    else if(_undoItems.count() > 0)
    {
        for(int i = 0; i < _undoItems.count(); ++i)
        {
            const UndoFowBase* action = dynamic_cast<const UndoFowBase*>(_undoItems.at(i));
            if((action) && (!action->isRemoved()))
            {
                UndoFowBase* newAction = action->clone();
                newAction->setLayer(newLayer);
                newLayer->_undoItems.append(newAction);
            }
        }
    }

    return newLayer;
}

QImage LayerFow::getImage() const
{
    return _imgFow;
}

QUndoStack* LayerFow::getUndoStack() const
{
    return _undoStack;
}

void LayerFow::undoPaint()
{
    qDebug() << "[LayerFow]: undoPaint";

    //_mapSource->applyPaintTo(nullptr, QColor(0,0,0,128), _mapSource->getUndoStack()->index() - 1)
    applyPaintTo(getUndoStack()->index() - 1);
}

void LayerFow::applyPaintTo(int index, int startIndex)
{
    qDebug() << "[LayerFow]: applyPaintTo, index: " << index << ", startIndex: " << startIndex;

    if(index < startIndex)
        return;

    if(index > _undoStack->count())
        index = _undoStack->count();

    if(startIndex == 0)
    {
        _imgFow.fill(Qt::black);
        //_pixmapFow.fill(Qt::blue);
    }

    // Need to add some batch processing to avoid updating every step
    for( int i = startIndex; i < index; ++i )
    {
        const UndoFowBase* constAction = dynamic_cast<const UndoFowBase*>(_undoStack->command(i));
        if(constAction)
        {
            UndoFowBase* action = const_cast<UndoFowBase*>(constAction);
            if(action)
                action->apply();
        }
    }
    /*
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
    */

    emit dirty();
}

/*
void LayerFow::internalApplyPaintTo(QImage* target, const QColor& clearColor, int index, bool preview, int startIndex)
{
    if((!target) || (index < startIndex))
        return;

    if(index > _undoStack->count())
        index = _undoStack->count();

    if(startIndex == 0)
        target->fill(clearColor);

    for( int i = startIndex; i < index; ++i )
    {
        const UndoFowBase* action = dynamic_cast<const UndoFowBase*>(_undoStack->command(i));
        if(action)
            action->apply(preview, target);
    }
}
*/

void LayerFow::paintFoWPoint(QPoint point, const MapDraw& mapDraw)
{
    QPainter p(&_imgFow);
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
            p.setBrush(QColor(0,0,0,255));
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
            p.setBrush(QColor(0,0,0,255));
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.drawRect(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), mapDraw.radius() * 2, mapDraw.radius() * 2);
        }
    }

    p.end();
    emit dirty();
}

void LayerFow::paintFoWRect(QRect rect, const MapEditShape& mapEditShape)
{
    QPainter p(&_imgFow);
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
        p.setBrush(QColor(0,0,0,255));
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawRect(rect);
    }

    p.end();
    emit dirty();
}

void LayerFow::fillFoW(const QColor& color)
{
    QPainter p(&_imgFow);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(0, 0, _imgFow.width(), _imgFow.height(), color);
    p.end();
    emit dirty();
}

/*
QImage LayerFow::getBWFoWImage()
{
    // TODO: get layer and extract BW FOW image
    //return getBWFoWImage(_layerScene.sceneSize().toSize());
    return _imgFow;
}

QImage LayerFow::getBWFoWImage(const QImage &img)
{
    return getBWFoWImage(img.size());
}

QImage LayerFow::getBWFoWImage(const QSize &size)
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
*/

QSize LayerFow::getImageSize() const
{
    return _imageSize;
}

void LayerFow::setImageSize(const QSize& imageSize)
{
    qDebug() << "[LayerFow]::setImageSize";
    if(imageSize == _imageSize)
        return;

    _imageSize = imageSize;

    if(!_imgFow.isNull())
    {
        uninitialize();
        initialize(_imageSize);
    }
}

void LayerFow::dmInitialize(QGraphicsScene& scene)
{
    if(_graphicsItem)
    {
        qDebug() << "[LayerFow] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    _graphicsItem = scene.addPixmap(QPixmap::fromImage(_imgFow));
    if(_graphicsItem)
    {
        _graphicsItem->setEnabled(false);
        _graphicsItem->setZValue(getOrder());
        _graphicsItem->setOpacity(0.5);
    }
}

void LayerFow::dmUninitialize()
{
    cleanupDM();
}

void LayerFow::dmUpdate()
{
}

void LayerFow::playerGLInitialize()
{
    qDebug() << "[LayerFow]::playerGLInitialize";
    if(_backgroundObject)
    {
        qDebug() << "[LayerFow] ERROR: playerGLInitialize called although the background object already exists!";
        return;
    }

    _backgroundObject = new PublishGLBattleBackground(nullptr, getImage(), GL_NEAREST);
}

void LayerFow::playerGLUninitialize()
{
    qDebug() << "[LayerFow]::playerGLUninitialize";
    cleanupPlayer();
}

/*
bool LayerFow::playerGLUpdate()
{
}
*/

void LayerFow::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    qDebug() << "[LayerFow]::playerGLPaint";
    Q_UNUSED(projectionMatrix);

    if(!functions)
        return;

    if(!_backgroundObject)
    {
        playerGLInitialize();
        if(!_backgroundObject)
            return;
    }

    functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
    _backgroundObject->paintGL();
}

void LayerFow::playerGLResize(int w, int h)
{
    qDebug() << "[LayerFow]::playerGLResize";
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void LayerFow::initialize(const QSize& layerSize)
{
    qDebug() << "[LayerFow]::initialize";
    if(!_imgFow.isNull())
        return;

    _imageSize = layerSize;
    _imgFow = QImage(_imageSize, QImage::Format_ARGB32_Premultiplied);
    //_imgFow.fill(Qt::black);
    //_pixmapFow.fill(Qt::black);

    initializeUndoStack();
}

void LayerFow::uninitialize()
{
    qDebug() << "[LayerFow]::uninitialize";
    _imgFow = QImage();
    //_pixmapFow = QPixmap();
}

void LayerFow::setOrder(int order)
{
    if(_graphicsItem)
        _graphicsItem->setZValue(order);

    Layer::setOrder(order);
}

void LayerFow::setLayerVisible(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);

    Layer::setLayerVisible(layerVisible);
}

void LayerFow::updateFowInternal()
{
    qDebug() << "[LayerFow]::updateFowInternal";
    if(_graphicsItem)
        _graphicsItem->setPixmap(QPixmap::fromImage(_imgFow));

    if(_backgroundObject)
        _backgroundObject->setImage(_imgFow);
}

void LayerFow::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    if(_undoStack->index() > 0)
    {
        // Check if we can skip some paint commands because they have been covered up by a fill
        challengeUndoStack();

        QDomElement actionsElement = doc.createElement("actions");
        for(int i = 0; i < _undoStack->index(); ++i )
        {
            const UndoFowBase* action = dynamic_cast<const UndoFowBase*>(_undoStack->command(i));
            if((action) && (!action->isRemoved()))
            {
                QDomElement actionElement = doc.createElement("action");
                actionElement.setAttribute("type", action->getType());
                action->outputXML(doc, actionElement, targetDirectory, isExport);
                actionsElement.appendChild(actionElement);
            }
        }
        element.appendChild(actionsElement);
    }
    else if(_undoItems.count() > 0)
    {
        QDomElement actionsElement = doc.createElement("actions");
        for(int i = 0; i < _undoItems.count(); ++i)
        {
            const UndoFowBase* action = dynamic_cast<const UndoFowBase*>(_undoItems.at(i));
            if((action) && (!action->isRemoved()))
            {
                QDomElement actionElement = doc.createElement("action");
                actionElement.setAttribute("type", action->getType());
                action->outputXML(doc, actionElement, targetDirectory, isExport);
                actionsElement.appendChild(actionElement);
            }
        }
        element.appendChild(actionsElement);
    }

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerFow::challengeUndoStack()
{
    bool filled = false;
    for(int i = _undoStack->index(); i >= 0; --i)
    {
        const UndoFowBase* constAction = dynamic_cast<const UndoFowBase*>(_undoStack->command(i));
        if(constAction)
        {
            if(filled)
            {
                if((constAction->getType() == DMHelper::ActionType_Fill) ||
                   (constAction->getType() == DMHelper::ActionType_Path) ||
                   (constAction->getType() == DMHelper::ActionType_Point) ||
                   (constAction->getType() == DMHelper::ActionType_Rect))
                {
                    UndoFowBase* action = const_cast<UndoFowBase*>(constAction);
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

void LayerFow::cleanupDM()
{
    if(!_graphicsItem)
        return;

    QGraphicsScene* scene = _graphicsItem->scene();
    if(scene)
        scene->removeItem(_graphicsItem);

    delete _graphicsItem;
    _graphicsItem = nullptr;
}

void LayerFow::cleanupPlayer()
{
    qDebug() << "[LayerFow]::cleanupPlayer";
    delete _backgroundObject;
    _backgroundObject = nullptr;
}

void LayerFow::initializeUndoStack()
{
    qDebug() << "[LayerFow]::initializeUndoStack";
    if(_undoItems.count() > 0)
    {
        while(_undoItems.count() > 0)
        {
            UndoFowBase* undoItem = _undoItems.takeFirst();
            if(undoItem)
            {
                undoItem->setLayer(this);
                _undoStack->push(undoItem);
            }
        }
    }
    else
    {
        applyPaintTo(getUndoStack()->index());
    }
}
