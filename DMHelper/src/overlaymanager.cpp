#include "overlaymanager.h"
#include "overlay.h"
#include "campaign.h"
#include "dmh_opengl.h"
#include "overlay.h"
#include <QMatrix4x4>
#include <QDomDocument>
#include <QDir>
#include <QDebug>

OverlayManager::OverlayManager(Campaign* campaign, QObject* parent) :
    QObject(parent),
    _campaign(campaign),
    _targetSize(),
    _shaderProgramRGB(0),
    _shaderModelMatrixRGB(0),
    _shaderProjectionMatrixRGB(0)
{
}

OverlayManager::~OverlayManager()
{
}

Campaign* OverlayManager::getCampaign() const
{
    return _campaign;
}

void OverlayManager::setCampaign(Campaign* campaign)
{
    if(_campaign == campaign)
        return;

    _campaign = campaign;
    emit updateWindow();
}

void OverlayManager::initializeGL()
{
    qDebug() << "[OverlayManager] initializing overlay GL";

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    const char *vertexShaderSourceRGB = "#version 410 core\n"
                                        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
                                        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
                                        "layout (location = 2) in vec2 aTexCoord;\n"
                                        "uniform mat4 model;\n"
                                        "uniform mat4 view;\n"
                                        "uniform mat4 projection;\n"
                                        "out vec3 ourColor; // output a color to the fragment shader\n"
                                        "out vec2 TexCoord;\n"
                                        "void main()\n"
                                        "{\n"
                                        "   // note that we read the multiplication from right to left\n"
                                        "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                        "   ourColor = aColor; // set ourColor to the input color we got from the vertex data\n"
                                        "   TexCoord = aTexCoord;\n"
                                        "}\0";

    unsigned int vertexShaderRGB;
    vertexShaderRGB = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShaderRGB, 1, &vertexShaderSourceRGB, NULL);
    f->glCompileShader(vertexShaderRGB);

    int  success;
    char infoLog[512];
    f->glGetShaderiv(vertexShaderRGB, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShaderRGB, 512, NULL, infoLog);
        qDebug() << "[OverlayManager] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSourceRGB = "#version 410 core\n"
                                          "out vec4 FragColor;\n"
                                          "in vec3 ourColor;\n"
                                          "in vec2 TexCoord;\n"
                                          "uniform sampler2D texture1;\n"
                                          "void main()\n"
                                          "{\n"
                                          "    FragColor = texture(texture1, TexCoord);\n"
                                          "}\0";

    unsigned int fragmentShaderRGB;
    fragmentShaderRGB = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGB, 1, &fragmentShaderSourceRGB, NULL);
    f->glCompileShader(fragmentShaderRGB);

    f->glGetShaderiv(fragmentShaderRGB, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGB, 512, NULL, infoLog);
        qDebug() << "[OverlayManager] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGB = f->glCreateProgram();
    DMH_DEBUG_OPENGL_glCreateProgram(_shaderProgramRGB, "_shaderProgramRGB");

    f->glAttachShader(_shaderProgramRGB, vertexShaderRGB);
    f->glAttachShader(_shaderProgramRGB, fragmentShaderRGB);
    f->glLinkProgram(_shaderProgramRGB);

    f->glGetProgramiv(_shaderProgramRGB, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgramRGB, 512, NULL, infoLog);
        qDebug() << "[OverlayManager] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
    f->glUseProgram(_shaderProgramRGB);
    f->glDeleteShader(vertexShaderRGB);
    f->glDeleteShader(fragmentShaderRGB);
    _shaderModelMatrixRGB = f->glGetUniformLocation(_shaderProgramRGB, "model");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGB, _shaderModelMatrixRGB, "model");
    _shaderProjectionMatrixRGB = f->glGetUniformLocation(_shaderProgramRGB, "projection");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGB, _shaderProjectionMatrixRGB, "projection");

    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGB, f->glGetUniformLocation(_shaderProgramRGB, "view"), "view");
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGB, "view"), 1, GL_FALSE, viewMatrix.constData(), viewMatrix);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGB, "view"), 1, GL_FALSE, viewMatrix.constData());

    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    if(_campaign)
    {
        for(Overlay* overlay : _campaign->getOverlays())
        {
            if(overlay)
            {
                connect(overlay, &Overlay::triggerUpdate, this, &OverlayManager::updateWindow);
                overlay->initializeGL();
            }
        }
    }
}

void OverlayManager::resizeGL(int w, int h)
{
    if(_targetSize == QSize(w, h))
        return;

    qDebug() << "[PublishGLFrame] Resize w: " << w << ", h: " << h;
    _targetSize = QSize(w, h);

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(f)
    {
        DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
        f->glUseProgram(_shaderProgramRGB);

        QMatrix4x4 screenCoords;
        screenCoords.ortho(0.f, _targetSize.width(), 0.f, _targetSize.height(), 0.1f, 1000.f);
        DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, screenCoords.constData(), screenCoords);
        f->glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, screenCoords.constData());
    }

    if(_campaign)
    {
        for(Overlay* overlay : _campaign->getOverlays())
        {
            if(overlay)
            {
                overlay->resizeGL(w, h);
            }
        }
    }
}

void OverlayManager::paintGL()
{
    if((!_shaderProgramRGB) || (_targetSize.isNull()) || (!_campaign) || (_campaign->getOverlays().isEmpty()))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
    f->glUseProgram(_shaderProgramRGB);

    int yOffset = 0;

    for(Overlay* overlay : _campaign->getOverlays())
    {
        if(overlay)
        {
            if(!overlay->isInitialized())
            {
                connect(overlay, &Overlay::triggerUpdate, this, &OverlayManager::updateWindow);
                overlay->initializeGL();
                overlay->resizeGL(_targetSize.width(), _targetSize.height());
            }
            overlay->paintGL(f, _targetSize, _shaderModelMatrixRGB, yOffset);
            yOffset += overlay->getSize().height();
        }
    }
}
