#include "layerfow.h"
#include "publishglbattlebackground.h"
#include "undofowbase.h"
#include "undofowfill.h"
#include "undofowpath.h"
#include "undofowpoint.h"
#include "undofowshape.h"
#include "undomarker.h"
#include "dmh_opengl.h"
#include "layerfowsettings.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QUndoStack>
#include <QPainter>
#include <QDebug>

const qreal LAYER_FOW_DM_OPACITY = 0.6;
const qreal LAYER_FOW_DM_DIP = 0.5;
const qreal LAYER_FOW_DM_RAISE = 1.3;

LayerFow::LayerFow(const QString& name, const QSize& imageSize, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
    _fowGLObject(nullptr),
    _scene(nullptr),
    _fowColor(Qt::black),
    _imageFow(),
    _imageFowTexture(),
    _fowTextureFile(),
    _fowTextureScale(),
    _undoStack(nullptr),
    _undoItems()
{
    _undoStack = new QUndoStack(); // TODO: why does not leaking this avoid a crash at shutdown?
    setSize(imageSize);
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

    _fowColor = QColor(element.attribute(QString("fowColor")));
    _fowTextureFile = element.attribute(QString("textureFile"));
    _fowTextureScale = element.attribute("textureScale").toInt();

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
                case DMHelper::ActionType_SetMarker: // Don't do anything with these in an FOW layer
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

bool LayerFow::hasSettings() const
{
    return true;
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
    newLayer->_imageFowTexture = _imageFowTexture;
    newLayer->_fowTextureScale = _fowTextureScale;

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

void LayerFow::applyLayerVisibleDM(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);
}

void LayerFow::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerFow::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;

    if(_graphicsItem)
        _graphicsItem->setOpacity(opacity * LAYER_FOW_DM_OPACITY);
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
        delete _fowGLObject;
        _fowGLObject = nullptr;
    }
}

QImage LayerFow::getImage() const
{
    if(_imageFowTexture.isNull())
        return _imageFow;

    QImage combinedImage = _imageFow;
    QPainter p(&combinedImage);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.drawImage(0, 0, _imageFowTexture);
    p.end();
    return combinedImage;
}

QUndoStack* LayerFow::getUndoStack() const
{
    return _undoStack;
}

void LayerFow::undoPaint()
{
    applyPaintTo(getUndoStack()->index() - 1);
}

void LayerFow::applyPaintTo(int index, int startIndex)
{
    if(index < startIndex)
        return;

    if(index > _undoStack->count())
        index = _undoStack->count();

    if(startIndex == 0)
        fillFoWImage();

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

    updateFowInternal();
    emit dirty();
}

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
                grad.setColorAt(0, QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
                grad.setColorAt(1.0 - (5.0/static_cast<qreal>(mapDraw.radius())), QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
                grad.setColorAt(1, _fowColor.rgb());
                p.setBrush(grad);
            }
            else
            {
                p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
            }
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        }
        else
        {
            p.setBrush(_fowColor);
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
                p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 50));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 100));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 150));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
                radius += border;
                p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 200));
                p.drawRect(QRectF(point.x() - radius, point.y() - radius, radius * 2, radius * 2));
            }
            else
            {
                p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
                p.drawRect(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), mapDraw.radius() * 2, mapDraw.radius() * 2);
            }
        }
        else
        {
            p.setBrush(_fowColor);
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
            p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 50));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 100));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 150));
            p.drawRect(baseRect);
            baseRect.translate(-rectWidth, -rectHeight);
            baseRect.setWidth(static_cast<qreal>(baseRect.width()) + rectWidth * 2);
            baseRect.setHeight(static_cast<qreal>(baseRect.height()) + rectHeight * 2);
            p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 200));
            p.drawRect(baseRect);
        }
        else
        {
            p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
            p.drawRect(rect);
        }
    }
    else
    {
        p.setBrush(_fowColor);
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
    p.fillRect(0, 0, _imageFow.width(), _imageFow.height(), QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), color.alpha()));
    p.end();
    updateFowInternal();
    emit dirty();
}

QRect LayerFow::getFoWVisibleRect() const
{
    QColor testColorTL = _imageFow.pixelColor(0, 0);
    QColor testColorTR = _imageFow.pixelColor(_imageFow.width(), 0);
    QColor testColorBL = _imageFow.pixelColor(0, _imageFow.height());
    QColor testColorBR = _imageFow.pixelColor(_imageFow.width(), _imageFow.height());
    QColor testColorMid = _imageFow.pixelColor(_imageFow.width() / 2, _imageFow.height() / 2);
    QColor testColor;

    int top, bottom, left, right;
    top = bottom = left = right = -1;
    int i, j;
    for(j = 0; (j < _imageFow.height()) && (top == -1); ++j)
    {
        for(i = 0; (i < _imageFow.width()) && (top == -1); ++i)
        {
            testColor = _imageFow.pixelColor(i, j);
            if(_imageFow.pixelColor(i, j).alpha() < 255)
                top = j;
        }
    }

    for(j = _imageFow.height() - 1; (j > top) && (bottom == -1); --j)
    {
        for(i = 0; (i < _imageFow.width()) && (bottom == -1); ++i)
        {
            testColor = _imageFow.pixelColor(i, j);
            if(_imageFow.pixelColor(i, j).alpha() < 255)
                bottom = j;
        }
    }

    for(i = 0; (i < _imageFow.width()) && (left == -1); ++i)
    {
        for(j = top; (j < bottom) && (left == -1); ++j)
        {
            testColor = _imageFow.pixelColor(i, j);
            if(_imageFow.pixelColor(i, j).alpha() < 255)
                left = i;
        }
    }

    for(i = _imageFow.width() - 1; (i > left) && (right == -1); --i)
    {
        for(j = top; (j < bottom) && (right == -1); ++j)
        {
            testColor = _imageFow.pixelColor(i, j);
            if(_imageFow.pixelColor(i, j).alpha() < 255)
                right = i;
        }
    }

    return QRect(left, top, right - left, bottom - top);
}

