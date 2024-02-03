#include "publishglbattleeffectvideo.h"
#include "battledialogmodeleffectobjectvideo.h"
#include "publishglbattlebackground.h"
#include "videoplayer.h"
#include <QOpenGLFunctions>

PublishGLBattleEffectVideo::PublishGLBattleEffectVideo(PublishGLScene* scene, BattleDialogModelEffectObjectVideo* effect) :
    PublishGLBattleEffect(scene, effect),
    _videoPlayer{nullptr},
    _videoObject{nullptr},
    _playerSize{}
{
    if(effect)
    {
        _videoPlayer = new VideoPlayer(effect->getImageFile(), QSize(), true, false);
        connect(_videoPlayer, &VideoPlayer::frameAvailable, this, &PublishGLBattleEffectVideo::updateWidget);
        _videoPlayer->restartPlayer();
    }
}

PublishGLBattleEffectVideo::~PublishGLBattleEffectVideo()
{
    PublishGLBattleEffectVideo::cleanup();
}

void PublishGLBattleEffectVideo::cleanup()
{
    if(_videoPlayer)
    {
        disconnect(_videoPlayer, nullptr, nullptr, nullptr);
        VideoPlayer* deletePlayer = _videoPlayer;
        _videoPlayer = nullptr;
        deletePlayer->stopThenDelete();
    }

    delete _videoObject;
    _videoObject = nullptr;
}

void PublishGLBattleEffectVideo::paintGL()
{
    if((!_videoPlayer) || (!_videoPlayer->getImage()))
        return;

    if(!_videoObject)
    {
        if(!_videoPlayer->isNewImage())
            return;

        _videoObject = new PublishGLBattleBackground(nullptr, *(_videoPlayer->getImage()), GL_NEAREST);
        //QPoint pointTopLeft = _scene ? _scene->getSceneRect().toRect().topLeft() : QPoint();
        //_videoObject->setPosition(QPoint(pointTopLeft.x() + _position.x(), -pointTopLeft.y() - _position.y()));
        _videoObject->setTargetSize(_videoObject->getSize());
    }
    else if(_videoPlayer->isNewImage())
    {
        _videoObject->updateImage(*(_videoPlayer->getImage()));
    }

    _videoObject->paintGL();

    PublishGLBattleEffect::paintGL();
}

BattleDialogModelEffectObjectVideo* PublishGLBattleEffectVideo::getEffectVideo() const
{
    return dynamic_cast<BattleDialogModelEffectObjectVideo*>(_effect);
}
