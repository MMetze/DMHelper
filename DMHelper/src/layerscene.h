#ifndef LAYERSCENE_H
#define LAYERSCENE_H

#include <QList>
#include <QOpenGLFunctions>
#include "dmconstants.h"
#include "campaignobjectbase.h"

class Layer;
class QGraphicsScene;

class LayerScene : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit LayerScene(QObject *parent = nullptr);
    virtual ~LayerScene();

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    // Local
    virtual QRectF boundingRect() const;
    virtual QSizeF sceneSize() const;

    int getScale() const;
    void setScale(int scale);

    int layerCount() const;
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

    Layer* getPriority(DMHelper::LayerType type) const;
    Layer* getFirst(DMHelper::LayerType type) const;
    QImage mergedImage();

public slots:
    // Local
    void initializeLayers();
    void uninitializeLayers();

    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene);
    virtual void dmUninitialize();
    virtual void dmUpdate();

    // Player Window Generic Interface
    virtual void playerGLInitialize();
    virtual void playerGLUninitialize();
    virtual bool playerGLUpdate();
    virtual void playerGLPaint(QOpenGLFunctions* functions, unsigned int shaderProgram, GLint defaultModelMatrix, const GLfloat* projectionMatrix);
    virtual void playerGLResize(int w, int h);

signals:

protected slots:
    void removeLayer(Layer* reference);

protected:
    // From CampaignObjectBase
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

    // Local
    void resetLayerOrders();

    bool _initialized;
    QList<Layer*> _layers;
    int _scale;
    int _selected;
    QGraphicsScene* _dmScene;
};

#endif // LAYERSCENE_H
