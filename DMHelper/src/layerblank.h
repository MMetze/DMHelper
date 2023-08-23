#ifndef LAYERBLANK_H
#define LAYERBLANK_H

#include "layer.h"
#include <QColor>

class QGraphicsRectItem;
class PublishGLRect;

class LayerBlank : public Layer
{
    Q_OBJECT
public:
    explicit LayerBlank(const QString& name = QString(), const QColor& color = QColor(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerBlank() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;

    virtual QImage getLayerIcon() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;
    virtual bool playerIsInitialized() override;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) override;
    virtual void uninitialize() override;

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    bool createShadersGL();

    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();

    // DM Window Members
    QGraphicsRectItem* _graphicsItem;

    // Player Window Members
    PublishGLRect* _publishRect;
    PublishGLScene* _scene;

    // Core contents
    QColor _color;

    static unsigned int _shaderProgramRGBColorBlank;
    static int _shaderModelMatrixRGBColorBlank;
    static int _shaderProjectionMatrixRGBColorBlank;
    static int _shaderRGBColorBlank;
};

#endif // LAYERBLANK_H
