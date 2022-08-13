#ifndef LAYERIMAGE_H
#define LAYERIMAGE_H

#include "layer.h"

class QGraphicsPixmapItem;

class LayerImage : public Layer
{
    Q_OBJECT
public:
    explicit LayerImage(QObject *parent = nullptr);
    virtual ~LayerImage() override;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize() override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize() override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLUpdate() override;
    virtual void playerGLPaint() override;

protected:
    // DM Window Members
    QGraphicsPixmapItem* _backgroundImage;

};

#endif // LAYERIMAGE_H
