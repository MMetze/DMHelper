#ifndef LAYER_H
#define LAYER_H

#include "dmhobjectbase.h"
#include <QRectF>
#include <QOpenGLFunctions>
#include <QDomElement>
#include <QDir>
#include "dmconstants.h"

class LayerScene;
class QGraphicsScene;
class PublishGLRenderer;
class PublishGLScene;

/*
 *
 * Layers - central concept in the Map, where the existance, type, basic functions (order, draw, create, delete) are handled
    Each layer will have two child parts, one for the DM View (in DMH, Graphics Scene) and one for the Player View (OpenGL for rendering)
    Needs for DM Version:

    Needs for Player Version:
        init, uninit, update

    Both with static/dynamic content (eg videos)
        init, uninit, update, draw

    Types of Layers:
        Color
            --> Draw
        Grid
        Image
        Video (could this simplify things on different types of rendering)
        FoW
 *
 */

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
    virtual bool getLayerVisible() const;
    virtual qreal getOpacity() const;
    virtual QPoint getPosition() const;
    virtual QSize getSize() const;
    virtual QImage getLayerIcon() const;
    virtual bool defaultShader() const;

    virtual DMHelper::LayerType getType() const = 0;
    virtual DMHelper::LayerType getFinalType() const;
    virtual Layer* clone() const = 0;
    virtual void copyBaseValues(Layer *other) const;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) = 0;
    virtual void applyLayerVisible(bool layerVisible) = 0;
    virtual void applyOpacity(qreal opacity) = 0;
    virtual void applyPosition(const QPoint& position) = 0;
    virtual void applySize(const QSize& size) = 0;

public slots:    
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene);
    virtual void dmUninitialize() = 0;
    virtual void dmUpdate() = 0;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLScene* scene);
    virtual void playerGLUninitialize() = 0;
    virtual bool playerGLUpdate();
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) = 0;
    virtual void playerGLResize(int w, int h) = 0;
    virtual void playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA);
    virtual bool playerIsInitialized() = 0;

    // Layer Specific Interface
    virtual void initialize(const QSize& layerSize) = 0;
    virtual void uninitialize() = 0;
    virtual void setScale(int scale);
    virtual void setName(const QString& name);

    // Local Layer Interface (changing references doesn't change the source layer)
    virtual void setLayerScene(LayerScene* layerScene);
    virtual void setOrder(int order);
    virtual void setLayerVisible(bool layerVisible);
    virtual void setOpacity(qreal opacity);
    virtual void setPosition(const QPoint& position);
    virtual void setPosition(int x, int y);
    virtual void setSize(const QSize& size);
    virtual void setSize(int w, int h);

signals:
    void dirty();
    void orderChanged(int order);
    void layerDestroyed(Layer* layer);

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    LayerScene* _layerScene;
    QString _name;
    int _order;
    bool _layerVisible;
    qreal _opacity;
    QPoint _position;
    QSize _size;

    unsigned int _shaderProgramRGB;
    unsigned int _shaderProgramRGBA;
    int _shaderModelMatrixRGBA;
    int _shaderProjectionMatrixRGBA;
    int _shaderAlphaRGBA;
};

#endif // LAYER_H
