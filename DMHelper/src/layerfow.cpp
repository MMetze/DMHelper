#include "layerfow.h"
#include "fowgraphicsitem.h"
#include "publishglbattlebackground.h"
#include "undofowbase.h"
#include "undofowfill.h"
#include "undofowpath.h"
#include "undofowpoint.h"
#include "undofowshape.h"
#include "undofowpolygon.h"
#include "undomarker.h"
#include "dmh_opengl.h"
#include "layerfowsettings.h"
#include <QGraphicsScene>
#include <QImage>
#include <QTimer>
#include <QUndoStack>
#include <QPainter>
#include <QDebug>
#include <QElapsedTimer>

const qreal LAYER_FOW_DM_OPACITY = 0.6;
const qreal LAYER_FOW_DM_DIP = 0.3;
const qreal LAYER_FOW_DM_RAISE = 1.0;

// Coalescing window for deferred FOW updates: at most one dispatchFowUpdate() call
// per this many milliseconds during an active brush stroke.
static constexpr int FOW_UPDATE_COALESCE_MS = 16;

// Timing probe: records when the coalescer was last started (requestFowUpdate, first
// call in a burst) so dispatchFowUpdate can log how long the coalescer actually waited.
static QElapsedTimer s_coalesceStart;

// Records the wall-clock time of the most recent dispatchFowUpdate() call.
// Used in requestFowUpdate() to detect the "coalescer inactive but >= 16ms since
// last dispatch" case and dispatch directly instead of starting a fresh WM_TIMER.
static QElapsedTimer s_lastDispatch;

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
    _undoItems(),
    _batchProcessing(false),
    _fowGLImageDirty(false),
    _fowGLDeferredDestroy(false),
    _pendingDirtyRect(),
    _fowGLPendingRegion(),
    _cachedImage(),
    _updateCoalescer(nullptr)
{
    _updateCoalescer = new QTimer(this);
    _updateCoalescer->setSingleShot(true);
    _updateCoalescer->setInterval(FOW_UPDATE_COALESCE_MS);
    connect(_updateCoalescer, &QTimer::timeout, this, [this]() {
        // Deferred-upload contract — this slot runs on the GUI thread with no current GL
        // context. It MUST NOT call into _fowGLObject’s GL-mutating methods. The chunk-1
        // pending-flag pattern is the only permitted path to a GL upload: dispatchFowUpdate()
        // sets _fowGLImageDirty = true; the next playerGLPaint() consumes the flag via
        // applyGLPendingUpdates(). changed() (emitted inside dispatchFowUpdate) wakes the
        // player renderer. Reset _pendingDirtyRect before dispatching so any requestFowUpdate()
        // call that arrives during the dispatch itself starts a fresh accumulation.
        QRect region = _pendingDirtyRect;
        _pendingDirtyRect = QRect();
        s_lastDispatch.restart();
        dispatchFowUpdate(region);
    });

    _undoStack = new QUndoStack(this);
    setSize(imageSize);
}

LayerFow::~LayerFow()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerFow::aboutToDelete()
{
    if(_undoStack)
        _undoStack->clear();
    qDeleteAll(_undoItems);
    _undoItems.clear();
    Layer::aboutToDelete();
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
                case DMHelper::ActionType_Polygon:
                    newAction = new UndoFowPolygon(nullptr, MapEditPolygon(QPolygon(), true, false));
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

    if(_graphicsItem)
    {
        // Notify the scene that the bounding rect is changing, then refresh the
        // cached image and request a full repaint of the new bounds.
        _graphicsItem->notifyGeometryChange();
        dispatchFowUpdate(QRect(QPoint(), _imageFow.size()));
    }

    // Defer GL object destruction to playerGLPaint() / playerGLUninitialize(), which run
    // with a current GL context. The previous GL object remains alive and rendered for at
    // most one player-window paint cycle after a resize; this is acceptable because resize
    // already invalidates the FOW image content and the next playerGLInitialize() call
    // (triggered when playerIsInitialized() returns false) will rebuild on the following cycle.
    if(_fowGLObject)
        _fowGLDeferredDestroy = true;
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

    _batchProcessing = true;
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
    _batchProcessing = false;

    // Undo/redo replay: call dispatchFowUpdate() directly rather than via the coalescer.
    // The replay is an atomic operation that must flush immediately so the UI reflects the
    // correct state before this function returns. The coalescer is intentionally bypassed.
    dispatchFowUpdate(QRect(QPoint(), _imageFow.size()));
    emit dirty();
}

