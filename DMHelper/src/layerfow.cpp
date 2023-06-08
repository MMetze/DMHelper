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
    _fowGLObject(nullptr),
    _scene(nullptr),
    //_imageSize(imageSize),
    _imageFow(),
    //_pixmapFow(),
    _undoStack(nullptr),
    _undoItems()
{
    setSize(imageSize);

    _undoStack = new QUndoStack(); // TODO: why does not leaking this avoid a crash at shutdown?

    //connect(this, &LayerFow::dirty, this, &LayerFow::updateFowInternal);
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
            switch(actionElement.attribute(QString("type")).toInt())
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
    return _imageFow.isNull() ? QRectF() : QRectF(_position, _imageFow.size());
}

QImage LayerFow::getLayerIcon() const
{
    return QImage(":/img/data/icon_fow2.png");
}

DMHelper::LayerType LayerFow::getType() const
{
    return DMHelper::LayerType_Fow;
}

Layer* LayerFow::clone() const
{
    LayerFow* newLayer = new LayerFow(_name, _imageFow.size(), _order);

    copyBaseValues(newLayer);
    newLayer->_imageFow = _imageFow;
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

void LayerFow::applyOrder(int order)
{
    if(_graphicsItem)
        _graphicsItem->setZValue(order);
}

void LayerFow::applyLayerVisible(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);
}

void LayerFow::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;

    if(_graphicsItem)
        _graphicsItem->setOpacity(opacity * 0.5);
}

void LayerFow::applyPosition(const QPoint& position)
{
    if(_graphicsItem)
        _graphicsItem->setPos(position);

    if(_fowGLObject)
    {
        QPoint pointTopLeft = _scene ? _scene->getSceneRect().toRect().topLeft() : QPoint();
        _fowGLObject->setPosition(QPoint(pointTopLeft.x() + position.x(), -pointTopLeft.y() - position.y()));
    }
}

void LayerFow::applySize(const QSize& size)
{
    if(size == _imageFow.size())
        return;

    if(!_imageFow.isNull())
        uninitialize();

    _size = size;
    initialize(size);

    QImage newImage = getImage();

    if(_graphicsItem)
        _graphicsItem->setPixmap(QPixmap::fromImage(newImage));

    if(_fowGLObject)
    {
//        _fowGLObject->setTargetSize(size);
//        _fowGLObject->setImage(newImage);

        delete _fowGLObject;
        _fowGLObject = nullptr;

    }
}

QImage LayerFow::getImage() const
{
    return _imageFow;
}

QUndoStack* LayerFow::getUndoStack() const
{
    return _undoStack;
}

void LayerFow::undoPaint()
{
    //_mapSource->applyPaintTo(nullptr, QColor(0, 0, 0, 128), _mapSource->getUndoStack()->index() - 1)
    applyPaintTo(getUndoStack()->index() - 1);
}

void LayerFow::applyPaintTo(int index, int startIndex)
{
    if(index < startIndex)
        return;

    if(index > _undoStack->count())
        index = _undoStack->count();

    if(startIndex == 0)
    {
        _imageFow.fill(Qt::black);
        //_pixmapFow.fill(Qt::blue);
    }

    // Need to add some batch processing to avoid updating every step
    for(int i = startIndex; i < index; ++i)
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

    updateFowInternal();
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

    for(int i = startIndex; i < index; ++i)
    {
        const UndoFowBase* action = dynamic_cast<const UndoFowBase*>(_undoStack->command(i));
        if(action)
            action->apply(preview, target);
    }
}
*/

void LayerFow::paintFoWPoint(QPoint point, const MapDraw& mapDraw)
{
    QPainter p(&_imageFow);
    p.setPen(Qt::NoPen);

    if(mapDraw.brushType() == DMHelper::BrushType_Circle)
    {
        if(mapDraw.erase())
        {
            if(mapDraw.smooth())
            {
                QRadialGradient grad(point, mapDraw.radius());
                grad.setColorAt(0, QColor(0, 0, 0, 0));
                grad.setColorAt(1.0 - (5.0/static_cast<qreal>(mapDraw.radius())), QColor(0, 0, 0, 0));
                grad.setColorAt(1, QColor(255, 255, 255));
                p.setBrush(grad);
            }
            else
            {
                p.setBrush(QColor(0, 0, 0, 0));
            }
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        }
        else
        {
            p.setBrush(QColor(0, 0, 0, 255));
            p.setCompositionMode(QPainter::CompositionMode_Source);
        }

        p.drawEllipse(point, mapDraw.radius(), mapDraw.radius());
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
                p.setBrush(QColor(0, 0, 0, 0));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(0, 0, 0, 50));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(0, 0, 0, 100));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(0, 0, 0, 150));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(0, 0, 0, 200));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
            }
            else
            {
                p.setBrush(QColor(0, 0, 0, 0));
                p.drawRect(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), mapDraw.radius() * 2, mapDraw.radius() * 2);
            }
        }
        else
        {
            p.setBrush(QColor(0, 0, 0, 255));
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.drawRect(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), mapDraw.radius() * 2, mapDraw.radius() * 2);
        }
    }

    p.end();
    updateFowInternal();
    emit dirty();
}

