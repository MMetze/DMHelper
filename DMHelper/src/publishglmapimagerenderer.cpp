#include "publishglmapimagerenderer.h"
#include "map.h"
#include "party.h"
#include "videoplayerglplayer.h"
#include "battleglbackground.h"
#include "publishglobject.h"
#include "publishglimage.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QPainter>
#include <QDebug>

/*
if(_publishZoom)
    _publishRect = QRect(static_cast<int>(static_cast<qreal>(ui->graphicsView->horizontalScrollBar()->value()) / _scale),
                         static_cast<int>(static_cast<qreal>(ui->graphicsView->verticalScrollBar()->value()) / _scale),
                         static_cast<int>(static_cast<qreal>(ui->graphicsView->viewport()->width()) / _scale),
                         static_cast<int>(static_cast<qreal>(ui->graphicsView->viewport()->height()) / _scale));
    // TODO: Consider zoom factor...
else
    if(_publishVisible)
        pub = _mapSource->getShrunkPublishImage(&_publishRect);
    else
        pub = _mapSource->getPublishImage();
        _publishRect = pub.rect();

    QPainter p(&pub);
    QPointF topLeftOffset = (_publishZoom || _publishVisible) ? _publishRect.topLeft() : QPointF();
    if((_mapSource->getShowParty()) && ((_mapSource->getParty()) || (!_mapSource->getPartyAltIcon().isEmpty())))
    {
        QPixmap partyPixmap = _mapSource->getPartyPixmap();
        if(!partyPixmap.isNull())
        {
            qreal partyScale = 0.04 * static_cast<qreal>(_mapSource->getPartyScale());
            p.drawPixmap(_partyIcon->pos() - topLeftOffset, partyPixmap.scaled(partyPixmap.width() * partyScale, partyPixmap.height() * partyScale));
        }
    }

    p.setPen(QPen(QBrush(_mapSource->getDistanceLineColor()),
                  _mapSource->getDistanceLineWidth(),
                  static_cast<Qt::PenStyle>(_mapSource->getDistanceLineType())));
    if(_distanceLine)
        p.drawLine(_distanceLine->line().translated(-topLeftOffset));
    if(_distancePath)
        p.drawPath(_distancePath->path().translated(-topLeftOffset));
    if(_distanceText)
        p.drawText(_distanceText->pos() - topLeftOffset, _distanceText->text());

    if(_mapSource->getShowMarkers())
    {
        if(QUndoStack* stack = _mapSource->getUndoStack())
        {
            for( int i = 0; i < stack->index(); ++i )
            {
                const UndoMarker* markerAction = dynamic_cast<const UndoMarker*>(stack->command(i));
                if((markerAction) && (markerAction->getMarker().isPlayerVisible()))
                {
                    MapMarkerGraphicsItem* markerItem = markerAction->getMarkerItem();
                    if(markerItem)
                        markerItem->drawGraphicsItem(p);
                }
            }
        }
    }
}

if(_rotation != 0)
{
    pub = pub.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
}

emit publishImage(pub);
*/


PublishGLMapImageRenderer::PublishGLMapImageRenderer(Map* map, QObject *parent) :
    PublishGLRenderer(parent),
    _map(map),
    _image(),
    _targetSize(),
    _color(),
    _initialized(false),
    _shaderProgram(0),
    _shaderModelMatrix(0),
    _backgroundObject(nullptr),
    _partyToken(nullptr),
    _lineImage(nullptr),
    _recreatePartyToken(false)
{
    connect(_map, &Map::partyChanged, this, &PublishGLMapImageRenderer::handlePartyChanged);
    connect(_map, &Map::partyIconChanged, this, &PublishGLMapImageRenderer::handlePartyIconChanged);
    connect(_map, &Map::partyIconPosChanged, this, &PublishGLMapImageRenderer::handlePartyIconPosChanged);
    connect(_map, &Map::showPartyChanged, this, &PublishGLMapImageRenderer::handleShowPartyChanged);
    connect(_map, &Map::partyScaleChanged, this, &PublishGLMapImageRenderer::handlePartyScaleChanged);
}

PublishGLMapImageRenderer::~PublishGLMapImageRenderer()
{
    cleanup();
}

CampaignObjectBase* PublishGLMapImageRenderer::getObject()
{
    return _map;
}

void PublishGLMapImageRenderer::cleanup()
{
    _initialized = false;

    delete _lineImage;
    _lineImage = nullptr;

    delete _partyToken;
    _partyToken = nullptr;
    _recreatePartyToken = false;

    delete _backgroundObject;
    _backgroundObject = nullptr;

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
}

bool PublishGLMapImageRenderer::deleteOnDeactivation()
{
    return true;
}

void PublishGLMapImageRenderer::setBackgroundColor(const QColor& color)
{
    _color = color;
    emit updateWidget();
}

void PublishGLMapImageRenderer::initializeGL()
{    
    if((_initialized) || (!_targetWidget) || (!_map) || (!_map->isInitialized()))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    qDebug() << "[PublishGLMapRenderer] Initializing renderer";

    //f->glEnable(GL_TEXTURE_2D); // Enable texturing
    //f->glEnable(GL_BLEND);// you enable blending function
    //f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    _shaderModelMatrix = f->glGetUniformLocation(_shaderProgram, "model");

    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    // Create the objects
    if(_image.isNull())
        _image = _map->getBackgroundImage();
    _backgroundObject = new BattleGLBackground(nullptr, _image, GL_NEAREST);

    // Create the party token
    createPartyToken();

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

    //f->glUseProgram(_shaderProgram);
    //f->glUniform1i(f->glGetUniformLocation(_shaderProgram, "texture1"), 0); // set it manually

    _initialized = true;
}

