#include "publishgltextrenderer.h"
#include "encountertext.h"
#include "publishglbattlebackground.h"
#include "publishglimage.h"
#include "dmconstants.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTextDocument>
#include <QPainter>

PublishGLTextRenderer::PublishGLTextRenderer(EncounterText* encounter, QImage textImage, QObject *parent) :
    PublishGLRenderer(parent),
    _encounter(encounter),
    _targetSize(),
    _color(),
    _textImage(textImage),
    _scene(),
    _initialized(false),
    _shaderProgram(0),
    _shaderModelMatrix(0),
    _shaderProjectionMatrix(0),
    _projectionMatrix(),
    _scissorRect(),
    _textObject(nullptr),
    _textPos(),
    _elapsed(),
    _timerId(0),
    _recreateContent(false)
{
}

PublishGLTextRenderer::~PublishGLTextRenderer()
{
}

CampaignObjectBase* PublishGLTextRenderer::getObject()
{
    return _encounter;
}

QColor PublishGLTextRenderer::getBackgroundColor()
{
    return _color;
}

void PublishGLTextRenderer::cleanup()
{
    _initialized = false;

    stop();

    delete _textObject;
    _textObject = nullptr;

    _projectionMatrix.setToIdentity();

    if(_shaderProgram > 0)
    {
        if((_targetWidget) && (_targetWidget->context()))
        {
            QOpenGLFunctions *f = _targetWidget->context()->functions();
            if(f)
                f->glDeleteProgram(_shaderProgram);
        }
        _shaderProgram = 0;
    }
    _shaderModelMatrix = 0;
    _shaderProjectionMatrix = 0;

    PublishGLRenderer::cleanup();
}

bool PublishGLTextRenderer::deleteOnDeactivation()
{
    return true;
}

QRect PublishGLTextRenderer::getScissorRect()
{
    return _scissorRect;
}

void PublishGLTextRenderer::setBackgroundColor(const QColor& color)
{
    _color = color;
    emit updateWidget();
}

void PublishGLTextRenderer::initializeGL()
{
    if((_initialized) || (!_targetWidget))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

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
        qDebug() << "[PublishGLTextRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
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
    _shaderModelMatrix = f->glGetUniformLocation(_shaderProgram, "model");
    _shaderProjectionMatrix = f->glGetUniformLocation(_shaderProgram, "projection");

    // Create the objects
    initializeBackground();
    _scene.deriveSceneRectFromSize(getBackgroundSize());
    _recreateContent = isBackgroundReady();

    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());
    // Projection - note, this is set later when resizing the window
    updateProjectionMatrix();

    f->glUseProgram(_shaderProgram);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgram, "texture1"), 0); // set it manually

    _initialized = true;

    rewind();
    play();
}

void PublishGLTextRenderer::resizeGL(int w, int h)
{
    _targetSize = QSize(w, h);
    qDebug() << "[PublishGLTextRenderer] Resize w: " << w << ", h: " << h;
    resizeBackground(w, h);

    emit updateWidget();
}

void PublishGLTextRenderer::paintGL()
{
    if((!_targetWidget) || (!_targetWidget->context()))
        return;

    if(!isBackgroundReady())
    {
        updateBackground();
        if(!isBackgroundReady())
            return;

        updateProjectionMatrix();
        _recreateContent = true;
    }

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    if(_recreateContent)
        recreateContent();

    f->glUniformMatrix4fv(_shaderProjectionMatrix, 1, GL_FALSE, _projectionMatrix.constData());

    if(!_scissorRect.isEmpty())
    {
        f->glEnable(GL_SCISSOR_TEST);
        f->glScissor(_scissorRect.x(), _scissorRect.y(), _scissorRect.width(), _scissorRect.height());
    }

    // Draw the scene:
    f->glClearColor(_color.redF(), _color.greenF(), _color.blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f->glUseProgram(_shaderProgram);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    paintBackground(f);

    if(_textObject)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _textObject->getMatrixData());
        _textObject->paintGL();
    }

    if(!_scissorRect.isEmpty())
        f->glDisable(GL_SCISSOR_TEST);
}

