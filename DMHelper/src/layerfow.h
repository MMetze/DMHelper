#ifndef LAYERFOW_H
#define LAYERFOW_H

#include "layer.h"

class LayerFow : public Layer
{
    Q_OBJECT
public:
    explicit LayerFow(int order = 0, QObject *parent = nullptr);
    virtual ~LayerFow() override;

    virtual QRectF boundingRect() const override;
    virtual DMHelper::LayerType getType() const override;

    QImage getImage() const;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize() override;
    virtual void playerGLUninitialize() override;
//    virtual bool playerGLUpdate() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix) override;
    virtual void playerGLResize(int w, int h) override;

};

#endif // LAYERFOW_H
