#ifndef LAYERTOKENS_H
#define LAYERTOKENS_H

#include "layer.h"

class LayerTokens : public Layer
{
    Q_OBJECT
public:
    explicit LayerTokens(const QString& name = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerTokens() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize() override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;

    // Layer Specific Interface
    virtual void initialize(const QSize& layerSize) override;
    virtual void uninitialize() override;
    virtual void setOrder(int order) override;
    virtual void setLayerVisible(bool layerVisible) override;

    // Local Interface

signals:

protected slots:
    // Local Interface

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();

    // DM Window Members

    // Player Window Members

    // Core contents

};

#endif // LAYERTOKENS_H
