#include "publishglbattleeffectanimated.h"
#include "battledialogmodeleffect.h"
#include "layertokens.h"
#include "scaledpixmap.h"
#include "dmh_opengl.h"
#include <QOpenGLContext>
#include <QTimerEvent>

static constexpr int ANIMATION_TIMER_INTERVAL_MS = 30;
static constexpr float MS_TO_SECONDS = 1000.0f;
static constexpr int SHADER_LOG_BUFFER_SIZE = 512;
static constexpr float CAMERA_Z_DISTANCE = 500.f;
static constexpr int EFFECT_GRID_SCALE_DIVISOR = 5;
static constexpr int RADIUS_TO_DIAMETER = 2;
static constexpr int QUAD_INDEX_COUNT = 6;
static constexpr int VERTEX_STRIDE_FLOATS = 8;
static constexpr int VERTEX_POSITION_COMPONENTS = 3;
static constexpr int VERTEX_COLOR_COMPONENTS = 3;
static constexpr int VERTEX_TEXCOORD_OFFSET = 6;

PublishGLBattleEffectAnimated::PublishGLBattleEffectAnimated(PublishGLScene* scene, BattleDialogModelEffect* effect) :
    PublishGLBattleEffect(scene, effect),
    _shaderProgram(0),
    _shaderModelMatrix(0),
    _shaderProjectionMatrix(0),
    _shaderAlpha(0),
    _shaderTime(0),
    _shaderColor(0),
    _shaderSize(0),
    _animationTimer(),
    _milliseconds(0)
{
}

PublishGLBattleEffectAnimated::~PublishGLBattleEffectAnimated()
{
    _animationTimer.stop();
    PublishGLBattleEffectAnimated::cleanup();
}

void PublishGLBattleEffectAnimated::cleanup()
{
    cleanupShadersGL();
    PublishGLBattleEffect::cleanup();
}

void PublishGLBattleEffectAnimated::prepareObjectsGL()
{
    if((!QOpenGLContext::currentContext()) || (!_effect))
        return;

    createShadersGL();
    createQuadGL();

    PublishGLBattleEffect::effectMoved();

    _animationTimer.start(ANIMATION_TIMER_INTERVAL_MS, this);
}

void PublishGLBattleEffectAnimated::paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix)
{
    Q_UNUSED(projectionMatrix);

    if((!QOpenGLContext::currentContext()) || (!functions) || (!_effect))
        return;

    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if(!e)
        return;

    LayerTokens* tokensLayer = dynamic_cast<LayerTokens*>(_effect->getLayer());
    if(!tokensLayer)
        return;

    if(_recreateEffect)
    {
        _recreateEffect = false;
        cleanup();
        prepareObjectsGL();
    }
    else if((!_VAO) || (!_shaderProgram))
    {
        prepareObjectsGL();
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgram);
    functions->glUseProgram(_shaderProgram);
    DMH_DEBUG_OPENGL_glUniformMatrix4fv4(_shaderProjectionMatrix, 1, GL_FALSE, projectionMatrix);
    functions->glUniformMatrix4fv(_shaderProjectionMatrix, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0);

    QMatrix4x4 localMatrix = getMatrix();
    localMatrix.translate(tokensLayer->getPosition().x(), tokensLayer->getPosition().y());
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, localMatrix.constData(), localMatrix);
    functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, localMatrix.constData());
    DMH_DEBUG_OPENGL_glUniform1f(_shaderAlpha, getEffectAlpha() * tokensLayer->getOpacity());
    functions->glUniform1f(_shaderAlpha, getEffectAlpha() * tokensLayer->getOpacity());

    float timeSeconds = static_cast<float>(_milliseconds) / MS_TO_SECONDS;
    functions->glUniform1f(_shaderTime, timeSeconds);

    QColor c = _effect->getColor();
    functions->glUniform4f(_shaderColor, c.redF(), c.greenF(), c.blueF(), c.alphaF());

    float effectSize = static_cast<float>(_effect->getSize());
    functions->glUniform1f(_shaderSize, effectSize);

    setEffectUniforms(functions);

    e->glBindVertexArray(_VAO);
    functions->glDrawElements(GL_TRIANGLES, QUAD_INDEX_COUNT, GL_UNSIGNED_INT, 0);
}

bool PublishGLBattleEffectAnimated::hasCustomShaders() const
{
    return true;
}

void PublishGLBattleEffectAnimated::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == _animationTimer.timerId())
    {
        _milliseconds += ANIMATION_TIMER_INTERVAL_MS;
        emit updateWidget();
    }
    else
    {
        PublishGLBattleEffect::timerEvent(event);
    }
}

