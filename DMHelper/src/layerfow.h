#ifndef LAYERFOW_H
#define LAYERFOW_H

#include "layer.h"
#include "mapcontent.h"
#include <QImage>
#include <QPixmap>

class PublishGLBattleBackground;
class QGraphicsPixmapItem;
class QUndoStack;
class UndoFowBase;

class LayerFow : public Layer
{
    Q_OBJECT
public:
    explicit LayerFow(const QString& name = QString(), const QSize& imageSize = QSize(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerFow() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisible(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;

    QImage getImage() const;

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

    QSize getImageSize() const;
    void setImageSize(const QSize& imageSize);


public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
//    virtual bool playerGLUpdate() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;

    // Layer Specific Interface
    virtual void initialize(const QSize& layerSize) override;
    virtual void uninitialize() override;

protected slots:
    // Local Interface
    void updateFowInternal();

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    void challengeUndoStack();

    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();

    // Generic Methods
    void initializeUndoStack();

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Player Window Members
    PublishGLBattleBackground* _backgroundObject;

    // Core contents
    QSize _imageSize;
    QImage _imgFow;
    //QPixmap _pixmapFow;
    QUndoStack* _undoStack;
    QList<UndoFowBase*> _undoItems;

};

#endif // LAYERFOW_H
