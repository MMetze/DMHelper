#include "layervideotransparent.h"
#include <QMatrix4x4>

LayerVideoTransparent::LayerVideoTransparent(const QString& name, const QString& filename, int order, QObject *parent) :
    LayerVideo{name, filename, order, parent}
{
}

LayerVideoTransparent::~LayerVideoTransparent()
{
}

DMHelper::LayerType LayerVideoTransparent::getType() const
{
    return DMHelper::LayerType_VideoTransparent;
}

Layer* LayerVideoTransparent::clone() const
{
    LayerVideoTransparent* newLayer = new LayerVideoTransparent(_name, _filename, _order);

    copyBaseValues(newLayer);

    return newLayer;
}

void LayerVideoTransparent::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    createShaders();
    LayerVideo::playerGLInitialize(renderer, scene);
}

void LayerVideoTransparent::playerGLUninitialize()
{
    // Destroy the local shaders
    if((QOpenGLContext::currentContext()) && (_shaderProgramRGBA > 0))
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        if(f)
            f->glDeleteProgram(_shaderProgramRGBA);

        _shaderProgramRGBA = 0;
        _shaderModelMatrixRGBA = 0;
        _shaderProjectionMatrixRGBA = 0;
        _shaderAlphaRGBA = 0;
    }

    LayerVideo::playerGLUninitialize();
}

void LayerVideoTransparent::playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA)
{
    Q_UNUSED(modelMatrixRGB);
    Q_UNUSED(projectionMatrixRGB);
    Q_UNUSED(programRGBA);
    Q_UNUSED(projectionMatrixRGBA);
    Q_UNUSED(modelMatrixRGBA);
    Q_UNUSED(alphaRGBA);

    _shaderProgramRGB = programRGB;
}

void LayerVideoTransparent::createShaders()
{
    // Create the local shader program
    if((!QOpenGLContext::currentContext()) || (_shaderProgramRGBA != 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    int  success;
    char infoLog[512];

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

    unsigned int vertexShaderRGBA;
    vertexShaderRGBA = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShaderRGBA, 1, &vertexShaderSourceRGBA, NULL);
    f->glCompileShader(vertexShaderRGBA);

    f->glGetShaderiv(vertexShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerVideoTransparent] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

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

    unsigned int fragmentShaderRGBA;
    fragmentShaderRGBA = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGBA, 1, &fragmentShaderSourceRGBA, NULL);
    f->glCompileShader(fragmentShaderRGBA);

    f->glGetShaderiv(fragmentShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerVideoTransparent] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
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
        qDebug() << "[LayerVideoTransparent] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
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
}

