#ifndef PUBLISHGLTEXTRENDERER_H
#define PUBLISHGLTEXTRENDERER_H

#include "publishglrenderer.h"
#include "publishglscene.h"
#include <QImage>
#include <QElapsedTimer>
#include <QMatrix4x4>

class PublishGLBattleBackground;
class PublishGLImage;
class EncounterText;
class Layer;

class PublishGLTextRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLTextRenderer(EncounterText* encounter, QImage textImage, const QSize& frameSize, QObject *parent = nullptr);
    virtual ~PublishGLTextRenderer() override;

    virtual CampaignObjectBase* getObject() override;
    virtual QColor getBackgroundColor() override;

    // DMH OpenGL renderer calls
    virtual bool deleteOnDeactivation() override;
    virtual QRect getScissorRect() override;

    virtual void setBackgroundColor(const QColor& color) override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void cleanupGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

public slots:
    // DMH OpenGL renderer calls
    virtual void updateProjectionMatrix() override;
    virtual void setRotation(int rotation) override;

    virtual void rewind();
    virtual void play();
    virtual void stop();
    virtual void playPause(bool play);

signals:
    void playPauseChanged(bool playing);

protected slots:
    void contentChanged();
    void startScrollingTimer();
    void layerAdded(Layer* layer);

protected:
    // QObject overrides
    virtual void timerEvent(QTimerEvent *event) override;

    // Background overrides
    /*
    virtual void initializeBackground() = 0;
    virtual bool isBackgroundReady() = 0;
    virtual void resizeBackground(int w, int h) = 0;
    virtual void paintBackground(QOpenGLFunctions* functions) = 0;
    virtual QSizeF getBackgroundSize() = 0;
    virtual void updateBackground();
    */

    int getRotatedHeight(int rotation);
    void recreateContent();

    virtual void createShaders();
    void destroyShaders();

    EncounterText* _encounter;
    QSize _frameSize;
    QSize _targetSize;
    QColor _color;
    QImage _textImage;
    PublishGLScene _scene;
    bool _initialized;
    unsigned int _shaderProgramRGB;
    int _shaderModelMatrixRGB;
    int _shaderProjectionMatrixRGB;
    unsigned int _shaderProgramRGBA;
    int _shaderModelMatrixRGBA;
    int _shaderProjectionMatrixRGBA;
    int _shaderAlphaRGBA;
    unsigned int _shaderProgramRGBColor;
    int _shaderModelMatrixRGBColor;
    int _shaderProjectionMatrixRGBColor;
    int _shaderRGBColor;
    // unsigned int _shaderProgram;
    // int _shaderModelMatrix;
    // int _shaderProjectionMatrix;
    QMatrix4x4 _projectionMatrix;
    QRect _scissorRect;
    PublishGLImage* _textObject;

    QPointF _textPos;
    QElapsedTimer _elapsed;
    int _timerId;

    bool _recreateContent;
};

#endif // PUBLISHGLTEXTRENDERER_H
