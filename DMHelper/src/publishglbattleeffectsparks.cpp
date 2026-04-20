#include "publishglbattleeffectsparks.h"
#include "battledialogmodeleffectsparks.h"
#include "layertokens.h"
#include "scaledpixmap.h"
#include "dmh_opengl.h"
#include <QOpenGLContext>
#include <QRandomGenerator>
#include <QtMath>

static constexpr int ANIMATION_TIMER_INTERVAL_MS = 30;
static constexpr float MS_TO_SECONDS = 1000.0f;
static constexpr int EFFECT_GRID_SCALE_DIVISOR = 5;
static constexpr int RADIUS_TO_DIAMETER = 2;
static constexpr int PARTICLE_STRIDE_FLOATS = 7;  // position(3) + velocity(3) + speedVariation(1)
static constexpr int PARTICLE_POSITION_COMPONENTS = 3;
static constexpr int PARTICLE_VELOCITY_COMPONENTS = 3;
static constexpr int PARTICLE_VELOCITY_OFFSET = 3;
static constexpr int PARTICLE_SPEED_VAR_OFFSET = 6;
static constexpr float PARTICLE_SPAWN_SPREAD = 5.0f;
static constexpr float PARTICLE_MIN_SPEED = 0.3f;
static constexpr float PARTICLE_SPEED_RANGE = 0.7f;

// Vertex shader - particle system with gravity arcs
static const char *sparksVertexShader = "#version 410 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aVelocity;\n"
    "layout (location = 2) in float aSpeedVar;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform float u_time;\n"
    "uniform float u_sparkSpeed;\n"
    "uniform float u_arcFalloff;\n"
    "uniform float u_fadeDistance;\n"
    "uniform vec2 u_windVec;\n"
    "uniform float u_glowRadius;\n"
    "out float vAlpha;\n"
    "void main()\n"
    "{\n"
    "   float speed = u_sparkSpeed * (0.5 + aSpeedVar);\n"
    "   float lifetime = 2.0;\n"
    "   float t = mod(u_time * speed, lifetime);\n"
    "   float phase = t / lifetime;\n"
    "\n"
    "   vec3 pos = aPos;\n"
    "   pos.xy += aVelocity.xy * t * 200.0;\n"
    "\n"
    "   // Wind offset\n"
    "   pos.xy += u_windVec * t * 200.0;\n"
    "\n"
    "   // Gravity arc (particles fall back down as seen from above: spread outward then slow)\n"
    "   if(u_arcFalloff > 0.5) {\n"
    "       float gravity = t * t * 50.0;\n"
    "       pos.xy += aVelocity.xy * gravity * 0.5;\n"
    "   }\n"
    "\n"
    "   gl_Position = projection * view * model * vec4(pos, 1.0);\n"
    "   gl_PointSize = max(2.0, u_glowRadius * 20.0 * (1.0 - phase));\n"
    "\n"
    "   // Fade over distance/lifetime\n"
    "   vAlpha = 1.0;\n"
    "   if(u_fadeDistance > 0.5) {\n"
    "       vAlpha = 1.0 - phase;\n"
    "   }\n"
    "}\0";

// Fragment shader - glowing point
static const char *sparksFragmentShader = "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in float vAlpha;\n"
    "uniform float alpha;\n"
    "uniform vec4 u_color;\n"
    "uniform float u_glowOpacity;\n"
    "void main() {\n"
    "   vec2 coord = gl_PointCoord - vec2(0.5);\n"
    "   float dist = length(coord) * 2.0;\n"
    "   if(dist > 1.0) discard;\n"
    "\n"
    "   // Radial glow falloff\n"
    "   float glow = 1.0 - dist;\n"
    "   glow = pow(glow, 2.0);\n"
    "\n"
    "   float finalAlpha = glow * vAlpha * u_glowOpacity * alpha;\n"
    "   FragColor = vec4(u_color.rgb, finalAlpha);\n"
    "}\n";

PublishGLBattleEffectSparks::PublishGLBattleEffectSparks(PublishGLScene* scene, BattleDialogModelEffectSparks* effect) :
    PublishGLBattleEffectAnimated(scene, effect),
    _shaderGlowRadius(0),
    _shaderGlowOpacity(0),
    _shaderArcFalloff(0),
    _shaderFadeDistance(0),
    _shaderSparkSpeed(0),
    _shaderWindVec(0),
    _particleCount(0),
    _objectsDirty(false)
{
}

PublishGLBattleEffectSparks::~PublishGLBattleEffectSparks()
{
}

void PublishGLBattleEffectSparks::cleanup()
{
    PublishGLBattleEffectAnimated::cleanup();
}

void PublishGLBattleEffectSparks::prepareObjectsGL()
{
    if((!QOpenGLContext::currentContext()) || (!_effect))
        return;

    createShadersGL();
    createParticleGeometry();

    PublishGLBattleEffect::effectMoved();

    _animationTimer.start(ANIMATION_TIMER_INTERVAL_MS, this);
}

