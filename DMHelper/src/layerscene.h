#ifndef LAYERSCENE_H
#define LAYERSCENE_H

#include <QList>
#include "dmh_opengl.h"
#include "dmconstants.h"
#include "campaignobjectbase.h"

class Layer;
class QGraphicsScene;
class PublishGLRenderer;
class PublishGLScene;

class LayerScene : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit LayerScene(QObject *parent = nullptr);
    virtual ~LayerScene();

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual bool isTreeVisible() const override;

    // Local
    virtual QGraphicsScene* getDMScene();
    virtual PublishGLScene* getPlayerGLScene();
    virtual QRectF boundingRect() const;
    virtual QSizeF sceneSize() const;

    int getScale() const;
    void setScale(int scale);

    int layerCount() const;
    int layerCount(DMHelper::LayerType type) const;
    Layer* layerAt(int position) const;
    void insertLayer(int position, Layer* layer);
    void prependLayer(Layer* layer);
    void appendLayer(Layer* layer);
    void removeLayer(int position);
    void clearLayers();
    void moveLayer(int from, int to);
    Layer* findLayer(QUuid id);

    int getSelectedLayerIndex() const;
    void setSelectedLayerIndex(int selected);
    Layer* getSelectedLayer() const;
    void setSelectedLayer(Layer* layer);

    int getLayerIndex(Layer* layer) const;
    Layer* getPriority(DMHelper::LayerType type) const;
    Layer* getFirst(DMHelper::LayerType type) const;
    Layer* getFirstVisible(DMHelper::LayerType type, bool dmVisible) const;
    Layer* getPrevious(Layer* layer, DMHelper::LayerType type) const;
    Layer* getNext(Layer* layer, DMHelper::LayerType type) const;
    Layer* getNearest(Layer* layer, DMHelper::LayerType type) const;
    QImage mergedImage();

    PublishGLRenderer* getRenderer() const;

    QList<Layer*> getLayers() const;
    QList<Layer*> getLayers(DMHelper::LayerType type) const;

public slots:
    // Local
    void initializeLayers();
    void uninitializeLayers();

    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene);
    virtual void dmUninitialize();
    virtual void dmUpdate();

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene);
    virtual void playerGLUninitialize();
    virtual bool playerGLUpdate();
    virtual void playerGLPaint(QOpenGLFunctions* functions, unsigned int shaderProgram, GLint defaultModelMatrix, const GLfloat* projectionMatrix);
    virtual void playerGLResize(int w, int h);
    virtual void playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA);

signals:
    void layerAdded(Layer* layer);
    void layerRemoved(Layer* layer);
    void sceneChanged();
    void sceneSizeChanged();
    void layerVisibilityChanged(Layer* layer);
    void layerSelected(Layer* layer);
    void layerScaleChanged(Layer* layer);

protected slots:
    void handleLayerDirty();
    void handleLayerScaleChanged(Layer* layer);
    void updateLayerScales();
    void removeLayer(Layer* reference);
    void layerMoved(const QPoint& position);
    void layerResized(const QSize& size);

    void handleReferenceDestroyed(Layer* source, Layer* reference);

protected:
    // From CampaignObjectBase
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // Local
    void connectLayer(Layer* layer);
    void disconnectLayer(Layer* layer);
    void resetLayerOrders();
    QImage getLayerImage(Layer* layer);

    bool _initialized;
    QList<Layer*> _layers;
    int _scale;
    int _selected;
    QGraphicsScene* _dmScene;
    PublishGLScene* _playerGLScene;
    PublishGLRenderer* _renderer;
};

#endif // LAYERSCENE_H
