#ifndef LAYERREFERENCE_H
#define LAYERREFERENCE_H

#include "layer.h"

class Campaign;
class CampaignObjectBase;

class LayerReference : public Layer
{
    Q_OBJECT
public:
    explicit LayerReference(CampaignObjectBase* referenceObject = nullptr, Layer* referenceLayer = nullptr, int order = 0, QObject *parent = nullptr);
    virtual ~LayerReference();

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;

    virtual QString getName() const override;
    virtual QImage getLayerIcon() const override;
    virtual bool defaultShader() const override;

    virtual DMHelper::LayerType getType() const override;
    virtual DMHelper::LayerType getFinalType() const override;
    virtual Layer* getFinalLayer() override;
    virtual Layer* clone() const override;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    DMHelper::LayerType getReferencedType() const;
    Layer* getReferenceLayer() const;
    CampaignObjectBase* getReferenceObject() const;
    virtual void clearReference();

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual bool playerGLUpdate() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;
    virtual void playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA) override;
    virtual bool playerIsInitialized() override;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) override;
    virtual void uninitialize() override;
    virtual void aboutToDelete() override;
    virtual void setScale(int scale) override;

signals:
    void referenceDestroyed(Layer* layer, Layer* reference);

protected slots:
    void handleReferenceDestroyed(Layer *layer);

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    void setReferenceLayer(Layer* layer);
    void copyReferenceValues();

    CampaignObjectBase* _referenceObject;
    QUuid _referenceObjectId;
    Layer* _referenceLayer;
    QUuid _referenceLayerId;
};

#endif // LAYERREFERENCE_H
