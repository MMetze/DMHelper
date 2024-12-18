#ifndef PUBLISHGLBATTLEIMAGERENDERER_H
#define PUBLISHGLBATTLEIMAGERENDERER_H

#include "publishglbattlerenderer.h"

class PublishGLBattleBackground;

class PublishGLBattleImageRenderer : public PublishGLBattleRenderer
{
    Q_OBJECT
public:
    PublishGLBattleImageRenderer(BattleDialogModel* model, QObject *parent = nullptr);
    virtual ~PublishGLBattleImageRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanupGL() override;
    virtual QSizeF getBackgroundSize() override;

protected:
    // Background overrides
    virtual void initializeBackground() override;
    virtual bool isBackgroundReady() override;
    virtual void resizeBackground(int w, int h) override;
    virtual void paintBackground(QOpenGLFunctions* functions) override;

    PublishGLBattleBackground* _backgroundObject;
    QImage _backgroundImage;
};

#endif // PUBLISHGLBATTLEIMAGERENDERER_H
