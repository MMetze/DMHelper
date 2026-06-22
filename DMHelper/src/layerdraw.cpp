#include "layerdraw.h"
#include "layerscene.h"
#include "undodrawaddobject.h"
#include "layerdrawshape.h"
#include "publishglbattlebackground.h"
#include "publishglbattleobject.h"
#include "publishglscene.h"
#include <QPainter>
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QUndoStack>
#include <QOpenGLContext>
#include <cmath>

LayerDraw::LayerDraw(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    _glObjects{},
    _dirtyObjects{},
    _scene{nullptr},
    _playerInitialized{false},
    _layerDrawState{},
    _undoStack{new QUndoStack(this)}
{
    connect(&_layerDrawState, &LayerDrawState::objectAdded, this, &LayerDraw::handleObjectAdded);
    connect(&_layerDrawState, &LayerDrawState::objectRemoved, this, &LayerDraw::handleObjectRemoved);
}

LayerDraw::~LayerDraw()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerDraw::inputXML(const QDomElement &element, bool isImport)
{
    _layerDrawState.inputXML(element, isImport);

    Layer::inputXML(element, isImport);
}

QRectF LayerDraw::boundingRect() const
{
    return QRectF();
}

QImage LayerDraw::getLayerIcon() const
{
    return QImage(":/img/data/icon_pendashdot.png");
}

DMHelper::LayerType LayerDraw::getType() const
{
    return DMHelper::LayerType_Draw;
}

bool LayerDraw::defaultShader() const
{
    return false;
}

Layer* LayerDraw::clone() const
{
    LayerDraw* newLayer = new LayerDraw(this->getName(), this->getOrder());
    this->copyBaseValues(newLayer);
    return newLayer;
}

void LayerDraw::applyOrder(int order)
{
    for(QGraphicsItem* item : std::as_const(_graphicsItems))
    {
        if(item)
            item->setZValue(order);
    }
}

void LayerDraw::applyLayerVisibleDM(bool layerVisible)
{
    for(QGraphicsItem* item : std::as_const(_graphicsItems))
    {
        if(item)
            item->setVisible(layerVisible);
    }
}

void LayerDraw::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerDraw::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;
}

void LayerDraw::applyPosition(const QPoint& position)
{
    Q_UNUSED(position);
}

void LayerDraw::applySize(const QSize& size)
{
    Q_UNUSED(size);
}

QImage LayerDraw::getImage() const
{
    return QImage();
}

LayerDrawState& LayerDraw::getDrawState()
{
    return _layerDrawState;
}

