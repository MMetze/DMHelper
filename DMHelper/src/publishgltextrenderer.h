#ifndef PUBLISHGLTEXTRENDERER_H
#define PUBLISHGLTEXTRENDERER_H

#include "publishglrenderer.h"
#include "publishglbattlescene.h"
#include <QImage>
#include <QElapsedTimer>

class PublishGLBattleBackground;
class PublishGLImage;
class EncounterText;

class PublishGLTextRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
//    PublishGLTextRenderer(QImage& backgroundImage, QImage& textImage, QObject *parent = nullptr);
    PublishGLTextRenderer(EncounterText* encounter, /*QImage backgroundImage,*/ QImage textImage, QObject *parent = nullptr);
    virtual ~PublishGLTextRenderer() override;

    virtual CampaignObjectBase* getObject() override;
    virtual QColor getBackgroundColor() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual void setBackgroundColor(const QColor& color) override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

public slots:
    void rewind();
    void play();
    void stop();

protected:
    // QObject overrides
    virtual void timerEvent(QTimerEvent *event) override;

    // DMH OpenGL renderer calls
    virtual void updateProjectionMatrix() override;

    // Background overrides
    virtual void initializeBackground() = 0;
    virtual bool isBackgroundReady() = 0;
    virtual void resizeBackground(int w, int h) = 0;
    virtual void paintBackground(QOpenGLFunctions* functions) = 0;
    virtual QSizeF getBackgroundSize() = 0;
    virtual void updateBackground();

    EncounterText* _encounter;
    QSize _targetSize;
    QColor _color;
    //QImage _backgroundImage;
    QImage _textImage;
    PublishGLBattleScene _scene;
    bool _initialized;
    unsigned int _shaderProgram;
    int _shaderModelMatrix;
    //PublishGLBattleBackground* _backgroundObject;
    PublishGLImage* _textObject;

    QPointF _textPos;
    QElapsedTimer _elapsed;
    int _timerId;
};

#endif // PUBLISHGLTEXTRENDERER_H
