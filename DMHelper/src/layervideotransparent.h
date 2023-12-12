#ifndef LAYERVIDEOTRANSPARENT_H
#define LAYERVIDEOTRANSPARENT_H

#include "layervideo.h"
#include <QObject>

class LayerVideoTransparent : public LayerVideo
{
    Q_OBJECT
public:
    explicit LayerVideoTransparent(const QString& name = QString(), const QString& filename = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerVideoTransparent() override;

    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

public slots:
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual void playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA) override;

protected:
    void createShaders();
};

#endif // LAYERVIDEOTRANSPARENT_H