QGraphicsItem* LayerDraw::createGraphicsItem(LayerDrawObject* drawObject)
{
    if((!drawObject) || (!_layerScene))
        return nullptr;

    if(_graphicsItems.contains(drawObject))
        return _graphicsItems.value(drawObject);

    QGraphicsScene* scene = _layerScene->getDMScene();
    if(!scene)
        return nullptr;

    QGraphicsItem* graphicsItem = nullptr;
    switch(drawObject->getType())
    {
        case DMHelper::ActionType_Path:
        {
            LayerDrawObjectPath* pathObject = dynamic_cast<LayerDrawObjectPath*>(drawObject);
            if(!pathObject)
                return nullptr;

            QPainterPath painterPath;
            const QList<QPointF>& points = pathObject->getPoints();
            if(points.count() < 2)
                return nullptr;

            painterPath.moveTo(points.first());
            for(int i = 1; i < points.size(); ++i)
                painterPath.lineTo(points[i]);

            QPen pathPen(QBrush(pathObject->getPenColor()), pathObject->getPenWidth(), pathObject->getPenStyle());
            QGraphicsPathItem* pathItem = new LayerDrawShapePath(painterPath, pathObject);
            pathItem->setPen(pathPen);
            scene->addItem(pathItem);

            pathItem->setPos(pathObject->getPosition());
            pathItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            pathItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            pathItem->setZValue(getOrder());

            graphicsItem = pathItem;
            break;
        }
        case DMHelper::ActionType_Line:
        {
            LayerDrawObjectLine* lineObject = dynamic_cast<LayerDrawObjectLine*>(drawObject);
            if(!lineObject)
                return nullptr;

            QLineF line(lineObject->getStartPoint(), lineObject->getEndPoint());
            QPen linePen(QBrush(lineObject->getPenColor()), lineObject->getPenWidth(), lineObject->getPenStyle());
            LayerDrawShapeLine* lineItem = new LayerDrawShapeLine(line, lineObject);
            lineItem->setPen(linePen);
            scene->addItem(lineItem);

            lineItem->setPos(lineObject->getPosition());
            lineItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            lineItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            lineItem->setZValue(getOrder());

            graphicsItem = lineItem;
            break;
        }
        case DMHelper::ActionType_Rect:
        {
            LayerDrawObjectRect* rectObject = dynamic_cast<LayerDrawObjectRect*>(drawObject);
            if(!rectObject)
                return nullptr;

            QPen rectPen(QBrush(rectObject->getPenColor()), rectObject->getPenWidth(), rectObject->getPenStyle());
            LayerDrawShapeRect* rectItem = new LayerDrawShapeRect(rectObject->getRect(), rectObject);
            rectItem->setPen(rectPen);
            if(rectObject->isFilled())
                rectItem->setBrush(QBrush(rectObject->getFillColor()));
            scene->addItem(rectItem);

            rectItem->setPos(rectObject->getPosition());
            rectItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            rectItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            rectItem->setZValue(getOrder());

            graphicsItem = rectItem;
            break;
        }
        case DMHelper::ActionType_Ellipse:
        {
            LayerDrawObjectEllipse* ellipseObject = dynamic_cast<LayerDrawObjectEllipse*>(drawObject);
            if(!ellipseObject)
                return nullptr;

            QPen ellipsePen(QBrush(ellipseObject->getPenColor()), ellipseObject->getPenWidth(), ellipseObject->getPenStyle());
            LayerDrawShapeEllipse* ellipseItem = new LayerDrawShapeEllipse(ellipseObject->getRect(), ellipseObject);
            ellipseItem->setPen(ellipsePen);
            if(ellipseObject->isFilled())
                ellipseItem->setBrush(QBrush(ellipseObject->getFillColor()));
            scene->addItem(ellipseItem);

            ellipseItem->setPos(ellipseObject->getPosition());
            ellipseItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            ellipseItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            ellipseItem->setZValue(getOrder());

            graphicsItem = ellipseItem;
            break;
        }
        case DMHelper::ActionType_Text:
        {
            LayerDrawObjectText* textObject = dynamic_cast<LayerDrawObjectText*>(drawObject);
            if(!textObject)
                return nullptr;

            LayerDrawShapeText* textItem = new LayerDrawShapeText(textObject->getText(), textObject);
            QFont font(textObject->getFontFamily(), textObject->getFontSize());
            textItem->setFont(font);
            textItem->setBrush(QBrush(textObject->getTextColor()));
            scene->addItem(textItem);

            textItem->setPos(textObject->getPosition());
            textItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            textItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            textItem->setZValue(getOrder());

            graphicsItem = textItem;
            break;
        }
        default:
            return nullptr;
    }

    if(graphicsItem)
        _graphicsItems.insert(drawObject, graphicsItem);

    return graphicsItem;
}

void LayerDraw::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    // Add existing objects to the scene
    QList<LayerDrawObject*> drawObjects = _layerDrawState.getObjects();
    for(LayerDrawObject* object : std::as_const(drawObjects))
    {
        if((object) && (!_graphicsItems.contains(object)))
            createGraphicsItem(object);
    }

    Layer::dmInitialize(scene);
}

void LayerDraw::dmUninitialize()
{
    cleanupDM();
}

void LayerDraw::dmUpdate()
{
}

void LayerDraw::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    if(!scene)
        return;

    _scene = scene;

    QList<LayerDrawObject*> drawObjects = _layerDrawState.getObjects();
    for(LayerDrawObject* object : std::as_const(drawObjects))
    {
        if((object) && (!_glObjects.contains(object)))
            createGLObject(object);
    }

    _playerInitialized = true;
    Layer::playerGLInitialize(renderer, scene);
}

void LayerDraw::playerGLUninitialize()
{
    _playerInitialized = false;
    cleanupPlayer();
}

