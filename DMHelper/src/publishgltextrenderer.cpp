#include "publishgltextrenderer.h"
#include "encountertext.h"
#include "publishglbattlebackground.h"
#include "publishglimage.h"
#include "layer.h"
#include "dmconstants.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTextDocument>
#include <QPainter>

PublishGLTextRenderer::PublishGLTextRenderer(EncounterText* encounter, QImage textImage, const QSize& frameSize, QObject *parent) :
    PublishGLRenderer(parent),
    _encounter(encounter),
    _frameSize(frameSize),
    _targetSize(),
    _color(),
    _textImage(textImage),
    _scene(),
    _initialized(false),
    _shaderProgramRGB(0),
    _shaderModelMatrixRGB(0),
    _shaderProjectionMatrixRGB(0),
    _shaderProgramRGBA(0),
    _shaderModelMatrixRGBA(0),
    _shaderProjectionMatrixRGBA(0),
    _shaderAlphaRGBA(0),
    _shaderProgramRGBColor(0),
    _shaderModelMatrixRGBColor(0),
    _shaderProjectionMatrixRGBColor(0),
    _shaderRGBColor(0),
    _projectionMatrix(),
    _scissorRect(),
    _textObject(nullptr),
    _textPos(),
    _elapsed(),
    _timerId(0),
    _recreateContent(false)
{
    if(_encounter)
    {
        connect(&_encounter->getLayerScene(), &LayerScene::layerAdded, this, &PublishGLTextRenderer::layerAdded);
        connect(&_encounter->getLayerScene(), &LayerScene::layerRemoved, this, &PublishGLRenderer::updateWidget);
        connect(&_encounter->getLayerScene(), &LayerScene::layerVisibilityChanged, this, &PublishGLRenderer::updateWidget);
    }
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

void PublishGLTextRenderer::rendererDeactivated()
{
    if(_encounter)
    {
        disconnect(&_encounter->getLayerScene(), &LayerScene::layerAdded, this, &PublishGLTextRenderer::layerAdded);
        disconnect(&_encounter->getLayerScene(), &LayerScene::layerRemoved, this, &PublishGLRenderer::updateWidget);
        disconnect(&_encounter->getLayerScene(), &LayerScene::layerVisibilityChanged, this, &PublishGLRenderer::updateWidget);
    }

    PublishGLRenderer::rendererDeactivated();
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
    if((_initialized) || (!_targetWidget) || (!_targetWidget->context()) || (!_encounter))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    qDebug() << "[PublishGLTextRenderer] Initializing renderer";

    createShaders();
    _encounter->getLayerScene().playerSetShaders(_shaderProgramRGB, _shaderModelMatrixRGB, _shaderProjectionMatrixRGB, _shaderProgramRGBA, _shaderModelMatrixRGBA, _shaderProjectionMatrixRGBA, _shaderAlphaRGBA);

    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    if(_encounter->getLayerScene().sceneSize().isEmpty())
        _scene.deriveSceneRectFromSize(_frameSize);
    else
        _scene.deriveSceneRectFromSize(_encounter->getLayerScene().sceneSize());

    // Create the objects
    _encounter->getLayerScene().playerGLInitialize(this, &_scene);
    _recreateContent = true;

    QMatrix4x4 modelMatrix;
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    f->glUseProgram(_shaderProgramRGBColor);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGBColor, "texture1"), 0); // set it manually
    f->glUniformMatrix4fv(_shaderModelMatrixRGBColor, 1, GL_FALSE, modelMatrix.constData());
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBColor, "view"), 1, GL_FALSE, viewMatrix.constData());

    f->glUseProgram(_shaderProgramRGBA);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), 0); // set it manually
    f->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBA, "view"), 1, GL_FALSE, viewMatrix.constData());

    f->glUseProgram(_shaderProgramRGB);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGB, "texture1"), 0); // set it manually
    f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, modelMatrix.constData());
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGB, "view"), 1, GL_FALSE, viewMatrix.constData());

    // Projection - note, this is set later when resizing the window
    updateProjectionMatrix();

    _initialized = true;

    rewind();
    play();
}

void PublishGLTextRenderer::cleanupGL()
{
    _initialized = false;

    stop();

    delete _textObject;
    _textObject = nullptr;

    if(_encounter)
        _encounter->getLayerScene().playerGLUninitialize();

    _projectionMatrix.setToIdentity();

    destroyShaders();

    PublishGLRenderer::cleanupGL();
}

void PublishGLTextRenderer::resizeGL(int w, int h)
{
    _targetSize = QSize(w, h);
    qDebug() << "[PublishGLTextRenderer] Resize w: " << w << ", h: " << h;

    _scene.setTargetSize(_targetSize);
    if(_encounter)
        _encounter->getLayerScene().playerGLResize(w, h);

    updateProjectionMatrix();

    emit updateWidget();
}