void LayerFow::paintFoWPoint(QPoint point, const MapDraw& mapDraw)
{
    static int s_paintPointCount = 0;
    QElapsedTimer t;
    t.start();

    if(mapDraw.erase() && mapDraw.smooth())
        rebuildBrushStamp(mapDraw);

    QPainter p(&_imageFow);
    p.setPen(Qt::NoPen);

    if(mapDraw.brushType() == DMHelper::BrushType_Circle)
    {
        if(mapDraw.erase())
        {
            if(mapDraw.smooth())
            {
                // Stamp: pre-computed gradient baked into alpha channel. One drawImage +
                // DestinationIn is cheaper than per-call radial gradient rasterization.
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                p.drawImage(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), _brushStamp);
            }
            else
            {
                // Source + transparent: same result as DestinationIn + alpha=0 but
                // uses Qt's SIMD solid-fill path instead of the scalar DestinationIn path.
                p.setBrush(Qt::transparent);
                p.setCompositionMode(QPainter::CompositionMode_Source);
                p.drawEllipse(point, mapDraw.radius(), mapDraw.radius());
            }
        }
        else
        {
            p.setBrush(_fowColor);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.drawEllipse(point, mapDraw.radius(), mapDraw.radius());
        }
    }
    else
    {
        if(mapDraw.erase())
        {
            if(mapDraw.smooth())
            {
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                p.drawImage(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), _brushStamp);
            }
            else
            {
                p.setBrush(Qt::transparent);
                p.setCompositionMode(QPainter::CompositionMode_Source);
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
    const qint64 painterUs = t.nsecsElapsed() / 1000;
    if(!_batchProcessing)
    {
        const int r = mapDraw.radius();
        requestFowUpdate(QRect(point.x() - r, point.y() - r, r * 2 + 1, r * 2 + 1));
    }
    if(++s_paintPointCount % 10 == 0)
        qDebug() << "[FoW-perf] paintFoWPoint #" << s_paintPointCount
                 << " painter:" << painterUs << "us"
                 << " total:" << t.nsecsElapsed() / 1000 << "us";
}

void LayerFow::paintFoWPoints(const QList<QPoint>& points, const MapDraw& mapDraw)
{
    if(points.isEmpty())
        return;

    if(mapDraw.erase() && mapDraw.smooth())
        rebuildBrushStamp(mapDraw);

    QPainter p(&_imageFow);
    p.setPen(Qt::NoPen);

    if(mapDraw.brushType() == DMHelper::BrushType_Circle)
    {
        if(mapDraw.erase())
        {
            if(mapDraw.smooth())
            {
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                for(const QPoint& point : points)
                    p.drawImage(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), _brushStamp);
            }
            else
            {
                p.setBrush(Qt::transparent);
                p.setCompositionMode(QPainter::CompositionMode_Source);
                for(const QPoint& point : points)
                    p.drawEllipse(point, mapDraw.radius(), mapDraw.radius());
            }
        }
        else
        {
            p.setBrush(_fowColor);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            for(const QPoint& point : points)
                p.drawEllipse(point, mapDraw.radius(), mapDraw.radius());
        }
    }
    else
    {
        if(mapDraw.erase())
        {
            if(mapDraw.smooth())
            {
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                for(const QPoint& point : points)
                    p.drawImage(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), _brushStamp);
            }
            else
            {
                p.setBrush(Qt::transparent);
                p.setCompositionMode(QPainter::CompositionMode_Source);
                for(const QPoint& point : points)
                    p.drawRect(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), mapDraw.radius() * 2, mapDraw.radius() * 2);
            }
        }
        else
        {
            p.setBrush(_fowColor);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            for(const QPoint& point : points)
                p.drawRect(point.x() - mapDraw.radius(), point.y() - mapDraw.radius(), mapDraw.radius() * 2, mapDraw.radius() * 2);
        }
    }

    p.end();
    if(!_batchProcessing)
    {
        const int r = mapDraw.radius();
        QRect footprint;
        for(const QPoint& pt : points)
            footprint = footprint.united(QRect(pt.x() - r, pt.y() - r, r * 2 + 1, r * 2 + 1));
        requestFowUpdate(footprint);
    }
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
    if(!_batchProcessing)
    {
        dispatchFowUpdate(rect);
        emit dirty();
    }
}

