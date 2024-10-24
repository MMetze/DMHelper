#ifndef LAYER_H
#define LAYER_H

#include "dmhobjectbase.h"
#include "dmh_opengl.h"
#include <QRectF>
#include <QDomElement>
#include <QDir>
#include "dmconstants.h"

class Campaign;
class LayerScene;
class QGraphicsScene;
class PublishGLRenderer;
class PublishGLScene;

class Layer : public DMHObjectBase
{
    Q_OBJECT
public:
    Layer(const QString& name, int order = 0, QObject *parent = nullptr);
    virtual ~Layer();

    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &parentElement, QDir& targetDirectory, bool isExport) override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport);

    virtual QRectF boundingRect() const;

    virtual LayerScene* getLayerScene() const;
    virtual QString getName() const;
    virtual int getOrder() const;
    virtual bool getLayerVisibleDM() const;
    virtual bool getLayerVisiblePlayer() const;
    virtual bool getLinkedUp() const;
    virtual qreal getOpacity() const;
    virtual QPoint getPosition() const;
    virtual QSize getSize() const;
    virtual QImage getLayerIcon() const;
    virtual bool defaultShader() const;
    virtual bool hasSettings() const;
    virtual bool hasAudio() const;

    virtual DMHelper::LayerType getType() const = 0;
    virtual DMHelper::LayerType getFinalType() const;
    virtual Layer* getFinalLayer();
    virtual Layer* clone() const = 0;
    virtual void copyBaseValues(Layer *other) const;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) = 0;
    virtual void applyLayerVisibleDM(bool layerVisible) = 0;
    virtual void applyLayerVisiblePlayer(bool layerVisible) = 0;
    virtual void applyOpacity(qreal opacity) = 0;
    virtual void applyPosition(const QPoint& position) = 0;
    virtual void applySize(const QSize& size) = 0;

public slots:    
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene);
    virtual void dmUninitialize() = 0;
    virtual void dmUpdate() = 0;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene);
    virtual void playerGLUninitialize() = 0;
    virtual bool playerGLUpdate();
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) = 0;
    virtual void playerGLResize(int w, int h) = 0;
    virtual void playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA);
    virtual bool playerIsInitialized() = 0;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) = 0;
    virtual void uninitialize() = 0;
    virtual void aboutToDelete();
    virtual void setScale(int scale);
    virtual void setName(const QString& name);
    virtual void editSettings();

    // Local Layer Interface (changing references doesn't change the source layer)
    virtual void setLayerScene(LayerScene* layerScene);
    virtual void setOrder(int order);
    virtual void setLayerVisibleDM(bool layerVisible);
    virtual void setLayerVisiblePlayer(bool layerVisible);
    virtual void setLinkedUp(bool linkedUp);
    virtual void setOpacity(qreal opacity);
    virtual void setPosition(const QPoint& position);
    virtual void setPosition(int x, int y);
    virtual void setSize(const QSize& size);
    virtual void setSize(int w, int h);

signals:
    void dirty();
    void orderChanged(int order);
    void layerDestroyed(Layer* layer);
    void layerMoved(const QPoint& position);
    void layerResized(const QSize& size);
    void layerVisibilityChanged(Layer* layer);
    void layerScaleChanged(Layer* layer);

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    LayerScene* _layerScene;
    QString _name;
    int _order;
    bool _layerVisibleDM;
    bool _layerVisiblePlayer;
    bool _linkedUp;
    qreal _layerOpacity;
    qreal _opacityReference;
    QPoint _position;
    QSize _size;

    unsigned int _shaderProgramRGB;
    unsigned int _shaderProgramRGBA;
    int _shaderModelMatrixRGBA;
    int _shaderProjectionMatrixRGBA;
    int _shaderAlphaRGBA;
};

#endif // LAYER_H
