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
    "    vec2 uv = TexCoord - 0.5;\n"
    "    float dist = length(uv) * 2.0;\n"
    "    if(dist > 1.0) discard;\n"
    "\n"
    "    // Frequency scales with intensity: higher = more turbulent/raging\n"
    "    float freq = 3.0 + u_intensity * 5.0;\n"
    "\n"
    "    // Vertical bias: noise scrolls upward (simulating rising heat)\n"
    "    vec2 noiseUV = TexCoord * freq;\n"
    "    float scrollSpeed = 1.0 + u_intensity * 2.0;\n"
    "    float n1 = snoise(vec3(noiseUV.x, noiseUV.y - u_time * scrollSpeed, u_time * 0.5)) * 0.5 + 0.5;\n"
    "    float n2 = snoise(vec3(noiseUV.x * 2.0, noiseUV.y * 2.0 - u_time * scrollSpeed * 1.5, u_time * 0.8)) * 0.5 + 0.5;\n"
    "    float noise = n1 * 0.6 + n2 * 0.4;\n"
    "\n"
    "    // Flicker modulation\n"
    "    float flicker = 1.0 - 0.15 * sin(u_time * u_flickerSpeed * 12.0) * sin(u_time * u_flickerSpeed * 7.3 + 1.5);\n"
    "\n"
    "    // Fire shape: stronger at center, fading at edges\n"
    "    float fireMask = (1.0 - smoothstep(0.3, 1.0, dist)) * noise * flicker;\n"
    "\n"
    "    // Dark-to-light color: center is light (hot), edges are dark\n"
    "    vec4 fireColor = mix(u_darkColor, u_lightColor, (1.0 - dist) * noise);\n"
    "\n"
    "    float finalAlpha = fireMask * fireColor.a * alpha;\n"
    "    FragColor = vec4(fireColor.rgb, finalAlpha);\n"
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