void LayerFow::paintFoWPolygon(const MapEditPolygon& mapEditPolygon)
{
    QPainter p(&_imageFow);
    p.setPen(Qt::NoPen);

    if(mapEditPolygon.erase())
    {
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
        p.drawPolygon(mapEditPolygon.polygon());
    }
    else
    {
        p.setBrush(_fowColor);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPolygon(mapEditPolygon.polygon());
    }

    p.end();
    if(!_batchProcessing)
    {
        dispatchFowUpdate(mapEditPolygon.polygon().boundingRect());
        emit dirty();
    }
}

void LayerFow::fillFoW(const QColor& color)
{
    QPainter p(&_imageFow);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(0, 0, _imageFow.width(), _imageFow.height(), QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), color.alpha()));
    p.end();
    if(!_batchProcessing)
    {
        dispatchFowUpdate(QRect(QPoint(), _imageFow.size()));
        emit dirty();
    }
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

    _cachedImage = getImage().convertToFormat(QImage::Format_RGBA8888);
    _graphicsItem = new FowGraphicsItem(&_cachedImage);
    _graphicsItem->setPos(_position);
    _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
    _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    _graphicsItem->setZValue(getOrder());
    scene->addItem(_graphicsItem);

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

    _fowGLObject = new PublishGLBattleBackground(nullptr, getImage().convertToFormat(QImage::Format_RGBA8888), GL_NEAREST,
                                                 /*sourceIsRgba8888=*/true,
                                                 /*sourceNeedsVerticalFlip=*/false);

    Layer::playerGLInitialize(renderer, scene);
}

void LayerFow::playerGLUninitialize()
{
    applyGLPendingUpdates();
    cleanupPlayer();
}

void LayerFow::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    applyGLPendingUpdates();

    Q_UNUSED(defaultModelMatrix);

    if(!_fowGLObject)
        return;

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

    // Format_ARGB32_Premultiplied: Qt's software rasterizer has SIMD-optimised
    // compositing paths for this format. CPU painting (paintFoWPoint, paintFoWPoints)
    // is the dominant per-event cost; keeping _imageFow in the native rasterizer
    // format avoids falling back to scalar loops on every brush stroke.
    // _cachedImage is produced as RGBA8888 by dispatchFowUpdate() so the GL upload
    // path can call glTexSubImage2D directly without an extra conversion.
    _imageFow = QImage(getSize(), QImage::Format_ARGB32_Premultiplied);
    // Same format as _imageFow so getImage() compositing stays within
    // ARGB32_Premultiplied throughout the CPU painting path.
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
    dispatchFowUpdate(QRect(QPoint(), _imageFow.size()));
    emit dirty();

    dlg->deleteLater();
}

