#include "publishglbattleeffectsparks.h"
#include "battledialogmodeleffectsparks.h"
#include "dmh_opengl.h"
#include <QtMath>

// Vertex shader - standard textured quad
static const char *sparksVertexShader = "#version 410 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";

// Fragment shader - procedural sparks on a quad (robust fallback vs point sprites)
static const char *sparksFragmentShader = "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform float alpha;\n"
    "uniform float u_time;\n"
    "uniform vec4 u_color;\n"
    "uniform float u_size;\n"
    "uniform float u_glowRadius;\n"
    "uniform float u_glowOpacity;\n"
    "uniform float u_arcFalloff;\n"
    "uniform float u_fadeDistance;\n"
    "uniform float u_sparkSpeed;\n"
    "uniform vec2 u_windVec;\n"
    "uniform float u_particleCount;\n"
    "\n"
    "float hash11(float p) {\n"
    "    return fract(sin(p * 127.1) * 43758.5453123);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec2 uv = (TexCoord - 0.5) * 2.0;\n"
    "    float distFromCenter = length(uv);\n"
    "    // Loose cull so individual sparks can clearly travel past the nominal radius.\n"
    "    if(distFromCenter > 1.45) discard;\n"
    "\n"
    "    const int MAX_SPARKS = 64;\n"
    "    float sparkCount = clamp(u_particleCount, 8.0, float(MAX_SPARKS));\n"
    "    float speed = max(u_sparkSpeed, 0.05);\n"
    "    float sparkSize = 0.012 + u_glowRadius * 0.085;\n"
    "    // Forge preset uses arcFalloff; lift the cap so sparks fling outside the nominal radius.\n"
    "    float radialCap = (u_arcFalloff > 0.5) ? 1.10 : 0.90;\n"
    "\n"
    "    float accum = 0.0;\n"
    "\n"
    "    for(int i = 0; i < MAX_SPARKS; ++i) {\n"
    "        if(float(i) >= sparkCount)\n"
    "            break;\n"
    "\n"
    "        float fi = float(i) + 1.0;\n"
    "        float seedA = hash11(fi * 3.17);\n"
    "        float seedB = hash11(fi * 7.13);\n"
    "        float seedC = hash11(fi * 11.91);\n"
    "\n"
    "        float phase = fract(seedC + u_time * (0.20 + speed * (0.55 + seedA)));\n"
    "        float angle = seedA * 6.2831853;\n"
    "        vec2 dir = vec2(cos(angle), sin(angle));\n"
    "        float spawnRadius = 0.015 + seedB * 0.04;\n"
    "        float radial = phase;\n"
    "\n"
    "        if(u_arcFalloff > 0.5)\n"
    "            radial = radial * radial;\n"
    "\n"
    "        float ejectPhase = radial;\n"
    "        float skidPhase = smoothstep(0.42, 1.0, radial);\n"
    "        if(u_arcFalloff > 0.5)\n"
    "            ejectPhase = radial * radial;\n"
    "\n"
    "        float burstTravel = mix(spawnRadius, radialCap * 0.95, ejectPhase);\n"
    "        float skidTravel = skidPhase * skidPhase * (0.10 + speed * 0.30) * (0.55 + seedC * 0.85);\n"
    "        vec2 sparkPos = dir * (burstTravel + skidTravel);\n"
    "\n"
    "        // Wind bends particle motion without moving the emitter itself.\n"
    "        float windAmount = radial * (0.35 + seedB * 0.55);\n"
    "        sparkPos += u_windVec * windAmount;\n"
    "\n"
    "        // In top-down view, forge-style sparks should skid on the ground plane, not fall toward screen-bottom.\n"
    "        vec2 tangent = vec2(-dir.y, dir.x);\n"
    "        float scatter = (seedB - 0.5) * (0.02 + speed * 0.05);\n"
    "        sparkPos += tangent * scatter * (1.0 - skidPhase * 0.65);\n"
    "\n"
    "        // Pseudo-perspective arc: forge sparks fly off a tilted anvil. Squash Y to imply a low camera tilt,\n"
    "        // then add a parabolic hop along the spark's flight direction so each particle traces an arc.\n"
    "        if(u_arcFalloff > 0.5) {\n"
    "            // Foreshorten Y to fake a ~30 degree pitch.\n"
    "            sparkPos.y *= 0.62;\n"
    "            // Parabolic hop: peaks mid-flight, lands at end. Sign biases sparks 'upward' on screen (toward -Y).\n"
    "            float hop = 4.0 * radial * (1.0 - radial); // 0..1, peaks at radial=0.5\n"
    "            float hopHeight = (0.18 + speed * 0.30) * (0.65 + seedA * 0.70);\n"
    "            // Apex offset is screen-up plus a touch along the launch direction so the arc reads visually.\n"
    "            sparkPos += vec2(dir.x * hop * hopHeight * 0.25, -hop * hopHeight);\n"
    "        }\n"
    "\n"
    "        float d = length(uv - sparkPos);\n"
    "        float core = exp(-d * d / max(0.0005, sparkSize * sparkSize));\n"
    "\n"
    "        float life = 1.0 - radial;\n"
    "        life *= 1.0 - skidPhase * 0.25;\n"
    "        if(u_fadeDistance <= 0.5)\n"
    "            life = 0.6 + 0.4 * life;\n"
    "\n"
    "        // Forge sparks dim slightly as they descend past the arc apex.\n"
    "        if(u_arcFalloff > 0.5) {\n"
    "            float arcDescend = 1.0 - smoothstep(0.55, 1.0, radial) * 0.35;\n"
    "            life *= arcDescend;\n"
    "        }\n"
    "\n"
    "        accum += core * life;\n"
    "    }\n"
    "\n"
    "    // Keep center subtly hotter to read as a spark cluster at distance\n"
    "    accum += exp(-distFromCenter * distFromCenter / 0.2) * 0.2;\n"
    "\n"
    "    float edgeFade = 1.0 - smoothstep(1.10, 1.45, distFromCenter);\n"
    "    float finalAlpha = clamp(accum * edgeFade * u_glowOpacity * u_color.a * alpha, 0.0, 1.0);\n"
    "    if(finalAlpha < 0.01) discard;\n"
    "\n"
    "    vec3 finalColor = u_color.rgb * (0.7 + clamp(accum, 0.0, 1.0) * 0.9);\n"
    "    FragColor = vec4(finalColor, finalAlpha);\n"
    "}\n";