void PublishGLBattleEffectSparks::paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix)
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

    BattleDialogModelEffectSparks* sparksEffect = dynamic_cast<BattleDialogModelEffectSparks*>(_effect);
    if(!sparksEffect)
        return;

    if(_recreateEffect || _objectsDirty || (sparksEffect->getParticleCount() != _particleCount))
    {
        _recreateEffect = false;
        _objectsDirty = false;
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

    setEffectUniforms(functions);

    functions->glEnable(GL_PROGRAM_POINT_SIZE);
    e->glBindVertexArray(_VAO);
    functions->glDrawArrays(GL_POINTS, 0, _particleCount);
    functions->glDisable(GL_PROGRAM_POINT_SIZE);
}

const char* PublishGLBattleEffectSparks::getVertexShaderSource() const
{
    return sparksVertexShader;
}

const char* PublishGLBattleEffectSparks::getFragmentShaderSource() const
{
    return sparksFragmentShader;
}

void PublishGLBattleEffectSparks::getEffectUniformLocations(QOpenGLFunctions* f)
{
    _shaderGlowRadius = f->glGetUniformLocation(_shaderProgram, "u_glowRadius");
    _shaderGlowOpacity = f->glGetUniformLocation(_shaderProgram, "u_glowOpacity");
    _shaderArcFalloff = f->glGetUniformLocation(_shaderProgram, "u_arcFalloff");
    _shaderFadeDistance = f->glGetUniformLocation(_shaderProgram, "u_fadeDistance");
    _shaderSparkSpeed = f->glGetUniformLocation(_shaderProgram, "u_sparkSpeed");
    _shaderWindVec = f->glGetUniformLocation(_shaderProgram, "u_windVec");
}

void PublishGLBattleEffectSparks::setEffectUniforms(QOpenGLFunctions* f)
{
    BattleDialogModelEffectSparks* sparksEffect = dynamic_cast<BattleDialogModelEffectSparks*>(_effect);
    if(!sparksEffect)
        return;

    f->glUniform1f(_shaderGlowRadius, static_cast<float>(sparksEffect->getGlowRadius()));
    f->glUniform1f(_shaderGlowOpacity, static_cast<float>(sparksEffect->getGlowOpacity()));
    f->glUniform1f(_shaderArcFalloff, sparksEffect->getArcFalloff() ? 1.0f : 0.0f);
    f->glUniform1f(_shaderFadeDistance, sparksEffect->getFadeDistance() ? 1.0f : 0.0f);
    f->glUniform1f(_shaderSparkSpeed, static_cast<float>(sparksEffect->getSparkSpeed()));

    qreal dir = qDegreesToRadians(sparksEffect->getWindDirection());
    qreal str = sparksEffect->getWindStrength();
    f->glUniform2f(_shaderWindVec, static_cast<float>(qCos(dir) * str), static_cast<float>(qSin(dir) * str));
}

void PublishGLBattleEffectSparks::createParticleGeometry()
{
    BattleDialogModelEffectSparks* sparksEffect = dynamic_cast<BattleDialogModelEffectSparks*>(_effect);
    if(!sparksEffect)
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    _particleCount = sparksEffect->getParticleCount();
    int effectSize = DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] * _effect->getSize() / EFFECT_GRID_SCALE_DIVISOR;
    _textureSize = QSizeF(effectSize * RADIUS_TO_DIAMETER, effectSize * RADIUS_TO_DIAMETER);

    // Per-particle data: position(3) + velocity(3) + speedVariation(1)
    QVector<float> vertexData;
    vertexData.resize(_particleCount * PARTICLE_STRIDE_FLOATS);

    QRandomGenerator* rng = QRandomGenerator::global();
    for(int i = 0; i < _particleCount; ++i)
    {
        int offset = i * PARTICLE_STRIDE_FLOATS;
        // Start position: near center with small random offset
        vertexData[offset + 0] = (rng->bounded(2.0) - 1.0) * PARTICLE_SPAWN_SPREAD;  // x
        vertexData[offset + 1] = (rng->bounded(2.0) - 1.0) * PARTICLE_SPAWN_SPREAD;  // y
        vertexData[offset + 2] = 0.0f;                                // z

        // Random velocity direction (radial outward)
        float angle = rng->bounded(static_cast<float>(2.0 * M_PI));
        float speed = PARTICLE_MIN_SPEED + rng->bounded(PARTICLE_SPEED_RANGE);
        vertexData[offset + 3] = qCos(angle) * speed;  // vx
        vertexData[offset + 4] = qSin(angle) * speed;  // vy
        vertexData[offset + 5] = 0.0f;                  // vz

        // Speed variation
        vertexData[offset + 6] = rng->bounded(1.0f);
    }

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);

    e->glBindVertexArray(_VAO);
    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.constData(), GL_STATIC_DRAW);

    // position attribute (location 0)
    f->glVertexAttribPointer(0, PARTICLE_POSITION_COMPONENTS, GL_FLOAT, GL_FALSE, PARTICLE_STRIDE_FLOATS * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // velocity attribute (location 1)
    f->glVertexAttribPointer(1, PARTICLE_VELOCITY_COMPONENTS, GL_FLOAT, GL_FALSE, PARTICLE_STRIDE_FLOATS * sizeof(float), (void*)(PARTICLE_VELOCITY_OFFSET * sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // speed variation attribute (location 2)
    f->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, PARTICLE_STRIDE_FLOATS * sizeof(float), (void*)(PARTICLE_SPEED_VAR_OFFSET * sizeof(float)));
    f->glEnableVertexAttribArray(2);
}
