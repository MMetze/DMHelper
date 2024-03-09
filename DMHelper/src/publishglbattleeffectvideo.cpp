#include "publishglbattleeffectvideo.h"
#include "battledialogmodeleffectobjectvideo.h"
#include "videoplayer.h"
#include "scaledpixmap.h"
#include "layertokens.h"
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>

// Here are the various shader programs that we will use
extern const char *vertexShaderSourceBase;
extern const char *vertexShaderSourceColorize;
extern const char *vertexShaderSourceTransparentColor;
extern const char *vertexShaderSourceTransparentColorColorize;
extern const char *fragmentShaderSourceBase;
extern const char *fragmentShaderSourceRTransparent;
extern const char *fragmentShaderSourceGTransparent;
extern const char *fragmentShaderSourceBTransparent;
extern const char *fragmentShaderSourceTransparentColor;

PublishGLBattleEffectVideo::PublishGLBattleEffectVideo(PublishGLScene* scene, BattleDialogModelEffectObjectVideo* effect) :
    PublishGLBattleEffect(scene, effect),
    _videoPlayer{nullptr},
    _playerSize{},
    _shaderProgramRGBA(0),
    _shaderModelMatrixRGBA(0),
    _shaderProjectionMatrixRGBA(0),
    _shaderAlphaRGBA(0),
    _shaderTransparentColor(0),
    _shaderTransparentTolerance(0),
    _shaderColorizeColor(0)
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

void PublishGLBattleEffectVideo::cleanup()
{
    cleanupShadersGL();
    PublishGLBattleEffect::cleanup();
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

    createShadersGL();

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

void PublishGLBattleEffectVideo::paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix)
{
    Q_UNUSED(projectionMatrix);

    if((!QOpenGLContext::currentContext()) || (!functions))
        return;

    BattleDialogModelEffectObjectVideo* effectVideo = PublishGLBattleEffectVideo::getEffectVideo();
    if(!effectVideo)
        return;

    LayerTokens* tokensLayer = effectVideo->getLayer();
    if(!tokensLayer)
        return;

    if((!_videoPlayer) || (!_videoPlayer->getImage()))
        return;

    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if(!e)
        return;

    if(_recreateEffect)
    {
        _recreateEffect = false;
        cleanup();
        prepareObjectsGL();
    }
    else if((!_VAO) || (!_shaderProgramRGBA))
    {
        prepareObjectsGL();
    }

    functions->glUseProgram(_shaderProgramRGBA);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    QMatrix4x4 localMatrix = getMatrix();
    localMatrix.translate(tokensLayer->getPosition().x(), tokensLayer->getPosition().y());
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, localMatrix.constData());
    functions->glUniform1f(_shaderAlphaRGBA, getEffectAlpha() * tokensLayer->getOpacity());

    if(effectVideo->getEffectTransparencyType() == DMHelper::TransparentType_TransparentColor)
    {
        functions->glUniform3f(_shaderTransparentColor, effectVideo->getTransparentColor().redF(), effectVideo->getTransparentColor().greenF(), effectVideo->getTransparentColor().blueF());
        functions->glUniform1f(_shaderTransparentTolerance, effectVideo->getTransparentTolerance());
    }

    if(effectVideo->isColorize())
        functions->glUniform3f(_shaderColorizeColor, effectVideo->getColorizeColor().redF(), effectVideo->getColorizeColor().greenF(), effectVideo->getColorizeColor().blueF());

    e->glBindVertexArray(_VAO);
    functions->glBindTexture(GL_TEXTURE_2D, _textureID);

    if(_videoPlayer->isNewImage())
    {
        // load and generate the background texture
        QImage effectImage = _videoPlayer->getImage()->scaledToWidth(_textureSize.width(), Qt::FastTransformation).convertToFormat(QImage::Format_RGBA8888);
        functions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _textureSize.width(), _textureSize.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, effectImage.bits());
        functions->glGenerateMipmap(GL_TEXTURE_2D);
    }

    functions->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

bool PublishGLBattleEffectVideo::hasCustomShaders() const
{
    return true;
}

BattleDialogModelEffectObjectVideo* PublishGLBattleEffectVideo::getEffectVideo() const
{
    return dynamic_cast<BattleDialogModelEffectObjectVideo*>(_effect);
}

