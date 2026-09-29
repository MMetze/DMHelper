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
    "uniform float u_seed;\n"
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
    "    // Centered coordinates: r is normalized so r=1 == nominal radius.\n"
    "    vec2 uv = (TexCoord - 0.5) * 2.0;\n"
    "    float r = length(uv);\n"
    "    if(r > 1.45) discard;\n"
    "\n"
    "    // Flow-mapped noise: structures advect outward from the emitter (and downwind),\n"
    "    // sampled in Cartesian space so there are no radial spokes. Two time-phases are\n"
    "    // crossfaded so the flow looks continuous.\n"
    "    vec2 wind = u_windVec;\n"
    "    // Per-instance domain offset so different smoke effects don't share an identical noise path.\n"
    "    vec2 seedOffset = vec2(cos(u_seed), sin(u_seed * 1.37)) * 4.0;\n"
    "    float seedTime = u_seed * 0.61;\n"
    "    // Velocity field: radial outward (proportional to distance, like an expanding plume)\n"
    "    // plus a strong downwind drift. Billow factor accelerates outward production.\n"
    "    float billowSpeed = 1.0 + u_billowFactor * 1.6;\n"
    "    vec2 flow = (uv * 0.55 + wind * 1.30) * billowSpeed;\n"
    "    float speed = (0.18 + u_billowFactor * 0.35) * 1.0;\n"
    "    float phase1 = fract(u_time * speed);\n"
    "    float phase2 = fract(u_time * speed + 0.5);\n"
    "    float w = abs(2.0 * phase1 - 1.0);\n"
    "\n"
    "    vec2 sUV1 = uv - flow * phase1;\n"
    "    vec2 sUV2 = uv - flow * phase2;\n"
    "    sUV1 += seedOffset;\n"
    "    sUV2 += seedOffset;\n"
    "\n"
    "    // Time evolution rate of the noise field also scales with billow.\n"
    "    float churn = 1.0 + u_billowFactor * 1.4;\n"
    "    float n1a = snoise(vec3(sUV1 * 1.4, u_time * 0.10 * churn + seedTime)) * 0.5 + 0.5;\n"
    "    float n1b = snoise(vec3(sUV2 * 1.4, u_time * 0.10 * churn + seedTime)) * 0.5 + 0.5;\n"
    "    float nLow = mix(n1a, n1b, w);\n"
    "    float n2a = snoise(vec3(sUV1 * 2.8, u_time * 0.18 * churn + seedTime)) * 0.5 + 0.5;\n"
    "    float n2b = snoise(vec3(sUV2 * 2.8, u_time * 0.18 * churn + seedTime)) * 0.5 + 0.5;\n"
    "    float nMid = mix(n2a, n2b, w);\n"
    "    float n3a = snoise(vec3(sUV1 * 5.2, u_time * 0.26 * churn + seedTime)) * 0.5 + 0.5;\n"
    "    float n3b = snoise(vec3(sUV2 * 5.2, u_time * 0.26 * churn + seedTime)) * 0.5 + 0.5;\n"
    "    float nHigh = mix(n3a, n3b, w);\n"
    "    float noise = mix(nLow, nLow * 0.50 + nMid * 0.32 + nHigh * 0.18, u_billowFactor);\n"
    "\n"
    "    // Domain warp: noise pulls/pushes the effective radius so puffs bulge past r=1.\n"
    "    float perturb = (noise - 0.5) * (0.45 + u_billowFactor * 0.35);\n"
    "    // Wind stretches the envelope strongly downwind.\n"
    "    float windStretch = dot(uv, wind) * 0.40;\n"
    "    float effectiveR = clamp(r - perturb - windStretch, 0.0, 2.0);\n"
    "\n"
    "    // Soft Gaussian density centered on the emitter.\n"
    "    float density = exp(-effectiveR * effectiveR * 2.4);\n"
    "\n"
    "    // Mild center protection: the emitter source reads stronger than the surroundings,\n"
    "    // but smoke is still allowed to thin and darken there (unlike fire's hard floor).\n"
    "    float centerProtect = exp(-r * r * 6.0) * 0.55;\n"
    "\n"
    "    // Wisp threshold from noise so the plume isn't a smooth disc.\n"
    "    float denseMask = smoothstep(1.0 - u_density, 1.0, noise);\n"
    "    float wispyMask = smoothstep(0.50 - u_density * 0.25, 0.95, noise);\n"
    "    float smokeMask = density * mix(wispyMask, denseMask, 0.55);\n"
    "    // Lift the mask near the center so the source stays visibly denser than the wisps around it.\n"
    "    smokeMask = mix(smokeMask, max(smokeMask, density), centerProtect);\n"
    "    smokeMask = clamp(smokeMask, 0.0, 1.0);\n"
    "\n"
    "    // Color: lit center, darker edges (or whatever the user picked).\n"
    "    // Bias mix with a power curve so the center color holds across most of the plume.\n"
    "    float colorMix = pow(clamp(r, 0.0, 1.0), 1.8);\n"
    "    vec4 smokeColor = mix(u_centerColor, u_edgeColor, colorMix);\n"
    "    vec3 smokeRgb = mix(smokeColor.rgb,\n"
    "                        min(vec3(1.0), smokeColor.rgb * 1.25 + vec3(0.06)),\n"
    "                        denseMask * 0.35);\n"
    "\n"
    "    float finalAlpha = clamp(smokeMask * smokeColor.a * u_color.a * alpha * 2.40, 0.0, 1.0);\n"
    "    if(finalAlpha < 0.01) discard;\n"
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
    static constexpr qreal SMOKE_EXTENT_MULTIPLIER = 1.55;
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
