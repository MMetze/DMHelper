#include "publishglframe.h"
#include "publishglrenderer.h"
#include "publishglimagerenderer.h"
#include <QOpenGLFunctions>
#include <QDebug>

PublishGLFrame::PublishGLFrame(QWidget *parent) :
    QOpenGLWidget(parent),
    _initialized(false),
    _targetSize(),
    _renderer(nullptr),
    _shaderProgram(0)
{
}

PublishGLFrame::~PublishGLFrame()
{
    cleanup();
    setRenderer(nullptr);
}

bool PublishGLFrame::isInitialized() const
{
    return _initialized;
}

void PublishGLFrame::cleanup()
{
    _initialized = false;

    if(_renderer)
        _renderer->cleanup();

    if(_shaderProgram > 0)
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        if(f)
        {
            f->glDeleteProgram(_shaderProgram);
            _shaderProgram = 0;
        }
    }
}

void PublishGLFrame::updateWidget()
{
    update();
}

void PublishGLFrame::setRenderer(PublishGLRenderer* renderer)
{
    if(_renderer)
    {
        _renderer->rendererDeactivated();
        disconnect(_renderer, &PublishGLRenderer::updateWidget, this, &PublishGLFrame::updateWidget);
        if(_renderer->deleteOnDeactivation())
            _renderer->deleteLater();
    }

    _renderer = renderer;
    if(_renderer)
    {
        connect(_renderer, &PublishGLRenderer::updateWidget, this, &PublishGLFrame::updateWidget);
        connect(_renderer, &PublishGLRenderer::destroyed, this, &PublishGLFrame::clearRenderer);
        _renderer->rendererActivated(this);
        if(isInitialized())
        {
            _renderer->initializeGL();
            _renderer->resizeGL(_targetSize.width(), _targetSize.height());
            updateWidget();
        }
    }
}

void PublishGLFrame::clearRenderer()
{
    _renderer = nullptr;
}

void PublishGLFrame::setBackgroundColor(const QColor& color)
{
    if(_renderer)
        _renderer->setBackgroundColor(color);
}

void PublishGLFrame::setImage(QImage img, QColor color)
{
    PublishGLImageRenderer* imageRenderer = dynamic_cast<PublishGLImageRenderer*>(_renderer);
    if(imageRenderer)
    {
        imageRenderer->setBackgroundColor(color);
        imageRenderer->setImage(img);
    }
    else
    {
        setRenderer(new PublishGLImageRenderer(img, color));
    }
}

void PublishGLFrame::setImageNoScale(QImage img, QColor color)
{
    setImage(img, color);
}

void PublishGLFrame::setArrowVisible(bool visible)
{

}

void PublishGLFrame::setArrowPosition(const QPointF& position)
{

}

void PublishGLFrame::setPointerFile(const QString& filename)
{

}

void PublishGLFrame::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLFrame::cleanup);

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    /*
    f->glEnable(GL_TEXTURE_2D); // Enable texturing
    f->glEnable(GL_BLEND);// you enable blending function
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const char *vertexShaderSource = "#version 330 core\n"
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

    unsigned int vertexShader;
    vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    f->glCompileShader(vertexShader);

    int  success;
    char infoLog[512];
    f->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "[PublishGLFrame] ERROR: Vertex shader compilation failed: " << infoLog;
        return;
    }

    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(texture1, TexCoord); // FragColor = vec4(ourColor, 1.0f);\n"
        "}\0";

    unsigned int fragmentShader;
    fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    f->glCompileShader(fragmentShader);

    f->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "[PublishGLFrame] ERROR: Fragment shader compilation failed: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        qDebug() << "[PublishGLFrame] ERROR: Shader program compilation failed: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);
    */

    _initialized = true;

    if(_renderer)
        _renderer->initializeGL();
}

void PublishGLFrame::resizeGL(int w, int h)
{
    qDebug() << "[PublishGLFrame] Resize w: " << w << ", h: " << h;
    _targetSize = QSize(w, h);
    //setOrthoProjection();

    if(_renderer)
        _renderer->resizeGL(w, h);

    emit frameResized(_targetSize);
}

void PublishGLFrame::paintGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    //f->glUseProgram(_shaderProgram);

    if(_renderer)
        _renderer->paintGL();
}

void PublishGLFrame::setOrthoProjection()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

}
