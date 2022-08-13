#ifndef LAYERFOW_H
#define LAYERFOW_H

#include "layer.h"

class LayerFow : public Layer
{
    Q_OBJECT
public:
    explicit LayerFow(QObject *parent = nullptr);
    virtual ~LayerFow() override;

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

};

#endif // LAYERFOW_H
