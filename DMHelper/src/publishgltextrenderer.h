#ifndef PUBLISHGLTEXTRENDERER_H
#define PUBLISHGLTEXTRENDERER_H

#include "publishglrenderer.h"
#include "battleglscene.h"
#include <QImage>
#include <QElapsedTimer>

class BattleGLBackground;

class PublishGLTextRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLTextRenderer(QImage& backgroundImage, QImage& textImage, QObject *parent = nullptr);
    virtual ~PublishGLTextRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

public slots:
    void rewind();

protected:
    virtual void timerEvent(QTimerEvent *event) override;
    void setOrthoProjection();

    QImage _backgroundImage;
    QImage _textImage;
    BattleGLScene _scene;
    bool _initialized;
    unsigned int _shaderProgram;
    BattleGLBackground* _backgroundObject;
    BattleGLBackground* _textObject;

    QPointF _textPos;
    QElapsedTimer _elapsed;
    int _timerId;
};

#endif // PUBLISHGLTEXTRENDERER_H
