#ifndef LAYER_H
#define LAYER_H

#include <QObject>
#include <QRectF>
#include <QOpenGLFunctions>
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

    virtual QRectF boundingRect() const;

    virtual QString getName() const;
    virtual void setName(const QString& name);

    virtual int getOrder() const;
    virtual void setOrder(int order);

    virtual QImage getLayerIcon() const;

    virtual DMHelper::LayerType getType() const = 0;

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

signals:
    void dirty();
    void orderChanged(int order);

protected:
    QString _name;
    int _order;

};

#endif // LAYER_H
