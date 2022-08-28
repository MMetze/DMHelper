#include "layerfow.h"
#include "publishglbattlebackground.h"
#include "undofowbase.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QOpenGLFunctions>
#include <QImage>
#include <QUndoStack>
#include <QPainter>
#include <QDebug>

LayerFow::LayerFow(const QSize& imageSize, int order, QObject *parent) :
    Layer{order, parent},
    _graphicsItem(nullptr),
    _backgroundObject(nullptr),
    _imgFow(imageSize, QImage::Format_ARGB32),
    _undoStack(nullptr)
{
    if(imageSize.isEmpty())
        qDebug() << "[LayerFow] ERROR: layer fow created with an empty size!";

    _undoStack = new QUndoStack(this);
    _imgFow.fill(Qt::red);
}

LayerFow::~LayerFow()
{
    cleanupDM();
    cleanupPlayer();
}

QRectF LayerFow::boundingRect() const
{
    return QRectF(QPointF(0,0), _imgFow.size());
    //return _graphicsItem ? _graphicsItem->boundingRect() : QRectF();
}

DMHelper::LayerType LayerFow::getType() const
{
    return DMHelper::LayerType_Fow;
}

QImage LayerFow::getImage() const
{
    return _imgFow;
}

QPaintDevice* LayerFow::getImageTarget()
{
    return &_imgFow;
}

QUndoStack* LayerFow::getUndoStack() const
{
    return _undoStack;
}

void LayerFow::undoPaint()
{
    //_mapSource->applyPaintTo(nullptr, QColor(0,0,0,128), _mapSource->getUndoStack()->index() - 1)
    applyPaintTo(getUndoStack()->index() - 1);
}

void LayerFow::applyPaintTo(int index, int startIndex)
{
    if(index < startIndex)
        return;

    if(index > _undoStack->count())
        index = _undoStack->count();

    if(startIndex == 0)
        _imgFow.fill(Qt::black);

    for( int i = startIndex; i < index; ++i )
    {
        const UndoFowBase* action = dynamic_cast<const UndoFowBase*>(_undoStack->command(i));
        if(action)
            action->apply();
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
}

void LayerFow::fillFoW(const QColor& color)
{
    QPainter p(&_imgFow);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(0, 0, _imgFow.width(), _imgFow.height(), color);
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

void LayerFow::dmInitialize(QGraphicsScene& scene)
{
    if(_graphicsItem)
    {
        qDebug() << "[LayerFow] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    _graphicsItem = scene.addPixmap(QPixmap::fromImage(getImage()));
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
    if(_backgroundObject)
    {
        qDebug() << "[LayerImage] ERROR: playerGLInitialize called although the background object already exists!";
        return;
    }

    _backgroundObject = new PublishGLBattleBackground(nullptr, getImage(), GL_NEAREST);
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

void LayerFow::playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix)
{
    if((!_backgroundObject) || (!functions))
        return;

    functions->glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
    _backgroundObject->paintGL();
}

void LayerFow::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void LayerFow::cleanupDM()
{
    if(!_graphicsItem)
        return;

    if(_graphicsItem->scene())
        _graphicsItem->scene()->removeItem(_graphicsItem);

    delete _graphicsItem;
    _graphicsItem = nullptr;
}

void LayerFow::cleanupPlayer()
{
    delete _backgroundObject;
    _backgroundObject = nullptr;
}