void PublishGLTextRenderer::setRotation(int rotation)
{
    if((rotation != _rotation) && (_textObject) && (_encounter) && (!_encounter->getAnimated()))
        _textObject->setY((getRotatedHeight(rotation) / 2) - _textObject->getImageSize().height());

    PublishGLRenderer::setRotation(rotation);
}

void PublishGLTextRenderer::rewind()
{
    if((!_encounter) || (!_textObject))
        return;

    if(_encounter->getAnimated())
    {
        _textObject->setY((-getRotatedHeight(_rotation) / 2) - _textObject->getImageSize().height());
        _elapsed.start();
    }
    else
    {
        _textObject->setY((getRotatedHeight(_rotation) / 2) - _textObject->getImageSize().height());
    }

    emit updateWidget();
}

void PublishGLTextRenderer::play()
{
    startScrollingTimer();
}

void PublishGLTextRenderer::stop()
{
    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
        emit playPauseChanged(false);
    }
}

void PublishGLTextRenderer::playPause(bool play)
{
    if(play)
        PublishGLTextRenderer::play();
    else
        PublishGLTextRenderer::stop();
}

void PublishGLTextRenderer::contentChanged()
{
    _recreateContent = true;
    emit updateWidget();
}

void PublishGLTextRenderer::startScrollingTimer()
{
    if((!_encounter) || (!_encounter->getAnimated()) || (_timerId))
        return;

   _elapsed.start();
   _timerId = startTimer(DMHelper::ANIMATION_TIMER_DURATION, Qt::PreciseTimer);
   emit playPauseChanged(true);
}

void PublishGLTextRenderer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if((!_textObject) || (!_encounter))
        return;

    qreal elapsedtime = _elapsed.restart();
    _textObject->setY(_textObject->getY() + _encounter->getScrollSpeed() * (getRotatedHeight(_rotation) / 250) * (elapsedtime / 1000.0));

    emit updateWidget();
}

void PublishGLTextRenderer::updateProjectionMatrix()
{
    if((_shaderProgram == 0) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    QSizeF transformedTarget = _targetSize;
    if((_rotation == 90) || (_rotation == 270))
        transformedTarget.transpose();

    // Update projection matrix and other size related settings:
    QSizeF rectSize = transformedTarget.scaled(_scene.getSceneRect().size(), Qt::KeepAspectRatioByExpanding);
    _projectionMatrix.setToIdentity();
    _projectionMatrix.rotate(_rotation, 0.0, 0.0, -1.0);
    _projectionMatrix.ortho(-rectSize.width() / 2, rectSize.width() / 2, -rectSize.height() / 2, rectSize.height() / 2, 0.1f, 1000.f);

    QSizeF transformedBackgroundSize = getBackgroundSize();
    if((_rotation == 90) || (_rotation == 270))
        transformedBackgroundSize.transpose();

    QSizeF scissorSize = transformedBackgroundSize.scaled(_targetSize, Qt::KeepAspectRatio);
    _scissorRect.setX((_targetSize.width() - scissorSize.width()) / 2.0);
    _scissorRect.setY((_targetSize.height() - scissorSize.height()) / 2.0);
    _scissorRect.setWidth(scissorSize.width());
    _scissorRect.setHeight(scissorSize.height());
}

void PublishGLTextRenderer::updateBackground()
{
}

int PublishGLTextRenderer::getRotatedHeight(int rotation)
{
    if((rotation == 90) || (rotation == 270))
        return getBackgroundSize().width();
    else
        return getBackgroundSize().height();
}

void PublishGLTextRenderer::recreateContent()
{
    if(getBackgroundSize().isValid())
    {
        delete _textObject;
        _textObject = new PublishGLImage(_textImage, GL_NEAREST, false);
        _textObject->setX(-getBackgroundSize().width() / 2);

        rewind();

        _recreateContent = false;
    }
}
