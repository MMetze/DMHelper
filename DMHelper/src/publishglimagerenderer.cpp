#include "publishglimagerenderer.h"
#include "publishglbattlebackground.h"
#include "dmh_opengl.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>

PublishGLImageRenderer::PublishGLImageRenderer(CampaignObjectBase* renderObject, const QImage& image, QColor color, QObject *parent) :
    PublishGLRenderer(parent),
    _renderObject(renderObject),
    _image(image),
    _color(color),
    _scene(),
    _initialized(false),
    _newProjection(false),
    _shaderProgram(0),
    _imageGLObject(nullptr)
{
}

PublishGLImageRenderer::~PublishGLImageRenderer()
{
    PublishGLImageRenderer::cleanupGL();
}

QColor PublishGLImageRenderer::getBackgroundColor()
{
    return _color;
}

bool PublishGLImageRenderer::deleteOnDeactivation()
{
    return true;
}

void PublishGLImageRenderer::setBackgroundColor(const QColor& color)
{
    _color = color;
    emit updateWidget();
}

void PublishGLImageRenderer::initializeGL()
{
    if((_initialized) || (!_targetWidget))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    //f->glEnable(GL_TEXTURE_2D); // Enable texturing
    //f->glEnable(GL_BLEND);// you enable blending function
    //f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const char *vertexShaderSource = "#version 410 core\n"
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
        qDebug() << "[PublishGLImageRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSource = "#version 410 core\n"
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
        qDebug() << "[PublishGLImageRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();
    DMH_DEBUG_OPENGL_glCreateProgram(_shaderProgram, "_shaderProgram");

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        qDebug() << "[PublishGLImageRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgram);
    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);

    // Create the objects
    _scene.deriveSceneRectFromSize(_image.size());
    _imageGLObject = new PublishGLBattleBackground(nullptr, _image, GL_NEAREST);

    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, f->glGetUniformLocation(_shaderProgram, "model"), "model");
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, modelMatrix.constData(), modelMatrix);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, modelMatrix.constData());
    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, f->glGetUniformLocation(_shaderProgram, "view"), "view");
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData(), viewMatrix);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());
    // Projection - note, this is set later when resizing the window
    _newProjection = true;

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgram);
    f->glUseProgram(_shaderProgram);
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, f->glGetUniformLocation(_shaderProgram, "texture1"), "texture1");
    DMH_DEBUG_OPENGL_glUniform1i(f->glGetUniformLocation(_shaderProgram, "texture1"), 0); // set it manually
    f->glUniform1i(f->glGetUniformLocation(_shaderProgram, "texture1"), 0); // set it manually

    _initialized = true;
}

void PublishGLImageRenderer::cleanupGL()
{
    _initialized = false;

    delete _imageGLObject;
    _imageGLObject = nullptr;

    PublishGLRenderer::cleanupGL();
}

void PublishGLImageRenderer::resizeGL(int w, int h)
{
    _scene.setTargetSize(QSize(w, h));
    qDebug() << "[PublishGLImageRenderer] Resize w: " << w << ", h: " << h;

    _newProjection = true;
    emit updateWidget();
}

void PublishGLImageRenderer::paintGL()
{
    if((!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    DMH_DEBUG_OPENGL_PAINTGL();

    // Draw the scene:
    f->glClearColor(_color.redF(), _color.greenF(), _color.blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgram);
    f->glUseProgram(_shaderProgram);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    if(_newProjection)
    {
        updateProjectionMatrix();
        _newProjection = false;
    }

    if(_imageGLObject)
    {
        DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgram, f->glGetUniformLocation(_shaderProgram, "model"), "model");
        DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, _imageGLObject->getMatrixData(), _imageGLObject->getMatrix());
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, _imageGLObject->getMatrixData());
        _imageGLObject->paintGL(f, nullptr);
    }
}

const QImage& PublishGLImageRenderer::getImage() const
{
    return _image;
}

QColor PublishGLImageRenderer::getColor() const
{
    return _color;
}

void PublishGLImageRenderer::updateProjectionMatrix()
{
    if((_shaderProgram == 0) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    // Update projection matrix and other size related settings:
    QSizeF rectSize = QSizeF(_scene.getTargetSize()).scaled(_scene.getSceneRect().size(), Qt::KeepAspectRatioByExpanding);
    QMatrix4x4 projectionMatrix;
    projectionMatrix.ortho(-rectSize.width() / 2, rectSize.width() / 2, -rectSize.height() / 2, rectSize.height() / 2, 0.1f, 1000.f);
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, projectionMatrix.constData(), projectionMatrix);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, projectionMatrix.constData());

    if(_imageGLObject)
    {
        QPoint pointTopLeft = _scene.getSceneRect().toRect().topLeft();
        _imageGLObject->setPosition(QPoint(pointTopLeft.x(), -pointTopLeft.y()));
    }
}

void PublishGLImageRenderer::setImage(const QImage& image)
{
    if(image != _image)
    {
        _image = image;

        if(_imageGLObject)
        {
            _imageGLObject->setImage(image);
            _newProjection = true;
            emit updateWidget();
        }
    }
}

/*
void PublishGLImageRenderer::setColor(QColor color)
{
    _color = color;
    emit updateWidget();
}
*/