void LayerFow::applyGLPendingUpdates()
{
    // This is the ONLY function in LayerFow permitted to call GL-state-mutating methods
    // on _fowGLObject (constructor, updateImage, setImage, destructor). Every other call
    // site in LayerFow sets _fowGLImageDirty or _fowGLDeferredDestroy instead of touching
    // _fowGLObject directly. This function is only ever invoked from *GL* functions where
    // the player window's GL context is current.
    if(_fowGLDeferredDestroy)
    {
        delete _fowGLObject;
        _fowGLObject = nullptr;
        _fowGLDeferredDestroy = false;
        _fowGLImageDirty = false; // image will be re-uploaded after the next playerGLInitialize()
        _fowGLPendingRegion = QRect(); // cleared; new object will upload full image on initialize
    }
    else if(_fowGLImageDirty && _fowGLObject)
    {
        QElapsedTimer tGL;
        tGL.start();
        const QRect fullImageRect(QPoint(), _imageFow.size());
        bool partialUpload = false;
        if(_fowGLPendingRegion.isValid() && _fowGLPendingRegion != fullImageRect)
        {
            // Partial region: clamp defensively then use glTexSubImage2D for an
            // efficient sub-image upload. _cachedImage is already up-to-date for
            // the dirty region (updated in dispatchFowUpdate before this runs).
            const QRect safeRegion = _fowGLPendingRegion.intersected(fullImageRect);
            if(safeRegion.isValid())
            {
                _fowGLObject->updateImageRegion(_cachedImage, safeRegion);
                partialUpload = true;
            }
            else
                _fowGLObject->updateImage(_cachedImage);
        }
        else
        {
            // Full image or no pending region: upload the complete texture.
            _fowGLObject->updateImage(_cachedImage);
        }
        qDebug() << "[FoW-perf] applyGLPendingUpdates"
                 << (partialUpload ? "partial" : "full")
                 << "region=" << _fowGLPendingRegion
                 << " glUpload:" << tGL.nsecsElapsed() / 1000 << "us";
        _fowGLImageDirty = false;
        _fowGLPendingRegion = QRect();
    }
}

void LayerFow::flushPendingUpdate()
{
    // If the coalescing timer is pending, cancel it and apply the deferred update
    // immediately. This is called by stroke-end handlers (mouse-up) so the player
    // renderer sees consistent imagery before the caller emits dirty().
    // Does NOT emit dirty() — the caller is responsible for that.
    if(_updateCoalescer->isActive())
    {
        _updateCoalescer->stop();
        QRect region = _pendingDirtyRect;
        _pendingDirtyRect = QRect();
        dispatchFowUpdate(region);
    }
}

void LayerFow::requestFowUpdate(const QRect& region)
{
    // Clamp to image bounds so out-of-image brush strokes (partially above or to
    // the right of the map) do not produce a negative-y or oversized pending region
    // that would cause QImage::constScanLine() to assert in updateImageRegion().
    if(_imageFow.isNull())
        return;
    const QRect clamped = region.intersected(QRect(QPoint(), _imageFow.size()));
    if(clamped.isEmpty())
        return;
    // Union the new footprint into the pending dirty rect. Subsequent calls within
    // the same FOW_UPDATE_COALESCE_MS window accumulate into the union; a single
    // dispatchFowUpdate fires when the timer expires.
    _pendingDirtyRect = _pendingDirtyRect.united(clamped);
    if(!_updateCoalescer->isActive()) {
        // Coalescer is inactive: the WM_TIMER already fired (or this is the first call).
        // If >= 16ms has elapsed since the last dispatch we are already inside the mouse-
        // event handler (posted-event priority) — dispatch directly now rather than
        // starting a fresh WM_TIMER that will be starved for another ~35ms.
        // Keep the coalescer running as a tail-timer for the final stamp when the mouse stops.
        if(s_lastDispatch.isValid() && s_lastDispatch.elapsed() >= FOW_UPDATE_COALESCE_MS) {
            QRect pendingRegion = _pendingDirtyRect;
            _pendingDirtyRect = QRect();
            s_coalesceStart.restart();
            s_lastDispatch.restart();
            _updateCoalescer->start();
            dispatchFowUpdate(pendingRegion);
        } else {
            s_coalesceStart.restart();
            _updateCoalescer->start();
        }
    } else if(s_coalesceStart.elapsed() >= FOW_UPDATE_COALESCE_MS) {
        // WM_TIMER is starved while the mouse is dragging: hardware input keeps the
        // Windows message queue non-empty so WM_TIMER never fires. We're called from
        // a Qt mouse-event handler (posted-event priority, reliably scheduled), so
        // dispatch here instead of waiting. Restart the coalescer so the final stamp
        // after the mouse stops is still delivered.
        _updateCoalescer->stop();
        QRect pendingRegion = _pendingDirtyRect;
        _pendingDirtyRect = QRect();
        s_coalesceStart.restart();
        s_lastDispatch.restart();
        _updateCoalescer->start();
        dispatchFowUpdate(pendingRegion);
    }
}

