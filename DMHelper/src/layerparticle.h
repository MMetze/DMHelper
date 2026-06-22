#ifndef LAYERPARTICLE_H
#define LAYERPARTICLE_H

#include "layer.h"
#include <QColor>

class QGraphicsPixmapItem;

class LayerParticle : public Layer
{
    Q_OBJECT
public:
    explicit LayerParticle(const QString& name = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerParticle() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual bool defaultShader() const override;
    virtual bool hasSettings() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    static const int defaultParticleCount  = 500;
    static const int defaultRainSpeed      = 250;
    static const int defaultRainDirection  = 0;
    static const int defaultRainAngle      = 90;
    static const int defaultRainLength     = 10;
    static const int defaultRainOpacity    = 50;
    static const int defaultRainWidth      = 1;
    static const int defaultRainMovement     = 0;

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

    void setParticleCount(int count);
    void setRainSpeed(int speed);
    void setRainDirection(int direction);
    void setRainAngle(int angle);
    void setRainColor(const QColor& color);
    void setRainLength(int length);
    void setRainOpacity(int opacity);
    void setRainWidth(int width);
    void setRainMovement(int movement);

protected:
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    void cleanupDM();
    void cleanupPlayer();

    void createShaders();
    void destroyShaders();
    void createObjects();
    void destroyObjects();

    void refreshDMPreview();
    QImage createRainPreview(const QSize& size) const;

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Player Window Members
    PublishGLScene* _scene;
    int _timerId;
    int _milliseconds;
    bool _objectsDirty;

    unsigned int _VAO;
    unsigned int _VBO;

    int _shaderTime;
    int _shaderSpeed;
    int _shaderLength;
    int _shaderColor;
    int _shaderOpacity;
    int _shaderMVP;
    int _shaderMovement;
    int _vertexCount;

    // Rain parameters
    int     _particleCount;
    int     _rainSpeed;
    int     _rainDirection;
    int     _rainAngle;
    QColor  _rainColor;
    int     _rainLength;
    int     _rainOpacity;
    int     _rainWidth;
    int     _rainMovement;
};

#endif // LAYERPARTICLE_H
