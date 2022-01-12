#ifndef PUBLISHGLBATTLEIMAGERENDERER_H
#define PUBLISHGLBATTLEIMAGERENDERER_H

#include "publishglbattlerenderer.h"

class BattleGLBackground;

class PublishGLBattleImageRenderer : public PublishGLBattleRenderer
{
    Q_OBJECT
public:
    PublishGLBattleImageRenderer(BattleDialogModel* model, QObject *parent = nullptr);

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

#endif // PUBLISHGLBATTLEIMAGERENDERER_H
