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
    virtual bool hasSettings() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    QImage getImage() const;

    QUndoStack* getUndoStack() const;
    void undoPaint();
    void applyPaintTo(int index, int startIndex = 0);

    void paintFoWPoint(QPoint point, const MapDraw& mapDraw);
    void paintFoWRect(QRect rect, const MapEditShape& mapEditShape);
    void fillFoW(const QColor& color);

    QRect getFoWVisibleRect() const;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;
    virtual bool playerIsInitialized() override;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) override;
    virtual void uninitialize() override;
    virtual void editSettings() override;

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
    void fillFoWImage();

    // Generic Methods
    void initializeUndoStack();

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Player Window Members
    PublishGLBattleBackground* _fowGLObject;
    PublishGLScene* _scene;

    // Core contents
    QColor _fowColor;
    QImage _imageFow;
    QImage _imageFowTexture;
    QString _fowTextureFile;
    int _fowTextureScale;
    QUndoStack* _undoStack;
    QList<UndoFowBase*> _undoItems;

};

#endif // LAYERFOW_H
