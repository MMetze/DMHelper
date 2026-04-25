#include "publishglbattleeffectsmoke.h"
#include "battledialogmodeleffectsmoke.h"
#include "dmh_opengl.h"
#include <QtMath>

// Vertex shader - standard textured quad
static const char *smokeVertexShader = "#version 410 core\n"
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

// Fragment shader - simplex noise smoke effect
static const char *smokeFragmentShader = "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform float alpha;\n"
    "uniform float u_time;\n"
    "uniform vec4 u_color;\n"
    "uniform float u_size;\n"
    "uniform float u_density;\n"
    "uniform vec4 u_centerColor;\n"
    "uniform vec4 u_edgeColor;\n"
    "uniform float u_billowFactor;\n"
    "uniform vec2 u_windVec;\n"
    "\n"
    "vec3 mod289(const in vec3 x) { return x - floor(x * (1. / 289.)) * 289.; }\n"
    "vec4 mod289(const in vec4 x) { return x - floor(x * (1. / 289.)) * 289.; }\n"
    "vec4 permute(const in vec4 v) { return mod289(((v * 34.0) + 1.0) * v); }\n"
    "vec4 taylorInvSqrt(in vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }\n"
    "\n"
    "float snoise(in vec3 v) {\n"
    "    const vec2 C = vec2(1.0/6.0, 1.0/3.0);\n"
    "    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);\n"
    "    vec3 i = floor(v + dot(v, C.yyy));\n"
    "    vec3 x0 = v - i + dot(i, C.xxx);\n"
    "    vec3 g = step(x0.yzx, x0.xyz);\n"
    "    vec3 l = 1.0 - g;\n"
    "    vec3 i1 = min(g.xyz, l.zxy);\n"
    "    vec3 i2 = max(g.xyz, l.zxy);\n"
    "    vec3 x1 = x0 - i1 + C.xxx;\n"
    "    vec3 x2 = x0 - i2 + C.yyy;\n"
    "    vec3 x3 = x0 - D.yyy;\n"
    "    i = mod289(i);\n"
    "    vec4 p = permute(permute(permute(\n"
    "        i.z + vec4(0.0, i1.z, i2.z, 1.0))\n"
    "        + i.y + vec4(0.0, i1.y, i2.y, 1.0))\n"
    "        + i.x + vec4(0.0, i1.x, i2.x, 1.0));\n"
    "    float n_ = 0.142857142857;\n"
    "    vec3 ns = n_ * D.wyz - D.xzx;\n"
    "    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);\n"
    "    vec4 x_ = floor(j * ns.z);\n"
    "    vec4 y_ = floor(j - 7.0 * x_);\n"
    "    vec4 x = x_ * ns.x + ns.yyyy;\n"
    "    vec4 y = y_ * ns.x + ns.yyyy;\n"
    "    vec4 h = 1.0 - abs(x) - abs(y);\n"
    "    vec4 b0 = vec4(x.xy, y.xy);\n"
    "    vec4 b1 = vec4(x.zw, y.zw);\n"
    "    vec4 s0 = floor(b0) * 2.0 + 1.0;\n"
    "    vec4 s1 = floor(b1) * 2.0 + 1.0;\n"
    "    vec4 sh = -step(h, vec4(0.0));\n"
    "    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;\n"
    "    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;\n"
    "    vec3 p0 = vec3(a0.xy, h.x);\n"
    "    vec3 p1 = vec3(a0.zw, h.y);\n"
    "    vec3 p2 = vec3(a1.xy, h.z);\n"
    "    vec3 p3 = vec3(a1.zw, h.w);\n"
    "    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2,p2), dot(p3,p3)));\n"
    "    p0 *= norm.x; p1 *= norm.y; p2 *= norm.z; p3 *= norm.w;\n"
    "    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);\n"
    "    m = m * m;\n"
    "    return 42.0 * dot(m*m, vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec2 uv = TexCoord - 0.5;\n"
    "    float dist = length(uv) * 2.0;\n"
    "    // Hard cull only well past the active envelope; billows live outside the nominal radius.\n"
    "    if(dist > 1.95) discard;\n"
    "\n"
    "    // Apply wind drift to noise coordinates\n"
    "    vec2 noiseUV = TexCoord * 3.0 + u_windVec * u_time;\n"
    "\n"
    "    // Multi-octave noise: billow controls octave count blend\n"
    "    float n1 = snoise(vec3(noiseUV, u_time * 0.3)) * 0.5 + 0.5;\n"
    "    float n2 = snoise(vec3(noiseUV * 2.0, u_time * 0.5)) * 0.5 + 0.5;\n"
    "    float n3 = snoise(vec3(noiseUV * 4.0, u_time * 0.7)) * 0.5 + 0.5;\n"
    "    float noise = mix(n1, n1 * 0.5 + n2 * 0.35 + n3 * 0.15, u_billowFactor);\n"
    "\n"
    "    // Low-frequency turbulence deforms the silhouette so plume edges bulge irregularly past nominal radius.\n"
    "    float silhouetteNoise = snoise(vec3(TexCoord * 1.8, u_time * 0.25)) * 0.5 + 0.5;\n"
    "    float bulge = (silhouetteNoise - 0.45) * (0.55 + u_billowFactor * 0.55);\n"
    "    // Effective radius the smoke fills: nominal core (~1.0) plus billow bulge that can reach ~1.7.\n"
    "    float effectiveRadius = 1.0 + bulge + noise * 0.35;\n"
    "\n"
    "    // Density threshold with a softer shoulder so smoke keeps a body outside the core.\n"
    "    float denseMask = smoothstep(1.0 - u_density, 1.0, noise);\n"
    "    float wispyMask = smoothstep(0.55 - u_density * 0.25, 0.98, noise);\n"
    "    float smokeMask = mix(wispyMask, denseMask, 0.62);\n"
    "\n"
    "    // Radial fade is relative to the perturbed envelope, so wisps clearly billow beyond the nominal bound.\n"
    "    float radialT = dist / max(0.35, effectiveRadius);\n"
    "    float coreFade = 1.0 - smoothstep(0.45, 0.95, radialT);\n"
    "    float outerFade = 1.0 - smoothstep(0.85, 1.30, radialT);\n"
    "    float edgeFade = max(coreFade, outerFade * 0.65);\n"
    "\n"
    "    // Color mix references the perturbed envelope as well.\n"
    "    float colorMix = clamp(radialT * 0.82, 0.0, 1.0);\n"
    "    vec4 smokeColor = mix(u_centerColor, u_edgeColor, colorMix);\n"
    "    vec3 smokeRgb = mix(smokeColor.rgb, min(vec3(1.0), smokeColor.rgb * 1.18 + vec3(0.04)), denseMask * 0.28);\n"
    "\n"
    "    float finalAlpha = clamp(smokeMask * edgeFade * smokeColor.a * u_color.a * alpha * 2.65, 0.0, 1.0);\n"
    "    FragColor = vec4(smokeRgb, finalAlpha);\n"
    "}\n";