void PublishGLBattleEffectAnimated::createShadersGL()
{
    if((!QOpenGLContext::currentContext()) || (_shaderProgram != 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    int  success;
    char infoLog[SHADER_LOG_BUFFER_SIZE];

    const char* vertexSource = getVertexShaderSource();
    unsigned int vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShader, 1, &vertexSource, NULL);
    f->glCompileShader(vertexShader);

    f->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShader, SHADER_LOG_BUFFER_SIZE, NULL, infoLog);
        qDebug() << "[PublishGLBattleEffectAnimated] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char* fragmentSource = getFragmentShaderSource();
    unsigned int fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    f->glCompileShader(fragmentShader);

    f->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShader, SHADER_LOG_BUFFER_SIZE, NULL, infoLog);
        qDebug() << "[PublishGLBattleEffectAnimated] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();
    DMH_DEBUG_OPENGL_glCreateProgram(_shaderProgram, "_shaderProgram");

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgram, SHADER_LOG_BUFFER_SIZE, NULL, infoLog);
        qDebug() << "[PublishGLBattleEffectAnimated] ERROR::SHADER::PROGRAM::LINK_FAILED: " << infoLog;
        return;
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgram);
    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);

    _shaderModelMatrix = f->glGetUniformLocation(_shaderProgram, "model");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, _shaderModelMatrix, "model");
    _shaderProjectionMatrix = f->glGetUniformLocation(_shaderProgram, "projection");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, _shaderProjectionMatrix, "projection");
    _shaderAlpha = f->glGetUniformLocation(_shaderProgram, "alpha");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, _shaderAlpha, "alpha");
    _shaderTime = f->glGetUniformLocation(_shaderProgram, "u_time");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, _shaderTime, "u_time");
    _shaderColor = f->glGetUniformLocation(_shaderProgram, "u_color");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, _shaderColor, "u_color");
    _shaderSize = f->glGetUniformLocation(_shaderProgram, "u_size");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, _shaderSize, "u_size");

    getEffectUniformLocations(f);

    QMatrix4x4 modelMatrix;
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, CAMERA_Z_DISTANCE), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData(), modelMatrix);
    f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, f->glGetUniformLocation(_shaderProgram, "view"), "view");
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData(), viewMatrix);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());
}

void PublishGLBattleEffectAnimated::cleanupShadersGL()
{
    if((!QOpenGLContext::currentContext()) || (_shaderProgram == 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(f)
    {
        DMH_DEBUG_OPENGL_Singleton::removeProgram(_shaderProgram);
        f->glDeleteProgram(_shaderProgram);
    }

    _shaderProgram = 0;
    _shaderModelMatrix = 0;
    _shaderProjectionMatrix = 0;
    _shaderAlpha = 0;
    _shaderTime = 0;
    _shaderColor = 0;
    _shaderSize = 0;
}

void PublishGLBattleEffectAnimated::createQuadGL()
{
    if(!_effect)
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    int effectSize = DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] * _effect->getSize() / EFFECT_GRID_SCALE_DIVISOR;
    effectSize *= RADIUS_TO_DIAMETER; // These effects use radius → diameter like radius effects

    _textureSize = QSizeF(effectSize, effectSize);

    float hw = static_cast<float>(effectSize) / 2.f;
    float hh = static_cast<float>(effectSize) / 2.f;

    float vertices[] = {
        // positions          // colors           // texture coords
         hw,  hh, 0.0f,      1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   // top right
         hw, -hh, 0.0f,      1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // bottom right
        -hw, -hh, 0.0f,      1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -hw,  hh, 0.0f,      1.0f, 1.0f, 1.0f,   0.0f, 1.0f    // top left
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
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
    f->glVertexAttribPointer(0, VERTEX_POSITION_COMPONENTS, GL_FLOAT, GL_FALSE, VERTEX_STRIDE_FLOATS * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // color attribute
    f->glVertexAttribPointer(1, VERTEX_COLOR_COMPONENTS, GL_FLOAT, GL_FALSE, VERTEX_STRIDE_FLOATS * sizeof(float), (void*)(VERTEX_POSITION_COMPONENTS * sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // texture coord attribute
    f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VERTEX_STRIDE_FLOATS * sizeof(float), (void*)(VERTEX_TEXCOORD_OFFSET * sizeof(float)));
    f->glEnableVertexAttribArray(2);
}

void PublishGLBattleEffectAnimated::getEffectUniformLocations(QOpenGLFunctions* f)
{
    Q_UNUSED(f);
}
