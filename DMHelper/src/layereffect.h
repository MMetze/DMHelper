#ifndef LAYEREFFECT_H
#define LAYEREFFECT_H

#include "layer.h"
#include <QColor>

class QGraphicsPixmapItem;

class LayerEffect : public Layer
{
    Q_OBJECT
public:
    explicit LayerEffect(const QString& name = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerEffect() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual bool defaultShader() const override;
    virtual bool hasSettings() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

signals:
    void update();

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
    virtual void playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA) override;
    virtual bool playerIsInitialized() override;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) override;
    virtual void uninitialize() override;
    virtual void editSettings() override;

    void setEffectWidth(int width);
    void setEffectHeight(int height);
    void setEffectThickness(int thickness);
    void setEffectDirection(int direction);
    void setEffectSpeed(int speed);
    void setEffectMorph(int morph);
    void setEffectColor(const QColor& color);

protected:
    // QObject overrides
    virtual void timerEvent(QTimerEvent *event) override;

    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    void velocityChanged();

    // DM Window Methods
    void cleanupDM();

    // Player Window Methods
    void cleanupPlayer();

    void createShaders();
    void destroyShaders();
    void createObjects();
    void destroyObjects();

    // Generic Methods

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Player Window Members
    PublishGLScene* _scene;
    int _shaderFragmentResolution;
    int _shaderFragmentTime;
    int _shaderFragmentWidth;
    int _shaderFragmentHeight;
    int _shaderFragmentThickness;
    int _shaderFragmentVelocity;
    int _shaderFragmentColor;

    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;

    int _timerId;
    int _milliseconds;

    // Core contents
    int _effectWidth;
    int _effectHeight;
    int _effectThickness;
    int _effectDirection;
    int _effectSpeed;
    int _effectMorph;
    QColor _effectColor;

    qreal _xVelocity;
    qreal _yVelocity;
    qreal _morphVelocity;
};

#endif // LAYEREFFECT_H