void LayerFow::dipOpacity()
{
    if(_graphicsItem)
        _graphicsItem->setOpacity(_opacityReference * LAYER_FOW_DM_OPACITY * LAYER_FOW_DM_DIP);
}

void LayerFow::raiseOpacity()
{
    if(_graphicsItem)
        _graphicsItem->setOpacity(_opacityReference * LAYER_FOW_DM_OPACITY * LAYER_FOW_DM_RAISE);
}

void LayerFow::resetOpacity()
{
    if(_graphicsItem)
        _graphicsItem->setOpacity(_opacityReference * LAYER_FOW_DM_OPACITY);
}

void LayerFow::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    if(_graphicsItem)
    {
        qDebug() << "[LayerFow] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    _graphicsItem = scene->addPixmap(QPixmap::fromImage(getImage()));
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

void LayerFow::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    if(_fowGLObject)
    {
        qDebug() << "[LayerFow] ERROR: playerGLInitialize called although the background object already exists!";
        return;
    }

    _scene = scene;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    f->glUseProgram(_shaderProgramRGBA);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    _fowGLObject = new PublishGLBattleBackground(nullptr, getImage(), GL_NEAREST);

    Layer::playerGLInitialize(renderer, scene);
}

void LayerFow::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerFow::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if(!functions)
        return;

    DMH_DEBUG_OPENGL_PAINTGL();

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    functions->glUseProgram(_shaderProgramRGBA);
    DMH_DEBUG_OPENGL_glUniformMatrix4fv4(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _fowGLObject->getMatrixData(), _fowGLObject->getMatrix());
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _fowGLObject->getMatrixData());
    DMH_DEBUG_OPENGL_glUniform1f(_shaderAlphaRGBA, _opacityReference);
    functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
    _fowGLObject->paintGL(functions, projectionMatrix);

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

    if(getSize().isEmpty())
        setSize(sceneSize);

    _imageFow = QImage(getSize(), QImage::Format_ARGB32_Premultiplied);
    _imageFowTexture = QImage(getSize(), QImage::Format_ARGB32_Premultiplied);
    fillFoWImage();

    initializeUndoStack();
}

void LayerFow::uninitialize()
{
    _imageFow = QImage();
}

void LayerFow::editSettings()
{
    LayerFowSettings* dlg = new LayerFowSettings();

    dlg->setFowColor(_fowColor);
    dlg->setFowTextureFile(_fowTextureFile);
    dlg->setFowScale(_fowTextureScale);

    dlg->exec();

    _fowColor = dlg->fowColor();
    _fowTextureFile = dlg->fowTextureFile();
    _fowTextureScale = dlg->fowScale();

    fillFoWImage();
    updateFowInternal();

    dlg->deleteLater();
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
    if(_fowColor != Qt::black)
        element.setAttribute("fowColor", _fowColor.name(QColor::HexArgb));

    if(!_fowTextureFile.isEmpty())
        element.setAttribute("textureFile", targetDirectory.relativeFilePath(_fowTextureFile));

    if(_fowTextureScale > 0)
        element.setAttribute("textureScale", QString::number(_fowTextureScale));

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

void LayerFow::fillFoWImage()
{
    // Todo: Use QBrush to draw tiled scaled images to the image
    _imageFow.fill(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue()));
    if(_fowTextureFile.isEmpty())
    {
        _imageFowTexture = QImage();
    }
    else
    {
        if(_imageFowTexture.isNull())
            _imageFowTexture = _imageFow;
        else
            _imageFowTexture.fill(_fowColor);

        QImage newTexture(_fowTextureFile);
        if(!newTexture.isNull())
        {
            newTexture.convertTo(QImage::Format_ARGB32_Premultiplied);
            newTexture = newTexture.scaled(_imageFowTexture.size() * _fowTextureScale / 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QPainter p(&_imageFowTexture);
                if(_fowColor.alpha() == 0)
                    p.setCompositionMode(QPainter::CompositionMode_Source);
                for(int x = 0; x < _imageFowTexture.width(); x += newTexture.width())
                    for(int y = 0; y < _imageFowTexture.height(); y += newTexture.height())
                        p.drawImage(x, y, newTexture);
            p.end();
        }
    }
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