void PublishGLBattleEffectVideo::createShadersGL()
{
    // Create the local shader program
    if((!QOpenGLContext::currentContext()) || (_shaderProgramRGBA != 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    BattleDialogModelEffectObjectVideo* effectVideo = PublishGLBattleEffectVideo::getEffectVideo();
    if(!effectVideo)
        return;

    int  success;
    char infoLog[512];

    const char * vertexSource = getVertexShaderSource(effectVideo);
    unsigned int vertexShaderRGBA;
    vertexShaderRGBA = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShaderRGBA, 1, &vertexSource, NULL);
    f->glCompileShader(vertexShaderRGBA);

    f->glGetShaderiv(vertexShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerVideoEffect] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char * fragmentSource = getFragmentShaderSource(effectVideo);
    unsigned int fragmentShaderRGBA;
    fragmentShaderRGBA = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGBA, 1, &fragmentSource, NULL);
    f->glCompileShader(fragmentShaderRGBA);

    f->glGetShaderiv(fragmentShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerVideoEffect] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGBA = f->glCreateProgram();

    f->glAttachShader(_shaderProgramRGBA, vertexShaderRGBA);
    f->glAttachShader(_shaderProgramRGBA, fragmentShaderRGBA);
    f->glLinkProgram(_shaderProgramRGBA);

    f->glGetProgramiv(_shaderProgramRGBA, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgramRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerVideoEffect] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgramRGBA);
    f->glDeleteShader(vertexShaderRGBA);
    f->glDeleteShader(fragmentShaderRGBA);
    _shaderModelMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "model");
    _shaderProjectionMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "projection");
    _shaderAlphaRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "alpha");

    if(effectVideo->getEffectTransparencyType() == DMHelper::TransparentType_TransparentColor)
    {
        _shaderTransparentColor = f->glGetUniformLocation(_shaderProgramRGBA, "transparentColor");
        _shaderTransparentTolerance = f->glGetUniformLocation(_shaderProgramRGBA, "transparentTolerance");
    }

    if(effectVideo->isColorize())
        _shaderColorizeColor = f->glGetUniformLocation(_shaderProgramRGBA, "colorizeColor");

    QMatrix4x4 modelMatrix;
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), 0); // set it manually
    f->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBA, "view"), 1, GL_FALSE, viewMatrix.constData());
}

void PublishGLBattleEffectVideo::cleanupShadersGL()
{
    if((!QOpenGLContext::currentContext()) || (_shaderProgramRGBA == 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(f)
        f->glDeleteProgram(_shaderProgramRGBA);

    _shaderProgramRGBA = 0;
    _shaderModelMatrixRGBA = 0;
    _shaderProjectionMatrixRGBA = 0;
    _shaderAlphaRGBA = 0;
    _shaderTransparentColor = 0;
    _shaderTransparentTolerance = 0;
    _shaderColorizeColor = 0;
}

const char* PublishGLBattleEffectVideo::getVertexShaderSource(BattleDialogModelEffectObjectVideo* effectVideo)
{
    if(effectVideo)
    {
        if(effectVideo->isColorize())
        {
            if(effectVideo->getEffectTransparencyType() == DMHelper::TransparentType_TransparentColor)
                return vertexShaderSourceTransparentColorColorize;
            else
                return vertexShaderSourceColorize;
        }

        if(effectVideo->getEffectTransparencyType() == DMHelper::TransparentType_TransparentColor)
            return vertexShaderSourceTransparentColor;
    }

    return vertexShaderSourceBase;
}

const char* PublishGLBattleEffectVideo::getFragmentShaderSource(BattleDialogModelEffectObjectVideo* effectVideo)
{
    if(effectVideo)
    {
        switch(effectVideo->getEffectTransparencyType())
        {
        case DMHelper::TransparentType_RedChannel:
            return fragmentShaderSourceRTransparent;
        case DMHelper::TransparentType_GreenChannel:
            return fragmentShaderSourceGTransparent;
        case DMHelper::TransparentType_BlueChannel:
            return fragmentShaderSourceBTransparent;
        case DMHelper::TransparentType_TransparentColor:
            return fragmentShaderSourceTransparentColor;
        default:
            break;
        }
    }

    return fragmentShaderSourceBase;
}
