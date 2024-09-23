#include "layervideoeffect.h"
#include "layervideoeffectsettings.h"
#include "tokeneditor.h"
#include <QMatrix4x4>
#include <QGraphicsPixmapItem>

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

const char *vertexShaderSourceColorize =
    "#version 410 core\n"
    "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
    "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform float alpha;\n"
    "uniform vec3 colorizeColor;\n"
    "out vec4 ourColor; // output a color to the fragment shader\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   // note that we read the multiplication from right to left\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   ourColor = vec4(aColor * colorizeColor, alpha); // set ourColor to the input color we got from the vertex data\n"
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
    "uniform vec3 transparentColor;\n"
    "uniform float transparentTolerance;\n"
    "out vec4 ourColor; // output a color to the fragment shader\n"
    "out vec3 fragTransparentColor;\n"
    "out float fragTransparentTolerance;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   // note that we read the multiplication from right to left\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   ourColor = vec4(aColor, alpha); // set ourColor to the input color we got from the vertex data\n"
    "   TexCoord = aTexCoord;\n"
    "   fragTransparentColor = transparentColor;\n"
    "   fragTransparentTolerance = transparentTolerance;\n"
    "}\0";

const char *vertexShaderSourceTransparentColorColorize =
    "#version 410 core\n"
    "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
    "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform float alpha;\n"
    "uniform vec3 colorizeColor;\n"
    "uniform vec3 transparentColor;\n"
    "uniform float transparentTolerance;\n"
    "out vec4 ourColor; // output a color to the fragment shader\n"
    "out vec3 fragTransparentColor;\n"
    "out float fragTransparentTolerance;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   // note that we read the multiplication from right to left\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   ourColor = vec4(aColor * colorizeColor, alpha); // set ourColor to the input color we got from the vertex data\n"
    "   TexCoord = aTexCoord;\n"
    "   fragTransparentColor = transparentColor;\n"
    "   fragTransparentTolerance = transparentTolerance;\n"
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
    "    FragColor = texture(texture1, TexCoord);\n"
    "    FragColor.a = FragColor.r;\n"
    "    FragColor = FragColor * ourColor;\n"
    "}\0";

const char *fragmentShaderSourceGTransparent =
    "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(texture1, TexCoord);\n"
    "    FragColor.a = FragColor.g;\n"
    "    FragColor = FragColor * ourColor;\n"
    "}\0";

const char *fragmentShaderSourceBTransparent =
    "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(texture1, TexCoord);\n"
    "    FragColor.a = FragColor.b;\n"
    "    FragColor = FragColor * ourColor;\n"
    "}\0";

const char *fragmentShaderSourceTransparentColor =
    "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "in vec3 fragTransparentColor;\n"
    "in float fragTransparentTolerance;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "    vec4 texColor = texture(texture1, TexCoord);\n"
    "    vec3 delta = texColor.rgb - fragTransparentColor;\n"
    "    if(dot(delta, delta) < fragTransparentTolerance)\n"
    "        discard;\n"
    "    FragColor = texColor * ourColor;\n"
    "}\0";

LayerVideoEffect::LayerVideoEffect(const QString& name, const QString& filename, int order, QObject *parent) :
    LayerVideo{name, filename, order, parent},
    _recreateShaders(true),
    _effectTransparencyType(DMHelper::TransparentType_None),
    _transparentColor(),
    _transparentTolerance(0.15),
    _colorize(false),
    _colorizeColor(),
    _effectScreenshot(),
    _effectDirty(true),
    _shaderTransparentColor(0),
    _shaderTransparentTolerance(0),
    _shaderColorizeColor(0)
{
    connect(this, &LayerVideo::screenshotAvailable, this, &LayerVideoEffect::updateEffectScreenshot);
}

LayerVideoEffect::~LayerVideoEffect()
{
}

void LayerVideoEffect::inputXML(const QDomElement &element, bool isImport)
{
    if(element.hasAttribute("effect"))
        _effectTransparencyType = static_cast<DMHelper::TransparentType>(element.attribute("effect").toInt());

    if(element.hasAttribute("transparentColor"))
        _transparentColor = QColor(element.attribute("transparentColor"));

    if(element.hasAttribute("transparentTolerance"))
        _transparentTolerance = element.attribute("transparentTolerance").toDouble();

    if(element.hasAttribute("colorize"))
        _colorize = static_cast<bool>(element.attribute("colorize").toInt());

    if(element.hasAttribute("colorizeColor"))
        _colorizeColor = QColor(element.attribute("colorizeColor"));

    LayerVideo::inputXML(element, isImport);
}