void LayerDraw::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if(!functions)
        return;

    DMH_DEBUG_OPENGL_PAINTGL();

    if(_shaderProgramRGBA == 0)
        return;

    // Re-upload textures for any dirty objects
    for(LayerDrawObject* dirtyObj : std::as_const(_dirtyObjects))
    {
        if(!dirtyObj)
            continue;

        // Remove old GL object and recreate
        PublishGLBattleBackground* oldGLObj = _glObjects.take(dirtyObj);
        delete oldGLObj;
        createGLObject(dirtyObj);
    }
    _dirtyObjects.clear();

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    functions->glUseProgram(_shaderProgramRGBA);
    DMH_DEBUG_OPENGL_glUniformMatrix4fv4(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0);

    QList<LayerDrawObject*> drawObjects = _layerDrawState.getObjects();
    for(LayerDrawObject* object : std::as_const(drawObjects))
    {
        if(!object)
            continue;

        PublishGLBattleBackground* glObj = _glObjects.value(object);
        if(!glObj)
            continue;

        QMatrix4x4 localMatrix = glObj->getMatrix();
        localMatrix.translate(_position.x(), _position.y());
        DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, localMatrix.constData(), localMatrix);
        functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, localMatrix.constData());
        DMH_DEBUG_OPENGL_glUniform1f(_shaderAlphaRGBA, _opacityReference);
        functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);

        glObj->paintGL(functions, projectionMatrix);
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
    functions->glUseProgram(_shaderProgramRGB);
}

void LayerDraw::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

bool LayerDraw::playerIsInitialized()
{
    return _playerInitialized;
}

void LayerDraw::initialize(const QSize& sceneSize)
{
    if(getSize().isEmpty())
        setSize(sceneSize);
}

void LayerDraw::uninitialize()
{
}

void LayerDraw::addObject(LayerDrawObject* drawObject)
{
    if(!drawObject)
        return;

    _undoStack->push(new UndoDrawAddObject(&_layerDrawState, drawObject));
}

void LayerDraw::removeObject(LayerDrawObject* drawObject)
{
    if(!drawObject)
        return;

    _graphicsItems.remove(drawObject);
    _dirtyObjects.remove(drawObject);
    delete _glObjects.take(drawObject);
}

void LayerDraw::handleObjectAdded(LayerDrawObject* object, int index)
{
    Q_UNUSED(index);

    if(!object)
        return;

    if(!_graphicsItems.contains(object))
        createGraphicsItem(object);

    if(_playerInitialized)
        _dirtyObjects.insert(object);

    connect(object, &LayerDrawObject::objectMoved, this, &LayerDraw::handleObjectMoved);

    emit dirty();
    emit contentChanged();
}

void LayerDraw::handleObjectRemoved(LayerDrawObject* object, int index)
{
    Q_UNUSED(index);

    disconnect(object, &LayerDrawObject::objectMoved, this, &LayerDraw::handleObjectMoved);

    _dirtyObjects.remove(object);
    delete _glObjects.take(object);

    emit dirty();
    emit contentChanged();
}

void LayerDraw::handleObjectMoved(LayerDrawObject* object)
{
    if((!object) || (!_scene))
        return;

    PublishGLBattleBackground* glObj = _glObjects.value(object);
    if(!glObj)
    {
        // GL object hasn't been created yet; mark as dirty for next paint
        if(_playerInitialized)
            _dirtyObjects.insert(object);
        emit contentChanged();
        return;
    }

    // Recompute the world position for the moved object
    QRectF sceneBounds;
    renderObjectToImage(object, sceneBounds); // only need bounds, discard image
    QPointF worldPos = PublishGLBattleObject::sceneToWorld(_scene->getSceneRect(),
                                                          sceneBounds.topLeft() + object->getPosition());
    glObj->setPosition(QPoint(static_cast<int>(worldPos.x()), static_cast<int>(worldPos.y())));

    emit contentChanged();
}

void LayerDraw::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    _layerDrawState.outputXML(doc, element, targetDirectory, isExport);

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerDraw::cleanupDM()
{
    QList<QGraphicsItem*> items = _graphicsItems.values();
    for(QGraphicsItem* item : std::as_const(items))
    {
        if((item) && (item->scene()))
            item->scene()->removeItem(item);

        delete item;
    }
    _graphicsItems.clear();
}

void LayerDraw::cleanupPlayer()
{
    qDeleteAll(_glObjects);
    _glObjects.clear();
    _dirtyObjects.clear();

    _scene = nullptr;
}

