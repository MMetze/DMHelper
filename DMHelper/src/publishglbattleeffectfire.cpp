#include "publishglbattleeffectfire.h"
#include "battledialogmodeleffectfire.h"
#include "dmh_opengl.h"

// Vertex shader - standard textured quad
static const char *fireVertexShader = "#version 410 core\n"
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

// Fragment shader - fire effect with noise and vertical bias
static const char *fireFragmentShader = "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform float alpha;\n"
    "uniform float u_time;\n"
    "uniform vec4 u_color;\n"
    "uniform float u_size;\n"
    "uniform float u_intensity;\n"
    "uniform vec4 u_darkColor;\n"
    "uniform vec4 u_lightColor;\n"
    "uniform float u_flickerSpeed;\n"
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
    "    if(r > 1.40) discard;\n"
    "\n"
    "    // Flow-mapped noise: structures advect outward from the emitter so flames clearly\n"
    "    // emanate from a central source. Two time-phases are crossfaded to keep flow continuous.\n"
    "    // Intensity scales gently. The low end is genuinely lazy (slow drift, soft flicker)\n"
    "    // while the high end raises into a raging flame.\n"
    "    float freq = 1.2 + u_intensity * 1.0;\n"
    "    float flickerRate = 0.6 + u_intensity * 1.4;\n"
    "    vec2 flow = uv * (0.55 + u_intensity * 0.45);\n"
    "    float speed = 0.22 + u_intensity * 0.55;\n"
    "    float phase1 = fract(u_time * speed);\n"
    "    float phase2 = fract(u_time * speed + 0.5);\n"
    "    float w = abs(2.0 * phase1 - 1.0);\n"
    "    vec2 sUV1 = uv - flow * phase1;\n"
    "    vec2 sUV2 = uv - flow * phase2;\n"
    "\n"
    "    float n1a = snoise(vec3(sUV1 * freq,        u_time * flickerRate       )) * 0.5 + 0.5;\n"
    "    float n1b = snoise(vec3(sUV2 * freq,        u_time * flickerRate       )) * 0.5 + 0.5;\n"
    "    float nLow = mix(n1a, n1b, w);\n"
    "    float n2a = snoise(vec3(sUV1 * freq * 2.1,  u_time * flickerRate * 1.7 )) * 0.5 + 0.5;\n"
    "    float n2b = snoise(vec3(sUV2 * freq * 2.1,  u_time * flickerRate * 1.7 )) * 0.5 + 0.5;\n"
    "    float nMid = mix(n2a, n2b, w);\n"
    "    float n3a = snoise(vec3(sUV1 * freq * 4.3,  u_time * flickerRate * 2.5 )) * 0.5 + 0.5;\n"
    "    float n3b = snoise(vec3(sUV2 * freq * 4.3,  u_time * flickerRate * 2.5 )) * 0.5 + 0.5;\n"
    "    float nHigh = mix(n3a, n3b, w);\n"
    "    float noise = nLow * 0.55 + nMid * 0.30 + nHigh * 0.15;\n"
    "\n"
    "    // Domain warp: gently nudge r by noise so the flame silhouette wavers and licks past r=1.\n"
    "    // Suppress the warp near the very center so the core stays anchored.\n"
    "    float centerProtect = exp(-r * r * 9.0);\n"
    "    float perturb = (noise - 0.5) * (0.55 + u_intensity * 0.15) * (1.0 - centerProtect);\n"
    "    float effectiveR = clamp(r - perturb, 0.0, 2.0);\n"
    "\n"
    "    // Gaussian density centered on the emitter.\n"
    "    float core = exp(-effectiveR * effectiveR * 3.0);\n"
    "\n"
    "    // Sharp high-frequency flicker on top of the base mask, damped near the center.\n"
    "    float flickerRaw = 0.85 + 0.18 * sin(u_time * u_flickerSpeed * 11.0)\n"
    "                              * sin(u_time * u_flickerSpeed * 6.7 + 1.3);\n"
    "    float pulseRaw   = 0.90 + 0.18 * sin(u_time * u_flickerSpeed * 5.3 + noise * 2.1);\n"
    "    float flicker = mix(flickerRaw, 1.0, centerProtect);\n"
    "    float pulse   = mix(pulseRaw,   1.0, centerProtect);\n"
    "\n"
    "    // Combine density and turbulent mask. The turbulent factor is lifted toward 1 near the\n"
    "    // center so the core never collapses to a thin/transparent value, even when noise is low.\n"
    "    float turbulent = mix(0.35 + noise * 1.30, 1.0, centerProtect);\n"
    "    float fireMask = core * turbulent * flicker * pulse;\n"
    "    // Hard floor: the very center is essentially always fully opaque.\n"
    "    fireMask = max(fireMask, centerProtect * 0.95);\n"
    "    fireMask = clamp(fireMask, 0.0, 1.0);\n"
    "\n"
    "    // Hot dense center, cooler at the tongues.\n"
    "    // Bias heat heavily toward center, then force it to ~1 in the protected core so the\n"
    "    // light color holds and only rarely dips toward the dark color.\n"
    "    float heat = clamp(pow(core, 0.55) * (0.70 + noise * 0.55), 0.0, 1.0);\n"
    "    heat = mix(heat, 1.0, centerProtect);\n"
    "    vec4 fireColor = mix(u_darkColor, u_lightColor, heat);\n"
    "    vec3 fireRgb = min(vec3(1.0), fireColor.rgb * (1.10 + heat * 0.55));\n"
    "\n"
    "    float finalAlpha = clamp(fireMask * fireColor.a * u_color.a * alpha * (2.10 + u_intensity * 0.20), 0.0, 1.0);\n"
    "    if(finalAlpha < 0.01) discard;\n"
    "    FragColor = vec4(fireRgb, finalAlpha);\n"
    "}\n";

