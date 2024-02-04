#ifndef PUBLISHGLBATTLEEFFECTVIDEO_H
#define PUBLISHGLBATTLEEFFECTVIDEO_H

#include "publishglbattleeffect.h"

class BattleDialogModelEffectObjectVideo;
class VideoPlayer;
//class PublishGLBattleBackground;

class PublishGLBattleEffectVideo : public PublishGLBattleEffect
{
    Q_OBJECT
public:
    PublishGLBattleEffectVideo(PublishGLScene* scene, BattleDialogModelEffectObjectVideo* effect);
    virtual ~PublishGLBattleEffectVideo() override;

    virtual void prepareObjectsGL() override;
    virtual void paintGL() override;

    virtual BattleDialogModelEffectObjectVideo* getEffectVideo() const;

signals:
    void updateWidget();

protected:

    VideoPlayer* _videoPlayer;
    QSize _playerSize;
};

#endif // PUBLISHGLBATTLEEFFECTVIDEO_H
