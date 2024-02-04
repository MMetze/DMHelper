#include "publishglbattleeffectvideo.h"
#include "battledialogmodeleffectobjectvideo.h"
#include "videoplayer.h"
#include "scaledpixmap.h"
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>

PublishGLBattleEffectVideo::PublishGLBattleEffectVideo(PublishGLScene* scene, BattleDialogModelEffectObjectVideo* effect) :
    PublishGLBattleEffect(scene, effect),
    _videoPlayer{nullptr},
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
    if(_videoPlayer)
    {
        disconnect(_videoPlayer, nullptr, nullptr, nullptr);
        VideoPlayer* deletePlayer = _videoPlayer;
        _videoPlayer = nullptr;
        deletePlayer->stopThenDelete();
    }
}

void PublishGLBattleEffectVideo::prepareObjectsGL()
{
    if(!QOpenGLContext::currentContext())
        return;

    if((!_effect) || (!_videoPlayer) || (!_videoPlayer->getImage()))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    int effectSize = DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] * _effect->getSize() / 5; // Primary dimension
    int effectWidth = DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] * _effect->getWidth() / 5; // Secondary dimension

    QImage effectImage = _videoPlayer->getImage()->scaledToWidth(effectSize, Qt::FastTransformation).convertToFormat(QImage::Format_RGBA8888);
    _textureSize = effectImage.size();

    float vertices[] = {
        // positions (mirrored vertically)                                               // colors           // texture coords
        (float)_textureSize.width() / 2.f,   (float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // top right
        (float)_textureSize.width() / 2.f,  -(float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   // bottom right
        -(float)_textureSize.width() / 2.f, -(float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   // bottom left
        -(float)_textureSize.width() / 2.f,  (float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f    // top left
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);
    f->glGenBuffers(1, &_EBO);

    e->glBindVertexArray(_VAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // color attribute
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // texture attribute
    f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    f->glEnableVertexAttribArray(2);

    // Texture
    f->glGenTextures(1, &_textureID);
    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load and generate the background texture
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _textureSize.width(), _textureSize.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, effectImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);

    PublishGLBattleEffect::effectMoved();
}

void PublishGLBattleEffectVideo::paintGL()
{
    if(!QOpenGLContext::currentContext())
        return;

    if((!_videoPlayer) || (!_videoPlayer->getImage()))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    if(_recreateEffect)
    {
        _recreateEffect = false;
        cleanup();
        prepareObjectsGL();
    }
    else if(!_VAO)
    {
        prepareObjectsGL();
    }

    e->glBindVertexArray(_VAO);
    f->glBindTexture(GL_TEXTURE_2D, _textureID);

    if(_videoPlayer->isNewImage())
    {
        // load and generate the background texture
        QImage effectImage = _videoPlayer->getImage()->scaledToWidth(_textureSize.width(), Qt::FastTransformation).convertToFormat(QImage::Format_RGBA8888);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _textureSize.width(), _textureSize.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, effectImage.bits());
        f->glGenerateMipmap(GL_TEXTURE_2D);
    }

    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

BattleDialogModelEffectObjectVideo* PublishGLBattleEffectVideo::getEffectVideo() const
{
    return dynamic_cast<BattleDialogModelEffectObjectVideo*>(_effect);
}