void LayerFow::dispatchFowUpdate(const QRect& region)
{
    // Deferred-upload contract — runs on the GUI thread. Sets pending flags and emits
    // signals; never calls into _fowGLObject. The next playerGLPaint() consumes
    // _fowGLImageDirty / _fowGLPendingRegion via applyGLPendingUpdates().
    // Guard: an empty region means there are no pending stamps to dispatch. This can
    // happen when the coalescer tail-timer fires after the elapsed-time path already
    // cleared _pendingDirtyRect. QRectF(QRect()) is null, and Qt interprets a null
    // QRectF passed to QGraphicsItem::update() as "update full bounding rect", so we
    // must return before reaching _graphicsItem->updateRegion().
    if(region.isEmpty())
        return;

    static QElapsedTimer s_dispatchInterval;
    static bool s_dispatchIntervalFirst = true;
    qint64 dispatchIntervalUs = 0;
    if(!s_dispatchIntervalFirst)
        dispatchIntervalUs = s_dispatchInterval.nsecsElapsed() / 1000;
    s_dispatchInterval.restart();
    s_dispatchIntervalFirst = false;

    QElapsedTimer t;
    t.start();

    // Partial composite update: rebuild only the dirty region of _cachedImage.
    // _imageFow is ARGB32_Premultiplied; _cachedImage is RGBA8888 (for GL upload).
    // The format difference is intentional — do NOT use _imageFow.format() as the
    // expected format here. Trigger a full rebuild only on first call (isNull) or
    // after applySize() changes the map dimensions.
    if(_cachedImage.isNull() || _cachedImage.size() != _imageFow.size() || _cachedImage.format() != QImage::Format_RGBA8888)
    {
        _cachedImage = getImage().convertToFormat(QImage::Format_RGBA8888);
    }
    else if(_imageFowTexture.isNull())
    {
        // No texture: copy dirty region from _imageFow directly.
        QPainter fowPainter(&_cachedImage);
        fowPainter.setCompositionMode(QPainter::CompositionMode_Source);
        fowPainter.drawImage(region, _imageFow, region);
    }
    else
    {
        // Has texture: composite dirty region — first copy _imageFow then apply texture.
        QPainter fowPainter(&_cachedImage);
        fowPainter.setCompositionMode(QPainter::CompositionMode_Source);
        fowPainter.drawImage(region, _imageFow, region);
        fowPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        fowPainter.drawImage(region, _imageFowTexture, region);
    }

    const qint64 compositeUs = t.nsecsElapsed() / 1000;

    // Wake the DM-side item to repaint the changed region only.
    if(_graphicsItem)
        _graphicsItem->updateRegion(region);
    const qint64 updateRegionUs = t.nsecsElapsed() / 1000;

    // Queue a full-image GL upload for the player window. The _fowGLPendingRegion
    // accumulator is populated for chunk-4 use; chunk-3 applyGLPendingUpdates()
    // still uploads the full image.
    _fowGLImageDirty = true;
    _fowGLPendingRegion = _fowGLPendingRegion.united(region);

    // Wake the player renderer.
    emit changed();

    qDebug() << "[FoW-perf] dispatchFowUpdate interval:" << dispatchIntervalUs << "us"
             << " coalesceAge:" << (s_coalesceStart.isValid() ? s_coalesceStart.nsecsElapsed() / 1000 : -1) << "us"
             << " composite:" << compositeUs << "us"
             << " updateRegion:" << updateRegionUs - compositeUs << "us"
             << " total:" << t.nsecsElapsed() / 1000 << "us"
             << " region=" << region;
}