void PublishGLMapImageRenderer::resizeGL(int w, int h)
{
    _targetSize = QSize(w, h);
    qDebug() << "[PublishGLMapRenderer] Resize w: " << w << ", h: " << h;
    if(_backgroundObject)
        setOrthoProjection();

    emit updateWidget();
}

void PublishGLMapImageRenderer::paintGL()
{
    qDebug() << "[PublishGLMapRenderer] Painting renderer";
    if(!_initialized)
        initializeGL();

    if((!_initialized) || (!_map))
        return;

    if((!_targetWidget) || (!_targetWidget->context()))
        return;

    if(_recreatePartyToken)
        createPartyToken();

    QSize sceneSize;
    if(_backgroundObject)
        sceneSize = _backgroundObject->getSize();

    if(_map->getMapItemCount() > 0)
    {
        MapDrawLine* line = dynamic_cast<MapDrawLine*>(_map->getMapItem(0));
        if((line) &&
                (line->line().p1() != line->line().p2()))

        {
            QSize lSize = line->lineSize();
            QPoint lOrigin = line->origin();
            QLine lOriginLine = line->originLine();


            QImage lineImage(line->lineSize(), QImage::Format_ARGB32_Premultiplied);
            lineImage.fill(Qt::transparent);
            QPainter linePainter;
            linePainter.begin(&lineImage);
                linePainter.setPen(QPen(QBrush(line->penColor()), line->penWidth(), line->penStyle()));
                linePainter.drawLine(line->originLine());
            linePainter.end();

            if(_lineImage)
                _lineImage->setImage(lineImage);
            else
                _lineImage = new PublishGLImage(lineImage, false);

            //_partyToken->setPosition(_map->getPartyIconPos().x() - (sceneSize.width() / 2), (sceneSize.height() / 2) - _map->getPartyIconPos().y() - _partyToken->getSize().height());
            _lineImage->setPosition(line->origin().x() - (sceneSize.width() / 2), (sceneSize.height() / 2) - line->origin().y() - _lineImage->getSize().height());
            //_lineImage->setPosition(line->origin());
        }
    }

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    // Draw the scene:
    f->glClearColor(_color.redF(), _color.greenF(), _color.blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f->glUseProgram(_shaderProgram);

    if(_backgroundObject)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
        _backgroundObject->paintGL();
    }

    if(_lineImage)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _lineImage->getMatrixData());
        _lineImage->paintGL();
    }

    if((_partyToken) && (_map->getShowParty()))
    {
        _partyToken->setPosition(_map->getPartyIconPos().x() - (sceneSize.width() / 2), (sceneSize.height() / 2) - _map->getPartyIconPos().y() - _partyToken->getSize().height());
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _partyToken->getMatrixData());
        _partyToken->paintGL();
    }
}

const QImage& PublishGLMapImageRenderer::getImage() const
{
    return _image;
}

QColor PublishGLMapImageRenderer::getColor() const
{
    return _color;
}

void PublishGLMapImageRenderer::setImage(const QImage& image)
{
    if(image != _image)
    {
        _image = image;

        if(_backgroundObject)
        {
            _backgroundObject->setImage(image);
            createPartyToken();
            setOrthoProjection();
            emit updateWidget();
        }
    }
}
/*
void PublishGLMapRenderer::setColor(QColor color)
{
    _color = color;
    emit updateWidget();
}*/

void PublishGLMapImageRenderer::setOrthoProjection()
{
    if((_shaderProgram == 0) || (!_targetWidget) || (!_targetWidget->context()) || (!_backgroundObject))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    // Update projection matrix and other size related settings:
    QSizeF rectSize = QSizeF(_targetSize).scaled(_backgroundObject->getSize(), Qt::KeepAspectRatioByExpanding);
    QMatrix4x4 projectionMatrix;
    projectionMatrix.ortho(-rectSize.width() / 2, rectSize.width() / 2, -rectSize.height() / 2, rectSize.height() / 2, 0.1f, 1000.f);
    qDebug() << "[PublishGLMapRenderer] Setting orthogonal projection to " << projectionMatrix;
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, projectionMatrix.constData());
}

void PublishGLMapImageRenderer::createPartyToken()
{
    if(_partyToken)
    {
        delete _partyToken;
        _partyToken = nullptr;
    }

    QImage partyImage = _map->getPartyPixmap().toImage();
    if(!partyImage.isNull())
    {
        _partyToken = new PublishGLImage(partyImage, false);
        _partyToken->setScale(0.04f * static_cast<float>(_map->getPartyScale()));
    }

    _recreatePartyToken = false;
}

void PublishGLMapImageRenderer::handlePartyChanged(Party* party)
{
    Q_UNUSED(party);
    _recreatePartyToken = true;
    updateRender();
}

void PublishGLMapImageRenderer::handlePartyIconChanged(const QString& partyIcon)
{
    Q_UNUSED(partyIcon);
    _recreatePartyToken = true;
    updateRender();
}

void PublishGLMapImageRenderer::handlePartyIconPosChanged(const QPoint& pos)
{
    Q_UNUSED(pos);
    updateRender();
}

void PublishGLMapImageRenderer::handleShowPartyChanged(bool showParty)
{
    Q_UNUSED(showParty);
    updateRender();
}

void PublishGLMapImageRenderer::handlePartyScaleChanged(int partyScale)
{
    if(_partyToken)
    {
        _partyToken->setScale(0.04f * static_cast<float>(partyScale));
        updateRender();
    }
}
