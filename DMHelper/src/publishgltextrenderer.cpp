#include "publishgltextrenderer.h"
#include "encountertext.h"
#include "battleglbackground.h"
#include "dmconstants.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTextDocument>
#include <QPainter>

PublishGLTextRenderer::PublishGLTextRenderer(EncounterText* encounter, QImage backgroundImage, QImage textImage, QObject *parent) :
    PublishGLRenderer(parent),
    _encounter(encounter),
    _targetSize(),
    _backgroundImage(backgroundImage),
    _textImage(textImage),
    _scene(),
    _initialized(false),
    _shaderProgram(0),
    _backgroundObject(nullptr),
    _textObject(nullptr),
    _textPos(),
    _elapsed(),
    _timerId(0)
{
}

PublishGLTextRenderer::~PublishGLTextRenderer()
{
    cleanup();
}

CampaignObjectBase* PublishGLTextRenderer::getObject()
{
    return _encounter;
}

void PublishGLTextRenderer::cleanup()
{
    _initialized = false;

    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    delete _backgroundObject;
    _backgroundObject = nullptr;
    delete _textObject;
    _textObject = nullptr;
}

void PublishGLTextRenderer::initializeGL()
{
    if((_initialized) || (!_targetWidget))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

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
        qDebug() << "[PublishGLTextRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
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
        qDebug() << "[PublishGLTextRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        qDebug() << "[PublishGLTextRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);

    // Create the objects
    _scene.deriveSceneRectFromSize(_backgroundImage.size());
    _backgroundObject = new BattleGLBackground(nullptr, _backgroundImage, GL_NEAREST);
    _textObject = new BattleGLBackground(nullptr, _textImage, GL_LINEAR);

    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, modelMatrix.constData());
    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());
    // Projection - note, this is set later when resizing the window
    setOrthoProjection();

    f->glUseProgram(_shaderProgram);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgram, "texture1"), 0); // set it manually

    _initialized = true;

    rewind();
    _elapsed.start();
    _timerId = startTimer(DMHelper::ANIMATION_TIMER_DURATION, Qt::PreciseTimer);
}

void PublishGLTextRenderer::resizeGL(int w, int h)
{
    _targetSize = QSize(w, h);
    _scene.setTargetSize(_targetSize);
    qDebug() << "[PublishGLTextRenderer] Resize w: " << w << ", h: " << h;

    setOrthoProjection();
    emit updateWidget();
}

void PublishGLTextRenderer::paintGL()
{
    if((!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    // Draw the scene:
    //f->glClearColor(_color.redF(), _color.greenF(), _color.blueF(), 1.0f);
    f->glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f->glUseProgram(_shaderProgram);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    if(_backgroundObject)
    {
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, _backgroundObject->getMatrixData());
        _backgroundObject->paintGL();
    }

    if(_textObject)
    {
        QMatrix4x4 textMatrix = _textObject->getMatrix();
        textMatrix.translate(0, _textPos.y());
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, textMatrix.constData() );
        _textObject->paintGL();
    }
}

void PublishGLTextRenderer::rewind()
{
    //_textPos.setY(_backgroundImage.height());
    _textPos.setY(0.0);
}

void PublishGLTextRenderer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    qreal elapsedtime = _elapsed.restart();
    //_textPos.ry() -= _encounter->getScrollSpeed() * (_prescaledImage.height() / 250) * (elapsedtime / 1000.0);
    _textPos.ry() += 25.0 * (_backgroundImage.height() / 250) * (elapsedtime / 1000.0);

    emit updateWidget();
}

void PublishGLTextRenderer::setOrthoProjection()
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
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, projectionMatrix.constData());
}