void PublishGLTextRenderer::paintGL()
{
    if((!_initialized) || (!_encounter) || (!_targetSize.isValid()) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    if(_encounter->getLayerScene().playerGLUpdate())
        updateProjectionMatrix();

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    if(_recreateContent)
        recreateContent();

    if(!_scissorRect.isEmpty())
    {
        f->glEnable(GL_SCISSOR_TEST);
        f->glScissor(_scissorRect.x(), _scissorRect.y(), _scissorRect.width(), _scissorRect.height());
    }

    // Draw the scene:
    f->glClearColor(_color.redF(), _color.greenF(), _color.blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f->glUseProgram(_shaderProgramRGB);
    f->glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, _projectionMatrix.constData());
    _encounter->getLayerScene().playerGLPaint(f, _shaderProgramRGB, _shaderModelMatrixRGB, _projectionMatrix.constData());
    f->glUseProgram(_shaderProgramRGB);

    if(_textObject)
    {
        f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _textObject->getMatrixData());
        _textObject->paintGL();
    }

    if(!_scissorRect.isEmpty())
        f->glDisable(GL_SCISSOR_TEST);
}

void PublishGLTextRenderer::updateProjectionMatrix()
{
    if((_shaderProgramRGB == 0) || (!_targetSize.isValid()) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    QSizeF transformedTarget = _targetSize;
    if((_rotation == 90) || (_rotation == 270))
        transformedTarget.transpose();

    // Update projection matrix and other size related settings:
    QSizeF rectSize = transformedTarget.scaled(_scene.getSceneRect().size(), Qt::KeepAspectRatioByExpanding);
    if(rectSize.isEmpty())
        qDebug() << "[PublishGLTextRenderer] updateProjectionMatrix ERROR -publish scene size empty!";

    _projectionMatrix.setToIdentity();
    _projectionMatrix.rotate(_rotation, 0.0, 0.0, -1.0);
    _projectionMatrix.ortho(-rectSize.width() / 2, rectSize.width() / 2, -rectSize.height() / 2, rectSize.height() / 2, 0.1f, 1000.f);
    //_projectionMatrix.ortho(0.0, 0.0, -rectSize.height(), rectSize.height(), 0.1f, 1000.f);
    //_projectionMatrix.ortho(0.0, rectSize.width(), -rectSize.height(), 0.0, 0.1f, 1000.f);

    QSizeF transformedBackgroundSize = _scene.getSceneRect().size(); //_encounter->getLayerScene().sceneSize();
    if(transformedBackgroundSize.isEmpty())
        qDebug() << "[PublishGLTextRenderer] updateProjectionMatrix ERROR - background size empty!";

    if((_rotation == 90) || (_rotation == 270))
        transformedBackgroundSize.transpose();

    QSizeF scissorSize = transformedBackgroundSize.scaled(_targetSize, Qt::KeepAspectRatio);
    _scissorRect.setX((_targetSize.width() - scissorSize.width()) / 2.0);
    _scissorRect.setY((_targetSize.height() - scissorSize.height()) / 2.0);
    _scissorRect.setWidth(scissorSize.width());
    _scissorRect.setHeight(scissorSize.height());
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

void PublishGLTextRenderer::layerAdded(Layer* layer)
{
    if((!layer) || (!_initialized))
        return;

    layer->playerSetShaders(_shaderProgramRGB, _shaderModelMatrixRGB, _shaderProjectionMatrixRGB, _shaderProgramRGBA, _shaderModelMatrixRGBA, _shaderProjectionMatrixRGBA, _shaderAlphaRGBA);
    emit updateWidget();
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

/*
void PublishGLTextRenderer::updateBackground()
{
}
*/

int PublishGLTextRenderer::getRotatedHeight(int rotation)
{
    if((rotation == 90) || (rotation == 270))
        return _encounter->getLayerScene().sceneSize().width();
    else
        return _encounter->getLayerScene().sceneSize().height();
}

void PublishGLTextRenderer::recreateContent()
{
    if((!_encounter) || (!_encounter->getLayerScene().sceneSize().isValid()))
        return;

    delete _textObject;
    _textObject = new PublishGLImage(_textImage, GL_NEAREST, false);
    _textObject->setX(-_scene.getSceneRect().width() / 2);

    rewind();

    _recreateContent = false;
}

void PublishGLTextRenderer::createShaders()
{
    int  success;
    char infoLog[512];

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

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

    f->glGetShaderiv(vertexShaderRGB, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShaderRGB, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
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

    //    "    FragColor = texture(texture1, TexCoord); // FragColor = vec4(ourColor, 1.0f);\n"
    //    "    FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"

    unsigned int fragmentShaderRGB;
    fragmentShaderRGB = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGB, 1, &fragmentShaderSourceRGB, NULL);
    f->glCompileShader(fragmentShaderRGB);

    f->glGetShaderiv(fragmentShaderRGB, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGB, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGB = f->glCreateProgram();

    f->glAttachShader(_shaderProgramRGB, vertexShaderRGB);
    f->glAttachShader(_shaderProgramRGB, fragmentShaderRGB);
    f->glLinkProgram(_shaderProgramRGB);

    f->glGetProgramiv(_shaderProgramRGB, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgramRGB, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgramRGB);
    f->glDeleteShader(vertexShaderRGB);
    f->glDeleteShader(fragmentShaderRGB);
    _shaderModelMatrixRGB = f->glGetUniformLocation(_shaderProgramRGB, "model");
    _shaderProjectionMatrixRGB = f->glGetUniformLocation(_shaderProgramRGB, "projection");

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
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
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
        "}\0";

    //   "    FragColor = texture(texture1, TexCoord) * ourColor; // FragColor = vec4(ourColor, 1.0f);\n"
    //    "    FragColor = texture(texture1, TexCoord); // FragColor = vec4(ourColor, 1.0f);\n"
    //    "    FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"

    unsigned int fragmentShaderRGBA;
    fragmentShaderRGBA = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGBA, 1, &fragmentShaderSourceRGBA, NULL);
    f->glCompileShader(fragmentShaderRGBA);

    f->glGetShaderiv(fragmentShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGBA = f->glCreateProgram();

    f->glAttachShader(_shaderProgramRGBA, vertexShaderRGBA);
    f->glAttachShader(_shaderProgramRGBA, fragmentShaderRGBA);
    f->glLinkProgram(_shaderProgramRGBA);

    f->glGetProgramiv(_shaderProgramRGBA, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgramRGBA, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgramRGBA);
    f->glDeleteShader(vertexShaderRGBA);
    f->glDeleteShader(fragmentShaderRGBA);
    _shaderModelMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "model");
    _shaderProjectionMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "projection");
    _shaderAlphaRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "alpha");

    const char *vertexShaderSourceRGBColor = "#version 410 core\n"
        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "uniform vec4 inColor;\n"
        "out vec4 ourColor; // output a color to the fragment shader\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "   // note that we read the multiplication from right to left\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   ourColor = inColor; // set ourColor to the input color we got from the vertex data\n"
        "   TexCoord = aTexCoord;\n"
        "}\0";

    unsigned int vertexShaderRGBColor;
    vertexShaderRGBColor = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShaderRGBColor, 1, &vertexShaderSourceRGBColor, NULL);
    f->glCompileShader(vertexShaderRGBColor);

    f->glGetShaderiv(vertexShaderRGBColor, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShaderRGBColor, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSourceRGBColor = "#version 410 core\n"
        "out vec4 FragColor;\n"
        "in vec4 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = ourColor;\n"
        "}\0";


    unsigned int fragmentShaderRGBColor;
    fragmentShaderRGBColor = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGBColor, 1, &fragmentShaderSourceRGBColor, NULL);
    f->glCompileShader(fragmentShaderRGBColor);

    f->glGetShaderiv(fragmentShaderRGBColor, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGBColor, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGBColor = f->glCreateProgram();

    f->glAttachShader(_shaderProgramRGBColor, vertexShaderRGBColor);
    f->glAttachShader(_shaderProgramRGBColor, fragmentShaderRGBColor);
    f->glLinkProgram(_shaderProgramRGBColor);

    f->glGetProgramiv(_shaderProgramRGBColor, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgramRGBColor, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgramRGBColor);
    f->glDeleteShader(vertexShaderRGBColor);
    f->glDeleteShader(fragmentShaderRGBColor);
    _shaderModelMatrixRGBColor = f->glGetUniformLocation(_shaderProgramRGBColor, "model");
    _shaderProjectionMatrixRGBColor = f->glGetUniformLocation(_shaderProgramRGBColor, "projection");
    _shaderRGBColor = f->glGetUniformLocation(_shaderProgramRGBColor, "inColor");
}

void PublishGLTextRenderer::destroyShaders()
{
    if((_targetWidget) && (_targetWidget->context()))
    {
        QOpenGLFunctions *f = _targetWidget->context()->functions();
        if(f)
        {
            if(_shaderProgramRGB > 0)
                f->glDeleteProgram(_shaderProgramRGB);
            if(_shaderProgramRGBA > 0)
                f->glDeleteProgram(_shaderProgramRGBA);
            if(_shaderProgramRGBColor > 0)
                f->glDeleteProgram(_shaderProgramRGBColor);
        }
    }

    _shaderProgramRGB = 0;
    _shaderModelMatrixRGB = 0;
    _shaderProjectionMatrixRGB = 0;
    _shaderProgramRGBA = 0;
    _shaderModelMatrixRGBA = 0;
    _shaderProjectionMatrixRGBA = 0;
    _shaderAlphaRGBA = 0;
    _shaderProgramRGBColor = 0;
    _shaderModelMatrixRGBColor = 0;
    _shaderProjectionMatrixRGBColor = 0;
    _shaderRGBColor = 0;
}
