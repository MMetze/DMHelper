#ifndef LAYER_H
#define LAYER_H

#include <QObject>
#include <QRectF>
#include <QOpenGLFunctions>
#include <QDomElement>
#include <QDir>
#include "dmconstants.h"

class QGraphicsScene;

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

class Layer : public QObject
{
    Q_OBJECT
public:
    Layer(const QString& name, int order = 0, QObject *parent = nullptr);
    virtual ~Layer();

    virtual void outputXML(QDomDocument &doc, QDomElement &parentElement, QDir& targetDirectory, bool isExport);
    virtual void inputXML(const QDomElement &element, bool isImport);

    virtual QRectF boundingRect() const;

    virtual QString getName() const;
    virtual int getOrder() const;
    virtual bool getLayerVisible() const;
    virtual QImage getLayerIcon() const;

    virtual DMHelper::LayerType getType() const = 0;
    virtual Layer* clone() const = 0;

public slots:    
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene) = 0;
    virtual void dmUninitialize() = 0;
    virtual void dmUpdate() = 0;

    // Player Window Generic Interface
    virtual void playerGLInitialize() = 0;
    virtual void playerGLUninitialize() = 0;
    virtual bool playerGLUpdate();
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix) = 0;
    virtual void playerGLResize(int w, int h) = 0;

    // Layer Specific Interface
    virtual void initialize(const QSize& layerSize) = 0;
    virtual void uninitialize() = 0;
    virtual void setName(const QString& name);
    virtual void setOrder(int order);
    virtual void setLayerVisible(bool layerVisible);

signals:
    void dirty();
    void orderChanged(int order);

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

    QString _name;
    int _order;
    bool _layerVisible;

};

#endif // LAYER_H