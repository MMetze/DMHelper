#include "layervideoeffect.h"
#include "layervideoeffectsettings.h"
#include <QMatrix4x4>

// Here are the various shader programs that we will use
const char *vertexShaderSourceBase =
    "#version 410 core\n"
    "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
    "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform float alpha;\n"
    "out vec4 ourColor; // output a color to the fragment shader\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   // note that we read the multiplication from right to left\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   ourColor = vec4(aColor, alpha); // set ourColor to the input color we got from the vertex data\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";

const char *vertexShaderSourceTransparentColor =
    "#version 410 core\n"
    "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
    "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform float alpha;\n"
    "uniform vec4 transparentColor;\n"
    "out vec4 ourColor; // output a color to the fragment shader\n"
    "out vec4 outTransparentColor;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   // note that we read the multiplication from right to left\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   ourColor = vec4(aColor, alpha); // set ourColor to the input color we got from the vertex data\n"
    "   TexCoord = aTexCoord;\n"
    "   outTransparentColor = transparentColor;\n"
    "}\0";

const char *fragmentShaderSourceBase =
    "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(texture1, TexCoord) * ourColor;\n"
    "}\0";

const char *fragmentShaderSourceRTransparent =
    "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(texture1, TexCoord) * ourColor;\n"
    "    FragColor.a = FragColor.r;\n"
    "}\0";

const char *fragmentShaderSourceGTransparent =
    "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(texture1, TexCoord) * ourColor;\n"
    "    FragColor.a = FragColor.g;\n"
    "}\0";

const char *fragmentShaderSourceBTransparent =
    "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(texture1, TexCoord) * ourColor;\n"
    "    FragColor.a = FragColor.b;\n"
    "}\0";

const char *fragmentShaderSourceTransparentColor =
    "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "in vec4 transparentColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "    if(texture(texture1, TexCoord) == transparentColor)\n"
    "        discard;\n"
    "    else\n"
    "        FragColor = texture(texture1, TexCoord) * ourColor;\n"
    "}\0";


LayerVideoEffect::LayerVideoEffect(const QString& name, const QString& filename, int order, QObject *parent) :
    LayerVideo{name, filename, order, parent},
    _recreateShaders(true),
    _effectType(LayerVideoEffectType_None),
    _colorize(false),
    _transparentColor(),
    _colorizeColor()
{
}

LayerVideoEffect::~LayerVideoEffect()
{
}

DMHelper::LayerType LayerVideoEffect::getType() const
{
    return DMHelper::LayerType_VideoEffect;
}

Layer* LayerVideoEffect::clone() const
{
    LayerVideoEffect* newLayer = new LayerVideoEffect(_name, _filename, _order);

    copyBaseValues(newLayer);

    return newLayer;
}

void LayerVideoEffect::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    _recreateShaders = true;
    LayerVideo::playerGLInitialize(renderer, scene);
}

void LayerVideoEffect::playerGLUninitialize()
{
    // Destroy the local shaders
    cleanupShadersGL();
    LayerVideo::playerGLUninitialize();
}

void LayerVideoEffect::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    if(_recreateShaders)
    {
        cleanupShadersGL();
        createShadersGL();
    }

    LayerVideo::playerGLPaint(functions, defaultModelMatrix, projectionMatrix);
}

void LayerVideoEffect::playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA)
{
    Q_UNUSED(modelMatrixRGB);
    Q_UNUSED(projectionMatrixRGB);
    Q_UNUSED(programRGBA);
    Q_UNUSED(projectionMatrixRGBA);
    Q_UNUSED(modelMatrixRGBA);
    Q_UNUSED(alphaRGBA);

    _shaderProgramRGB = programRGB;
}

void LayerVideoEffect::editSettings()
{
    LayerVideoEffectSettings* dlg = new LayerVideoEffectSettings();
    int result = dlg->exec();

    if(result == QDialog::Accepted)
    {
        if(_effectType != dlg->getEffectType())
        {
            _effectType = dlg->getEffectType();
            _recreateShaders = true;
        }

        if(_transparentColor != dlg->getTransparentColor())
        {
            _transparentColor = dlg->getTransparentColor();
            _recreateShaders = true;
        }

        if(_colorize != dlg->isColorize())
        {
            _colorize = dlg->isColorize();
            _recreateShaders = true;
        }

        if(_colorizeColor != dlg->getColorizeColor())
        {
            _colorizeColor = dlg->getColorizeColor();
            _recreateShaders = true;
        }

        if(_recreateShaders)
            emit dirty();
    }

    dlg->deleteLater();
}

void LayerVideoEffect::createShadersGL()
{
    // Create the local shader program
    if((!QOpenGLContext::currentContext()) || (_shaderProgramRGBA != 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    int  success;
    char infoLog[512];

    const char * vertexSource = getVertexShaderSource();
    /*
    const char *vertexShaderSourceRGBA = "#version 410 core\n"
                                         "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
                                         "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
                                         "layout (location = 2) in vec2 aTexCoord;\n"
                                         "uniform mat4 model;\n"
                                         "uniform mat4 view;\n"
                                         "uniform mat4 projection;\n"
                                         "uniform float alpha;\n"
                                         "out vec4 ourColor; // output a color to the fragment shader\n"
                                         "out vec2 TexCoord;\n"
                                         "void main()\n"
                                         "{\n"
                                         "   // note that we read the multiplication from right to left\n"
                                         "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                         "   ourColor = vec4(aColor, alpha); // set ourColor to the input color we got from the vertex data\n"
                                         "   TexCoord = aTexCoord;\n"
                                         "}\0";
    */

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

    const char * fragmentSource = getFragmentShaderSource();

    /*
    const char *fragmentShaderSourceRGBA = "#version 410 core\n"
                                           "out vec4 FragColor;\n"
                                           "in vec4 ourColor;\n"
                                           "in vec2 TexCoord;\n"
                                           "uniform sampler2D texture1;\n"
                                           "void main()\n"
                                           "{\n"
                                           "    FragColor = texture(texture1, TexCoord) * ourColor;\n"
                                           "    FragColor.a = FragColor.r;\n"
                                           "}\0";
    */

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

    QMatrix4x4 modelMatrix;
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), 0); // set it manually
    f->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBA, "view"), 1, GL_FALSE, viewMatrix.constData());

    _recreateShaders = false;
}

void LayerVideoEffect::cleanupShadersGL()
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
}

const char* LayerVideoEffect::getVertexShaderSource()
{
    if(_effectType == LayerVideoEffectType_TransparentColor)
        return vertexShaderSourceTransparentColor;
    else
        return vertexShaderSourceBase;
}

const char* LayerVideoEffect::getFragmentShaderSource()
{
    switch(_effectType)
    {
        case LayerVideoEffectType_Red:
            return fragmentShaderSourceRTransparent;
        case LayerVideoEffectType_Green:
            return fragmentShaderSourceGTransparent;
        case LayerVideoEffectType_Blue:
            return fragmentShaderSourceBTransparent;
        case LayerVideoEffectType_TransparentColor:
            return fragmentShaderSourceTransparentColor;
        default:
            break;
    }

    return fragmentShaderSourceBase;
}