void LayerFow::paintFoWRect(QRect rect, const MapEditShape& mapEditShape)
{
    QPainter p(&_imageFow);
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
                            static_cast<qreal>(rect.height()) - rectHeight * 4 * 2);
            p.setBrush(QColor(0, 0, 0, 0));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(0, 0, 0, 50));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(0, 0, 0, 100));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(0, 0, 0, 150));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(0, 0, 0, 200));
            p.drawRect(baseRect);
        }
        else
        {
            p.setBrush(QColor(0, 0, 0, 0));
            p.drawRect(rect);
        }
    }
    else
    {
        p.setBrush(QColor(0, 0, 0, 255));
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawRect(rect);
    }

    p.end();
    updateFowInternal();
    emit dirty();
}

void LayerFow::fillFoW(const QColor& color)
{
    QPainter p(&_imageFow);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(0, 0, _imageFow.width(), _imageFow.height(), color);
    p.end();
    updateFowInternal();
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

    applyPaintTo(&_imgBWFow, QColor(0, 0, 0, 255), _undoStack->index(), false, _indexBWFow);
    _indexBWFow = qMax(_undoStack->index() - 1, 0);

    return _imgBWFow;
}
*/

/*
QSize LayerFow::getImageSize() const
{
    return _imageSize;
}

void LayerFow::setImageSize(const QSize& imageSize)
{
    if(imageSize == _imageSize)
        return;

    _imageSize = imageSize;

    if(!_imageFow.isNull())
    {
        uninitialize();
        initialize(_imageSize);
    }
}
*/

void LayerFow::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    if(_graphicsItem)
    {
        qDebug() << "[LayerFow] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    _graphicsItem = scene->addPixmap(QPixmap::fromImage(_imageFow));
    if(_graphicsItem)
    {
        _graphicsItem->setPos(_position);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());
    }

    Layer::dmInitialize(scene);
}

void LayerFow::dmUninitialize()
{
    cleanupDM();
}

void LayerFow::dmUpdate()
{
}

void LayerFow::playerGLInitialize(PublishGLScene* scene)
{
    if(_fowGLObject)
    {
        qDebug() << "[LayerFow] ERROR: playerGLInitialize called although the background object already exists!";
        return;
    }

    _scene = scene;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glUseProgram(_shaderProgramRGBA);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    _fowGLObject = new PublishGLBattleBackground(nullptr, getImage(), GL_NEAREST);

    Layer::playerGLInitialize(scene);
}

void LayerFow::playerGLUninitialize()
{
    cleanupPlayer();
}

/*
bool LayerFow::playerGLUpdate()
{
}
*/

void LayerFow::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if(!functions)
        return;

    functions->glUseProgram(_shaderProgramRGBA);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _fowGLObject->getMatrixData());
    functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);

    _fowGLObject->paintGL();

    functions->glUseProgram(_shaderProgramRGB);
}

void LayerFow::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

bool LayerFow::playerIsInitialized()
{
    return _fowGLObject != nullptr;
}

void LayerFow::initialize(const QSize& sceneSize)
{
    if(!_imageFow.isNull())
        return;

    //_imageSize = layerSize;
    if(getSize().isEmpty())
        setSize(sceneSize);

    _imageFow = QImage(getSize(), QImage::Format_ARGB32_Premultiplied);
    //_imgFow.fill(Qt::black);
    //_pixmapFow.fill(Qt::black);

    initializeUndoStack();
}

void LayerFow::uninitialize()
{
    _imageFow = QImage();
    //_pixmapFow = QPixmap();
}

void LayerFow::updateFowInternal()
{
    QImage newImage = getImage();

    if(_graphicsItem)
        _graphicsItem->setPixmap(QPixmap::fromImage(newImage));

    if(_fowGLObject)
    {
        delete _fowGLObject;
        _fowGLObject = nullptr;
    }
}

void LayerFow::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    if(_undoStack->index() > 0)
    {
        // Check if we can skip some paint commands because they have been covered up by a fill
        challengeUndoStack();

        QDomElement actionsElement = doc.createElement("actions");
        for(int i = 0; i < _undoStack->index(); ++i)
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
    delete _fowGLObject;
    _fowGLObject = nullptr;

    _scene = nullptr;
}

void LayerFow::initializeUndoStack()
{
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