PublishGLBattleEffectFire::PublishGLBattleEffectFire(PublishGLScene* scene, BattleDialogModelEffectFire* effect) :
    PublishGLBattleEffectAnimated(scene, effect),
    _shaderIntensity(0),
    _shaderDarkColor(0),
    _shaderLightColor(0),
    _shaderFlickerSpeed(0)
{
}

PublishGLBattleEffectFire::~PublishGLBattleEffectFire()
{
}

qreal PublishGLBattleEffectFire::getExtentMultiplier() const
{
    static constexpr qreal FIRE_EXTENT_MULTIPLIER = 1.50;
    return FIRE_EXTENT_MULTIPLIER;
}

const char* PublishGLBattleEffectFire::getVertexShaderSource() const
{
    return fireVertexShader;
}

const char* PublishGLBattleEffectFire::getFragmentShaderSource() const
{
    return fireFragmentShader;
}

void PublishGLBattleEffectFire::getEffectUniformLocations(QOpenGLFunctions* f)
{
    _shaderIntensity = f->glGetUniformLocation(_shaderProgram, "u_intensity");
    _shaderDarkColor = f->glGetUniformLocation(_shaderProgram, "u_darkColor");
    _shaderLightColor = f->glGetUniformLocation(_shaderProgram, "u_lightColor");
    _shaderFlickerSpeed = f->glGetUniformLocation(_shaderProgram, "u_flickerSpeed");
}

void PublishGLBattleEffectFire::setEffectUniforms(QOpenGLFunctions* f)
{
    BattleDialogModelEffectFire* fireEffect = dynamic_cast<BattleDialogModelEffectFire*>(_effect);
    if(!fireEffect)
        return;

    f->glUniform1f(_shaderIntensity, static_cast<float>(fireEffect->getIntensity()));

    QColor dc = fireEffect->getDarkColor();
    f->glUniform4f(_shaderDarkColor, dc.redF(), dc.greenF(), dc.blueF(), dc.alphaF());

    QColor lc = fireEffect->getLightColor();
    f->glUniform4f(_shaderLightColor, lc.redF(), lc.greenF(), lc.blueF(), lc.alphaF());

    f->glUniform1f(_shaderFlickerSpeed, static_cast<float>(fireEffect->getFlickerSpeed()));
}
