#ifndef LAYERFOW_H
#define LAYERFOW_H

#include "layer.h"
#include "mapcontent.h"
#include <QImage>

class PublishGLBattleBackground;
class QGraphicsPixmapItem;
class QUndoStack;

class LayerFow : public Layer
{
    Q_OBJECT
public:
    explicit LayerFow(const QSize& imageSize, int order = 0, QObject *parent = nullptr);
    virtual ~LayerFow() override;

    virtual QRectF boundingRect() const override;
    virtual DMHelper::LayerType getType() const override;

    QImage getImage() const;
    QPaintDevice* getImageTarget();

    QUndoStack* getUndoStack() const;
    void undoPaint();
    void applyPaintTo(int index, int startIndex = 0);
    //void applyPaintTo(QImage* target, const QColor& clearColor, int index, bool preview = false, int startIndex = 0);
    //void internalApplyPaintTo(QImage* target, const QColor& clearColor, int index, bool preview = false, int startIndex = 0);

    void paintFoWPoint(QPoint point, const MapDraw& mapDraw);
    void paintFoWRect(QRect rect, const MapEditShape& mapEditShape);
    void fillFoW(const QColor& color);
    //void paintFoWPoint(QPoint point, const MapDraw& mapDraw, QPaintDevice* target, bool preview);
    //void paintFoWRect(QRect rect, const MapEditShape& mapEditShape, QPaintDevice* target, bool preview);
    //void fillFoW(const QColor& color, QPaintDevice* target);
    /*
    QImage getBWFoWImage();
    QImage getBWFoWImage(const QImage &img);
    QImage getBWFoWImage(const QSize &size);
    */


public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize() override;
    virtual void playerGLUninitialize() override;
//    virtual bool playerGLUpdate() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix) override;
    virtual void playerGLResize(int w, int h) override;

protected:
    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Player Window Members
    PublishGLBattleBackground* _backgroundObject;

    // Core contents
    QImage _imgFow;
    QUndoStack* _undoStack;

};

#endif // LAYERFOW_H