PublishGLBattleBackground* LayerDraw::createGLObject(LayerDrawObject* drawObject)
{
    if((!drawObject) || (!_scene))
        return nullptr;

    QRectF sceneBounds;
    QImage image = renderObjectToImage(drawObject, sceneBounds);
    if(image.isNull())
        return nullptr;

    PublishGLBattleBackground* glObj = new PublishGLBattleBackground(_scene, image, GL_LINEAR);

    QPointF worldPos = PublishGLBattleObject::sceneToWorld(_scene->getSceneRect(),
                                                          sceneBounds.topLeft() + drawObject->getPosition());
    glObj->setPosition(QPoint(static_cast<int>(worldPos.x()), static_cast<int>(worldPos.y())));

    _glObjects.insert(drawObject, glObj);
    return glObj;
}

QImage LayerDraw::renderObjectToImage(LayerDrawObject* drawObject, QRectF& sceneBounds)
{
    if(!drawObject)
        return QImage();

    switch(drawObject->getType())
    {
        case DMHelper::ActionType_Path:
        {
            LayerDrawObjectPath* pathObject = dynamic_cast<LayerDrawObjectPath*>(drawObject);
            if(!pathObject)
                return QImage();

            const QList<QPointF>& points = pathObject->getPoints();
            if(points.count() < 2)
                return QImage();

            QPainterPath painterPath;
            painterPath.moveTo(points.first());
            for(int i = 1; i < points.size(); ++i)
                painterPath.lineTo(points[i]);

            qreal penHalf = pathObject->getPenWidth() / 2.0 + 1.0;
            QRectF pathRect = painterPath.boundingRect();
            sceneBounds = pathRect.adjusted(-penHalf, -penHalf, penHalf, penHalf);

            int imgW = qMax(1, static_cast<int>(std::ceil(sceneBounds.width())));
            int imgH = qMax(1, static_cast<int>(std::ceil(sceneBounds.height())));
            QImage image(imgW, imgH, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.translate(-sceneBounds.topLeft());
            QPen pathPen(QBrush(pathObject->getPenColor()), pathObject->getPenWidth(), pathObject->getPenStyle());
            painter.setPen(pathPen);
            painter.drawPath(painterPath);
            painter.end();

            return image;
        }
        case DMHelper::ActionType_Line:
        {
            LayerDrawObjectLine* lineObject = dynamic_cast<LayerDrawObjectLine*>(drawObject);
            if(!lineObject)
                return QImage();

            QLineF line(lineObject->getStartPoint(), lineObject->getEndPoint());
            qreal penHalf = lineObject->getPenWidth() / 2.0 + 1.0;
            QRectF lineRect = QRectF(line.p1(), line.p2()).normalized();
            sceneBounds = lineRect.adjusted(-penHalf, -penHalf, penHalf, penHalf);

            int imgW = qMax(1, static_cast<int>(std::ceil(sceneBounds.width())));
            int imgH = qMax(1, static_cast<int>(std::ceil(sceneBounds.height())));
            QImage image(imgW, imgH, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.translate(-sceneBounds.topLeft());
            QPen linePen(QBrush(lineObject->getPenColor()), lineObject->getPenWidth(), lineObject->getPenStyle());
            painter.setPen(linePen);
            painter.drawLine(line);
            painter.end();

            return image;
        }
        case DMHelper::ActionType_Rect:
        {
            LayerDrawObjectRect* rectObject = dynamic_cast<LayerDrawObjectRect*>(drawObject);
            if(!rectObject)
                return QImage();

            QRectF rect = rectObject->getRect();
            qreal penHalf = rectObject->getPenWidth() / 2.0 + 1.0;
            sceneBounds = rect.adjusted(-penHalf, -penHalf, penHalf, penHalf);

            int imgW = qMax(1, static_cast<int>(std::ceil(sceneBounds.width())));
            int imgH = qMax(1, static_cast<int>(std::ceil(sceneBounds.height())));
            QImage image(imgW, imgH, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.translate(-sceneBounds.topLeft());
            QPen rectPen(QBrush(rectObject->getPenColor()), rectObject->getPenWidth(), rectObject->getPenStyle());
            painter.setPen(rectPen);
            if(rectObject->isFilled())
                painter.setBrush(QBrush(rectObject->getFillColor()));
            painter.drawRect(rect);
            painter.end();

            return image;
        }
        case DMHelper::ActionType_Ellipse:
        {
            LayerDrawObjectEllipse* ellipseObject = dynamic_cast<LayerDrawObjectEllipse*>(drawObject);
            if(!ellipseObject)
                return QImage();

            QRectF rect = ellipseObject->getRect();
            qreal penHalf = ellipseObject->getPenWidth() / 2.0 + 1.0;
            sceneBounds = rect.adjusted(-penHalf, -penHalf, penHalf, penHalf);

            int imgW = qMax(1, static_cast<int>(std::ceil(sceneBounds.width())));
            int imgH = qMax(1, static_cast<int>(std::ceil(sceneBounds.height())));
            QImage image(imgW, imgH, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.translate(-sceneBounds.topLeft());
            QPen ellipsePen(QBrush(ellipseObject->getPenColor()), ellipseObject->getPenWidth(), ellipseObject->getPenStyle());
            painter.setPen(ellipsePen);
            if(ellipseObject->isFilled())
                painter.setBrush(QBrush(ellipseObject->getFillColor()));
            painter.drawEllipse(rect);
            painter.end();

            return image;
        }
        case DMHelper::ActionType_Text:
        {
            LayerDrawObjectText* textObject = dynamic_cast<LayerDrawObjectText*>(drawObject);
            if(!textObject)
                return QImage();

            QFont font(textObject->getFontFamily(), textObject->getFontSize());
            QFontMetricsF fm(font);
            QRectF textRect = fm.boundingRect(textObject->getText());
            // Shift to origin-based rect
            textRect.moveTopLeft(QPointF(0.0, 0.0));
            sceneBounds = textRect.adjusted(-1.0, -1.0, 1.0, 1.0);

            int imgW = qMax(1, static_cast<int>(std::ceil(sceneBounds.width())));
            int imgH = qMax(1, static_cast<int>(std::ceil(sceneBounds.height())));
            QImage image(imgW, imgH, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setFont(font);
            painter.setPen(QPen(textObject->getTextColor()));
            painter.drawText(sceneBounds, Qt::AlignLeft | Qt::AlignTop, textObject->getText());
            painter.end();

            return image;
        }
        default:
            return QImage();
    }
}

LayerDrawObject* LayerDraw::findObjectForItem(QGraphicsItem* item) const
{
    if(!item)
        return nullptr;

    for(auto it = _graphicsItems.constBegin(); it != _graphicsItems.constEnd(); ++it)
    {
        if(it.value() == item)
            return it.key();
    }

    return nullptr;
}

bool LayerDraw::eraseObjectAtPosition(const QPointF& scenePos)
{
    if(!_layerScene)
        return false;

    QGraphicsScene* scene = _layerScene->getDMScene();
    if(!scene)
        return false;

    QList<QGraphicsItem*> itemsAtPos = scene->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder);
    for(QGraphicsItem* sceneItem : std::as_const(itemsAtPos))
    {
        LayerDrawObject* foundObj = findObjectForItem(sceneItem);
        if(!foundObj)
            continue;

        // Remove the graphics item from the scene
        QGraphicsItem* gItem = _graphicsItems.take(foundObj);
        if(gItem)
        {
            if(gItem->scene())
                gItem->scene()->removeItem(gItem);
            delete gItem;
        }

        // Remove from state (this also cleans up GL objects via signal)
        _layerDrawState.takeObject(foundObj->getId());
        delete foundObj;
        return true;
    }

    return false;
}

void LayerDraw::deleteSelectedObjects()
{
    if(!_layerScene)
        return;

    QGraphicsScene* scene = _layerScene->getDMScene();
    if(!scene)
        return;

    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for(QGraphicsItem* sceneItem : std::as_const(selectedItems))
    {
        LayerDrawObject* foundObj = findObjectForItem(sceneItem);
        if(!foundObj)
            continue;

        QGraphicsItem* gItem = _graphicsItems.take(foundObj);
        if(gItem)
        {
            if(gItem->scene())
                gItem->scene()->removeItem(gItem);
            delete gItem;
        }

        _layerDrawState.takeObject(foundObj->getId());
        delete foundObj;
    }
}