PublishGLBattleEffectSparks::PublishGLBattleEffectSparks(PublishGLScene* scene, BattleDialogModelEffectSparks* effect) :
    PublishGLBattleEffectAnimated(scene, effect),
    _shaderGlowRadius(0),
    _shaderGlowOpacity(0),
    _shaderArcFalloff(0),
    _shaderFadeDistance(0),
    _shaderSparkSpeed(0),
    _shaderWindVec(0),
    _shaderParticleCount(0),
    _particleCount(0),
    _objectsDirty(false)
{
}

PublishGLBattleEffectSparks::~PublishGLBattleEffectSparks()
{
}

qreal PublishGLBattleEffectSparks::getExtentMultiplier() const
{
    static constexpr qreal SPARKS_EXTENT_MULTIPLIER = 2.10;
    return SPARKS_EXTENT_MULTIPLIER;
}

void PublishGLBattleEffectSparks::cleanup()
{
    PublishGLBattleEffectAnimated::cleanup();
}

void PublishGLBattleEffectSparks::prepareObjectsGL()
{
    PublishGLBattleEffectAnimated::prepareObjectsGL();
}

void PublishGLBattleEffectSparks::paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix)
{
    if(!functions)
        return;

    functions->glEnable(GL_BLEND);
    functions->glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    PublishGLBattleEffectAnimated::paintGL(functions, projectionMatrix);

    functions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    _shaderParticleCount = f->glGetUniformLocation(_shaderProgram, "u_particleCount");
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
    f->glUniform1f(_shaderParticleCount, static_cast<float>(sparksEffect->getParticleCount()));

    qreal dir = qDegreesToRadians(sparksEffect->getWindDirection());
    qreal str = sparksEffect->getWindStrength();
    f->glUniform2f(_shaderWindVec, static_cast<float>(qCos(dir) * str), static_cast<float>(qSin(dir) * str));
}

void PublishGLBattleEffectSparks::createParticleGeometry()
{
    // Retained for compatibility; sparks now render procedurally on a quad.
}
