#include "publishglbattleeffectlight.h"
#include "battledialogmodeleffectlight.h"
#include "dmh_opengl.h"

// Vertex shader - standard textured quad
static const char *lightVertexShader = "#version 410 core\n"
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

// Fragment shader - radial light with optional flicker
static const char *lightFragmentShader = "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform float alpha;\n"
    "uniform float u_time;\n"
    "uniform vec4 u_color;\n"
    "uniform float u_size;\n"
    "uniform int u_gradientType;\n"
    "uniform float u_flickerEnabled;\n"
    "uniform float u_flickerFrequency;\n"
    "uniform float u_dimAmplitude;\n"
    "\n"
    "void main() {\n"
    "    vec2 uv = TexCoord - 0.5;\n"
    "    float dist = length(uv) * 2.0;\n"
    "    if(dist > 1.0) discard;\n"
    "\n"
    "    // Gradient falloff\n"
    "    float falloff;\n"
    "    if(u_gradientType == 0) {\n"
    "        // Linear falloff\n"
    "        falloff = 1.0 - dist;\n"
    "    } else {\n"
    "        // Quadratic (inverse-square-like) falloff\n"
    "        falloff = 1.0 - dist * dist;\n"
    "    }\n"
    "\n"
    "    // Flicker modulation\n"
    "    float flicker = 1.0;\n"
    "    if(u_flickerEnabled > 0.5) {\n"
    "        float flickerTime = u_time * max(u_flickerFrequency, 0.05);\n"
    "        float stepIndex = floor(flickerTime * 8.0);\n"
    "        float stepPhase = fract(flickerTime * 8.0);\n"
    "        float r1 = fract(sin((stepIndex + 13.1) * 12.9898) * 43758.5453);\n"
    "        float r2 = fract(sin((stepIndex + 14.1) * 12.9898) * 43758.5453);\n"
    "        float randomWave = mix(r1, r2, smoothstep(0.0, 1.0, stepPhase)) * 2.0 - 1.0;\n"
    "        float harmonic = sin(u_time * u_flickerFrequency * 11.37 + 0.73) * 0.35;\n"
    "        float flickerNoise = clamp(randomWave * 0.65 + harmonic, -1.0, 1.0);\n"
    "        float dim = (flickerNoise * 0.5 + 0.5) * u_dimAmplitude;\n"
    "        flicker = 1.0 - dim;\n"
    "    }\n"
    "\n"
    "    float finalAlpha = falloff * flicker * u_color.a * alpha;\n"
    "    FragColor = vec4(u_color.rgb, finalAlpha);\n"
    "}\n";

PublishGLBattleEffectLight::PublishGLBattleEffectLight(PublishGLScene* scene, BattleDialogModelEffectLight* effect) :
    PublishGLBattleEffectAnimated(scene, effect),
    _shaderGradientType(0),
    _shaderFlickerEnabled(0),
    _shaderFlickerFrequency(0),
    _shaderDimAmplitude(0)
{
}

PublishGLBattleEffectLight::~PublishGLBattleEffectLight()
{
}

const char* PublishGLBattleEffectLight::getVertexShaderSource() const
{
    return lightVertexShader;
}

const char* PublishGLBattleEffectLight::getFragmentShaderSource() const
{
    return lightFragmentShader;
}

void PublishGLBattleEffectLight::getEffectUniformLocations(QOpenGLFunctions* f)
{
    _shaderGradientType = f->glGetUniformLocation(_shaderProgram, "u_gradientType");
    _shaderFlickerEnabled = f->glGetUniformLocation(_shaderProgram, "u_flickerEnabled");
    _shaderFlickerFrequency = f->glGetUniformLocation(_shaderProgram, "u_flickerFrequency");
    _shaderDimAmplitude = f->glGetUniformLocation(_shaderProgram, "u_dimAmplitude");
}

void PublishGLBattleEffectLight::setEffectUniforms(QOpenGLFunctions* f)
{
    BattleDialogModelEffectLight* lightEffect = dynamic_cast<BattleDialogModelEffectLight*>(_effect);
    if(!lightEffect)
        return;

    f->glUniform1i(_shaderGradientType, lightEffect->getGradientType());
    f->glUniform1f(_shaderFlickerEnabled, lightEffect->getFlickerEnabled() ? 1.0f : 0.0f);
    f->glUniform1f(_shaderFlickerFrequency, static_cast<float>(lightEffect->getFlickerFrequency()));
    f->glUniform1f(_shaderDimAmplitude, static_cast<float>(lightEffect->getDimAmplitude()));
}
