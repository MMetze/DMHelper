#ifndef PUBLISHGLMAPIMAGERENDERER_H
#define PUBLISHGLMAPIMAGERENDERER_H

#include "publishglmaprenderer.h"

class PublishGLMapImageRenderer : public PublishGLMapRenderer
{
    Q_OBJECT
public:
    PublishGLMapImageRenderer(Map* map, QObject *parent = nullptr);
    virtual ~PublishGLMapImageRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;

protected:
    // Background overrides
    virtual void initializeBackground() override;
    virtual void resizeBackground(int w, int h) override;
    virtual void paintBackground(QOpenGLFunctions* functions) override;
    virtual QSizeF getBackgroundSize() override;

    BattleGLBackground* _backgroundObject;
    QImage _backgroundImage;
};

#endif // PUBLISHGLMAPIMAGERENDERER_H