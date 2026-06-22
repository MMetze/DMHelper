#ifndef LAYERDRAW_H
#define LAYERDRAW_H

#include "layer.h"
#include "layerdrawstate.h"
#include <QImage>
#include <QHash>
#include <QSet>

class PublishGLBattleBackground;
class QGraphicsItem;
class QUndoStack;

class LayerDraw : public Layer
{
    Q_OBJECT
public:
    explicit LayerDraw(const QString& name = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerDraw() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual bool defaultShader() const override;
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
    LayerDrawState& getDrawState();

    QGraphicsItem* createGraphicsItem(LayerDrawObject* drawObject);
    LayerDrawObject* findObjectForItem(QGraphicsItem* item) const;
    bool eraseObjectAtPosition(const QPointF& scenePos);

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

    void addObject(LayerDrawObject* drawObject);
    void removeObject(LayerDrawObject* drawObject);
    void deleteSelectedObjects();

signals:
    void contentChanged();

protected slots:
    void handleObjectAdded(LayerDrawObject* object, int index);
    void handleObjectRemoved(LayerDrawObject* object, int index);
    void handleObjectMoved(LayerDrawObject* object);

protected:
    // QObject overrides

    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();
    PublishGLBattleBackground* createGLObject(LayerDrawObject* drawObject);
    QImage renderObjectToImage(LayerDrawObject* drawObject, QRectF& sceneBounds);

    // Generic Methods

    // DM Window Members
    QHash<LayerDrawObject*, QGraphicsItem*> _graphicsItems;

    // Player Window Members
    QHash<LayerDrawObject*, PublishGLBattleBackground*> _glObjects;
    QSet<LayerDrawObject*> _dirtyObjects;
    PublishGLScene* _scene;
    bool _playerInitialized;

    // Core contents
    LayerDrawState _layerDrawState;
    QUndoStack* _undoStack;

};

#endif // LAYERDRAW_H