bool LayerVideoEffect::hasSettings() const
{
    return true;
}

DMHelper::LayerType LayerVideoEffect::getType() const
{
    return DMHelper::LayerType_VideoEffect;
}

Layer* LayerVideoEffect::clone() const
{
    LayerVideoEffect* newLayer = new LayerVideoEffect(_name, _filename, _order);

    copyBaseValues(newLayer);

    newLayer->_playAudio = _playAudio;
    newLayer->_effectTransparencyType = _effectTransparencyType;
    newLayer->_transparentColor = _transparentColor;
    newLayer->_transparentTolerance = _transparentTolerance;
    newLayer->_colorize = _colorize;
    newLayer->_colorizeColor = _colorizeColor;
    newLayer->_effectScreenshot = _effectScreenshot;
    newLayer->_effectDirty = _effectDirty;

    return newLayer;
}

QImage LayerVideoEffect::getScreenshot() const
{
    return _effectScreenshot.isNull() ? LayerVideo::getScreenshot() : _effectScreenshot;
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
    DMH_DEBUG_OPENGL_PAINTGL();

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
    dlg->setEffectTransparencyType(_effectTransparencyType);
    dlg->setTransparentColor(_transparentColor);
    dlg->setTransparentTolerance(_transparentTolerance);
    dlg->setColorize(_colorize);
    dlg->setColorizeColor(_colorizeColor);
    dlg->setPreviewImage(_layerScreenshot);
    int result = dlg->exec();

    if(result == QDialog::Accepted)
    {
        if(_effectTransparencyType != dlg->getEffectTransparencyType())
        {
            _effectTransparencyType = dlg->getEffectTransparencyType();
            _effectDirty = true;
        }

        if(_transparentColor != dlg->getTransparentColor())
        {
            _transparentColor = dlg->getTransparentColor();
            _effectDirty = true;
        }

        if(_transparentTolerance != dlg->getTransparentTolerance())
        {
            _transparentTolerance = dlg->getTransparentTolerance();
            _effectDirty = true;
        }

        if(_colorize != dlg->isColorize())
        {
            _colorize = dlg->isColorize();
            _effectDirty = true;
        }

        if(_colorizeColor != dlg->getColorizeColor())
        {
            _colorizeColor = dlg->getColorizeColor();
            _effectDirty = true;
        }

        _recreateShaders = _effectDirty;
        emit screenshotAvailable(); // should update the local screenshot and an editor dialog
        updateImage();

        if(_recreateShaders)
            emit dirty();
    }

    dlg->deleteLater();
}

void LayerVideoEffect::playerGLSetUniforms(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    LayerVideo::playerGLSetUniforms(functions, defaultModelMatrix, projectionMatrix);

    if(_effectTransparencyType == DMHelper::TransparentType_TransparentColor)
    {
        DMH_DEBUG_OPENGL_glUniform3f(_shaderTransparentColor, _transparentColor.redF(), _transparentColor.greenF(), _transparentColor.blueF());
        functions->glUniform3f(_shaderTransparentColor, _transparentColor.redF(), _transparentColor.greenF(), _transparentColor.blueF());
        DMH_DEBUG_OPENGL_glUniform1f(_shaderTransparentTolerance, _transparentTolerance);
        functions->glUniform1f(_shaderTransparentTolerance, _transparentTolerance);
    }

    if(_colorize)
    {
        DMH_DEBUG_OPENGL_glUniform3f(_shaderColorizeColor, _colorizeColor.redF(), _colorizeColor.greenF(), _colorizeColor.blueF());
        functions->glUniform3f(_shaderColorizeColor, _colorizeColor.redF(), _colorizeColor.greenF(), _colorizeColor.blueF());
    }
}

void LayerVideoEffect::updateEffectScreenshot()
{
    if((!_effectDirty) || (_layerScreenshot.isNull()))
        return;

    TokenEditor* editor = new TokenEditor();

    editor->setSourceImage(_layerScreenshot);
    editor->setTransparentValue(_effectTransparencyType);
    editor->setTransparentColor(_transparentColor);
    editor->setTransparentLevel(static_cast<int>(_transparentTolerance * 100.0));
    editor->setColorize(_colorize);
    editor->setColorizeColor(_colorizeColor);
    editor->setSquareFinalImage(false);
    _effectScreenshot = editor->getFinalImage();

    editor->deleteLater();

    _effectDirty = false;
}