void LayerFow::rebuildBrushStamp(const MapDraw& mapDraw)
{
    const int r = mapDraw.radius();
    if(!_brushStamp.isNull() &&
       _brushStamp.width() == 2 * r + 1 &&
       _brushStampBrushType == mapDraw.brushType() &&
       _brushStampSmooth == mapDraw.smooth() &&
       _brushStampFowColor == _fowColor.rgba())
        return;

    _brushStampBrushType = mapDraw.brushType();
    _brushStampSmooth = mapDraw.smooth();
    _brushStampFowColor = _fowColor.rgba();

    // Fill with full-opacity fowColor (alpha=255). Pixels outside the brush shape
    // then act as "preserve destination" when applied with CompositionMode_DestinationIn
    // (dest × 255/255 = dest, unchanged).
    _brushStamp = QImage(QSize(2 * r + 1, 2 * r + 1), QImage::Format_ARGB32_Premultiplied);
    _brushStamp.fill(_fowColor.rgba());

    QPainter sp(&_brushStamp);
    sp.setPen(Qt::NoPen);

    if(mapDraw.brushType() == DMHelper::BrushType_Circle)
    {
        // Pre-render the radial gradient inside the ellipse with Source composition.
        // Outside the ellipse the fill (alpha=255) remains, so DestinationIn applied
        // to _imageFow leaves those pixels unchanged.
        QRadialGradient grad(r, r, r);
        grad.setColorAt(0, QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
        grad.setColorAt(1.0 - (5.0 / static_cast<qreal>(r)), QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
        grad.setColorAt(1, _fowColor);
        sp.setBrush(grad);
        sp.setCompositionMode(QPainter::CompositionMode_Source);
        sp.drawEllipse(QPoint(r, r), r, r);
    }
    else
    {
        // Pre-render the 5-rect stepped soft edge on the small stamp image using
        // DestinationIn. The stamp's alpha channel encodes the compound multiplier
        // at each pixel. One drawImage+DestinationIn per event is cheaper than
        // 5 drawRect+DestinationIn calls on the full _imageFow image.
        sp.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        const qreal border = static_cast<qreal>(r) / 20.0;
        qreal sr = static_cast<qreal>(r) - border * 4;
        sp.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 0));
        sp.drawRect(QRectF(r - sr, r - sr, sr * 2, sr * 2));
        sr += border;
        sp.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 50));
        sp.drawRect(QRectF(r - sr, r - sr, sr * 2, sr * 2));
        sr += border;
        sp.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 100));
        sp.drawRect(QRectF(r - sr, r - sr, sr * 2, sr * 2));
        sr += border;
        sp.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 150));
        sp.drawRect(QRectF(r - sr, r - sr, sr * 2, sr * 2));
        sr += border;
        sp.setBrush(QColor(_fowColor.red(), _fowColor.green(), _fowColor.blue(), 200));
        sp.drawRect(QRectF(r - sr, r - sr, sr * 2, sr * 2));
    }
    sp.end();
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
                   (constAction->getType() == DMHelper::ActionType_Rect) ||
                   (constAction->getType() == DMHelper::ActionType_Polygon))
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
        _batchProcessing = true;
        while(_undoItems.count() > 0)
        {
            UndoFowBase* undoItem = _undoItems.takeFirst();
            if(undoItem)
            {
                undoItem->setLayer(this);
                _undoStack->push(undoItem);
            }
        }
        _batchProcessing = false;
        dispatchFowUpdate(QRect(QPoint(), _imageFow.size()));
    }
    else
    {
        applyPaintTo(getUndoStack()->index());
    }
}


