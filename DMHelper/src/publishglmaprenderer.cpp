#include "publishglmaprenderer.h"
#include "map.h"
#include "videoplayerglplayer.h"
#include "battleglbackground.h"
#include "publishglobject.h"
#include "publishglimage.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QDebug>

PublishGLMapRenderer::PublishGLMapRenderer(Map* map, QObject *parent) :
    PublishGLRenderer(parent),
    _map(map),
    _image(),
    _videoPlayer(nullptr),
    _targetSize(),
    _color(),
    _initialized(false),
    _shaderProgram(0),
    _backgroundObject(nullptr),
    _partyToken(nullptr)
{
}

PublishGLMapRenderer::~PublishGLMapRenderer()
{
    cleanup();
}

void PublishGLMapRenderer::cleanup()
{
    _initialized = false;

    delete _partyToken;
    _partyToken = nullptr;

    delete _backgroundObject;
    _backgroundObject = nullptr;

    delete _videoPlayer;
    _videoPlayer = nullptr;
}

bool PublishGLMapRenderer::deleteOnDeactivation()
{
    return true;
}

void PublishGLMapRenderer::setBackgroundColor(const QColor& color)
{
    _color = color;
    emit updateWidget();
}

void PublishGLMapRenderer::initializeGL()
{
    if((_initialized) || (!_targetWidget) || (!_map))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

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
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
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
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);

    // Create the objects
    _image.load(QString("C:\\Users\\turne\\Documents\\DnD\\DM Helper\\testdata\\Desert Stronghold.jpg"));
    _backgroundObject = new BattleGLBackground(nullptr, _image, GL_NEAREST);

    // Create the party token
    if(_map->getShowParty())
    {
        QImage partyImage = _map->getPartyPixmap().toImage();
        _partyToken = new PublishGLImage(partyImage, false);
        _partyToken->setScale(0.04f * static_cast<float>(_map->getPartyScale()));
    }

    // Create the objects
    _videoPlayer = new VideoPlayerGLPlayer(_map->getFileName(),
                                           _targetWidget->context(),
                                           _targetWidget->format(),
                                           _targetSize,
                                           true,
                                           false);
    connect(_videoPlayer, &VideoPlayerGLPlayer::frameAvailable, this, &PublishGLMapRenderer::updateWidget);

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
}

void PublishGLMapRenderer::resizeGL(int w, int h)
{
    _targetSize = QSize(w, h);
    qDebug() << "[PublishGLMapRenderer] Resize w: " << w << ", h: " << h;
    setOrthoProjection();
    if(_videoPlayer)
    {
        _videoPlayer->targetResized(_targetSize);
        _videoPlayer->initializationComplete();
    }
    emit updateWidget();
}

void PublishGLMapRenderer::paintGL()
{
    if((!_initialized) || (!_map))
        return;

    if((!_targetWidget) || (!_targetWidget->context()) || (!_videoPlayer))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    // Draw the scene:
    f->glClearColor(_color.redF(), _color.greenF(), _color.blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f->glUseProgram(_shaderProgram);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, modelMatrix.constData());
    _videoPlayer->paintGL();

    if(_partyToken)
    {
        QSize sceneSize = _videoPlayer->getSize();
        _partyToken->setPosition(_map->getPartyIconPos().x() - (sceneSize.width() / 2), (sceneSize.height() / 2) - _map->getPartyIconPos().y());
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, _partyToken->getMatrixData());
        _partyToken->paintGL();
    }
    /*
    if(_backgroundObject)
    {
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, _backgroundObject->getMatrixData());
        _backgroundObject->paintGL();
    }
    */

}

QImage PublishGLMapRenderer::getLastScreenshot()
{
    if(!_videoPlayer)
        return QImage();

    return _videoPlayer->getLastScreenshot();
}

const QImage& PublishGLMapRenderer::getImage() const
{
    return _image;
}

QColor PublishGLMapRenderer::getColor() const
{
    return _color;
}

void PublishGLMapRenderer::setImage(const QImage& image)
{
    if(image != _image)
    {
        _image = image;

        // TBD
        /*
        if(_backgroundObject)
        {
            _backgroundObject->setImage(image);
            setOrthoProjection();
            emit updateWidget();
        }
        */
    }
}
/*
void PublishGLMapRenderer::setColor(QColor color)
{
    _color = color;
    emit updateWidget();
}*/

void PublishGLMapRenderer::setOrthoProjection()
{
    if((_shaderProgram == 0) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    // Update projection matrix and other size related settings:
    /*
    QSizeF rectSize = QSizeF(_scene.getTargetSize()).scaled(_scene.getSceneRect().size(), Qt::KeepAspectRatioByExpanding);
    QMatrix4x4 projectionMatrix;
    projectionMatrix.ortho(-rectSize.width() / 2, rectSize.width() / 2, -rectSize.height() / 2, rectSize.height() / 2, 0.1f, 1000.f);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, projectionMatrix.constData());
    */

    QMatrix4x4 projectionMatrix;
    projectionMatrix.ortho(-_targetSize.width() / 2, _targetSize.width() / 2, -_targetSize.height() / 2, _targetSize.height() / 2, 0.1f, 1000.f);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, projectionMatrix.constData());
}
