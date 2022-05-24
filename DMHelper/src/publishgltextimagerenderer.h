#ifndef PUBLISHGLTEXTIMAGERENDERER_H
#define PUBLISHGLTEXTIMAGERENDERER_H

#include "publishgltextrenderer.h"

class PublishGLTextImageRenderer : public PublishGLTextRenderer
{
    Q_OBJECT
public:
    PublishGLTextImageRenderer(EncounterText* encounter, QImage backgroundImage, QImage textImage, QObject *parent = nullptr);
    virtual ~PublishGLTextImageRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;

protected:
    // Background overrides
    virtual void initializeBackground() override;
    virtual bool isBackgroundReady() override;
    virtual void resizeBackground(int w, int h) override;
    virtual void paintBackground(QOpenGLFunctions* functions) override;
    virtual QSizeF getBackgroundSize() override;

    PublishGLBattleBackground* _backgroundObject;
    QImage _backgroundImage;
};

#endif // PUBLISHGLTEXTIMAGERENDERER_H