PublishGLBattleEffectSmoke::PublishGLBattleEffectSmoke(PublishGLScene* scene, BattleDialogModelEffectSmoke* effect) :
    PublishGLBattleEffectAnimated(scene, effect),
    _shaderDensity(0),
    _shaderCenterColor(0),
    _shaderEdgeColor(0),
    _shaderBillowFactor(0),
    _shaderWindVec(0)
{
}

PublishGLBattleEffectSmoke::~PublishGLBattleEffectSmoke()
{
}

qreal PublishGLBattleEffectSmoke::getExtentMultiplier() const
{
    static constexpr qreal SMOKE_EXTENT_MULTIPLIER = 2.20;
    return SMOKE_EXTENT_MULTIPLIER;
}

const char* PublishGLBattleEffectSmoke::getVertexShaderSource() const
{
    return smokeVertexShader;
}

const char* PublishGLBattleEffectSmoke::getFragmentShaderSource() const
{
    return smokeFragmentShader;
}

void PublishGLBattleEffectSmoke::getEffectUniformLocations(QOpenGLFunctions* f)
{
    _shaderDensity = f->glGetUniformLocation(_shaderProgram, "u_density");
    _shaderCenterColor = f->glGetUniformLocation(_shaderProgram, "u_centerColor");
    _shaderEdgeColor = f->glGetUniformLocation(_shaderProgram, "u_edgeColor");
    _shaderBillowFactor = f->glGetUniformLocation(_shaderProgram, "u_billowFactor");
    _shaderWindVec = f->glGetUniformLocation(_shaderProgram, "u_windVec");
}

void PublishGLBattleEffectSmoke::setEffectUniforms(QOpenGLFunctions* f)
{
    BattleDialogModelEffectSmoke* smokeEffect = dynamic_cast<BattleDialogModelEffectSmoke*>(_effect);
    if(!smokeEffect)
        return;

    f->glUniform1f(_shaderDensity, static_cast<float>(smokeEffect->getDensity()));

    QColor cc = smokeEffect->getCenterColor();
    f->glUniform4f(_shaderCenterColor, cc.redF(), cc.greenF(), cc.blueF(), cc.alphaF());

    QColor ec = smokeEffect->getEdgeColor();
    f->glUniform4f(_shaderEdgeColor, ec.redF(), ec.greenF(), ec.blueF(), ec.alphaF());

    f->glUniform1f(_shaderBillowFactor, static_cast<float>(smokeEffect->getBillowFactor()));

    qreal dir = qDegreesToRadians(smokeEffect->getWindDirection());
    qreal str = smokeEffect->getWindStrength();
    f->glUniform2f(_shaderWindVec, static_cast<float>(qCos(dir) * str), static_cast<float>(qSin(dir) * str));
}