void LayerVideoEffect::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    if(_effectTransparencyType != DMHelper::TransparentType_None)
        element.setAttribute("effect", static_cast<int>(_effectTransparencyType));

    if((_effectTransparencyType == DMHelper::TransparentType_TransparentColor) && (_transparentColor.isValid()) && (_transparentColor != Qt::black))
        element.setAttribute("transparentColor", _transparentColor.name());

    if((_effectTransparencyType == DMHelper::TransparentType_TransparentColor) && (_transparentTolerance != 0.15))
        element.setAttribute("transparentTolerance", _transparentTolerance);

    if(_colorize)
        element.setAttribute("colorize", 1);

    if((_colorize) && (_colorizeColor.isValid()) && (_colorizeColor != Qt::black))
        element.setAttribute("colorizeColor", _colorizeColor.name());

    LayerVideo::internalOutputXML(doc, element, targetDirectory, isExport);
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
    DMH_DEBUG_OPENGL_glCreateProgram(_shaderProgramRGBA, "_shaderProgramRGBA");

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

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    f->glUseProgram(_shaderProgramRGBA);
    f->glDeleteShader(vertexShaderRGBA);
    f->glDeleteShader(fragmentShaderRGBA);
    _shaderModelMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "model");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, _shaderModelMatrixRGBA, "model");
    _shaderProjectionMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "projection");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, _shaderProjectionMatrixRGBA, "projection");
    _shaderAlphaRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "alpha");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, _shaderAlphaRGBA, "alpha");

    if(_effectTransparencyType == DMHelper::TransparentType_TransparentColor)
    {
        _shaderTransparentColor = f->glGetUniformLocation(_shaderProgramRGBA, "transparentColor");
        DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, _shaderTransparentColor, "transparentColor");
        _shaderTransparentTolerance = f->glGetUniformLocation(_shaderProgramRGBA, "transparentTolerance");
        DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, _shaderTransparentTolerance, "transparentTolerance");
    }

    if(_colorize)
    {
        _shaderColorizeColor = f->glGetUniformLocation(_shaderProgramRGBA, "colorizeColor");
        DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, _shaderColorizeColor, "colorizeColor");
    }

    QMatrix4x4 modelMatrix;
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), "texture1");
    DMH_DEBUG_OPENGL_glUniform1i(f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), 0); // set it manually
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), 0); // set it manually
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData(), modelMatrix);
    f->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, f->glGetUniformLocation(_shaderProgramRGBA, "view"), "view");
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBA, "view"), 1, GL_FALSE, viewMatrix.constData(), viewMatrix);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBA, "view"), 1, GL_FALSE, viewMatrix.constData());

    _recreateShaders = false;
}

void LayerVideoEffect::cleanupShadersGL()
{
    if((!QOpenGLContext::currentContext()) || (_shaderProgramRGBA == 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(f)
    {
        DMH_DEBUG_OPENGL_Singleton::removeProgram(_shaderProgramRGBA);
        f->glDeleteProgram(_shaderProgramRGBA);
    }

    _shaderProgramRGBA = 0;
    _shaderModelMatrixRGBA = 0;
    _shaderProjectionMatrixRGBA = 0;
    _shaderAlphaRGBA = 0;
    _shaderTransparentColor = 0;
    _shaderTransparentTolerance = 0;
    _shaderColorizeColor = 0;
}

const char* LayerVideoEffect::getVertexShaderSource()
{
    if(_colorize)
    {
        if(_effectTransparencyType == DMHelper::TransparentType_TransparentColor)
            return vertexShaderSourceTransparentColorColorize;
        else
            return vertexShaderSourceColorize;
    }
    else
    {
        if(_effectTransparencyType == DMHelper::TransparentType_TransparentColor)
            return vertexShaderSourceTransparentColor;
        else
            return vertexShaderSourceBase;
    }
}

const char* LayerVideoEffect::getFragmentShaderSource()
{
    switch(_effectTransparencyType)
    {
        case DMHelper::TransparentType_RedChannel:
            return fragmentShaderSourceRTransparent;
        case DMHelper::TransparentType_GreenChannel:
            return fragmentShaderSourceGTransparent;
        case DMHelper::TransparentType_BlueChannel:
            return fragmentShaderSourceBTransparent;
        case DMHelper::TransparentType_TransparentColor:
            return fragmentShaderSourceTransparentColor;
        default:
            return fragmentShaderSourceBase;
    }
}
