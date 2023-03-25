#include "publishglmaprenderer.h"
#include "map.h"
#include "party.h"
#include "undomarker.h"
#include "mapmarkergraphicsitem.h"
#include "publishglbattlebackground.h"
#include "publishglobject.h"
#include "publishglimage.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPainterPath>
#include <QUndoStack>
#include <QDebug>

PublishGLMapRenderer::PublishGLMapRenderer(Map* map, QObject *parent) :
    PublishGLRenderer(parent),
    _map(map),
    _scene(),
    _targetSize(),
    _color(),
    _projectionMatrix(),
    _cameraRect(),
    _scissorRect(),
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
//    _shaderProgram(0),
//    _shaderModelMatrix(0),
//    _fowImage(),
//    _fowObject(nullptr),
    _partyToken(nullptr),
    _lineImage(nullptr),
    _markerTokens(),
    _recreatePartyToken(false),
    _recreateLineToken(false),
    _recreateMarkers(false),
    _updateFow(false)
{
    if(_map)
    {
        connect(_map, &Map::partyChanged, this, &PublishGLMapRenderer::handlePartyChanged);
        connect(_map, &Map::partyIconChanged, this, &PublishGLMapRenderer::handlePartyIconChanged);
        connect(_map, &Map::partyIconPosChanged, this, &PublishGLMapRenderer::handlePartyIconPosChanged);
        connect(_map, &Map::showPartyChanged, this, &PublishGLMapRenderer::handleShowPartyChanged);
        connect(_map, &Map::partyScaleChanged, this, &PublishGLMapRenderer::handlePartyScaleChanged);
        connect(&_map->getLayerScene(), &LayerScene::layerAdded, this, &PublishGLMapRenderer::layerAdded);
        connect(&_map->getLayerScene(), &LayerScene::layerRemoved, this, &PublishGLRenderer::updateWidget);
    }
}

PublishGLMapRenderer::~PublishGLMapRenderer()
{
}

CampaignObjectBase* PublishGLMapRenderer::getObject()
{
    return _map;
}

QColor PublishGLMapRenderer::getBackgroundColor()
{
    return _color;
}

bool PublishGLMapRenderer::deleteOnDeactivation()
{
    return true;
}

QRect PublishGLMapRenderer::getScissorRect()
{
    return _scissorRect;
}

void PublishGLMapRenderer::setBackgroundColor(const QColor& color)
{
    _color = color;
    emit updateWidget();
}

void PublishGLMapRenderer::initializeGL()
{    
    if((_initialized) || (!_targetWidget) || (!_targetWidget->context()) || (!_map))
        return;

    _scene.setGridScale(_map->getPartyScale());

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    qDebug() << "[PublishGLMapRenderer] Initializing renderer";

    createShaders();
    _map->getLayerScene().playerSetShaders(_shaderProgramRGB, _shaderModelMatrixRGB, _shaderProjectionMatrixRGB, _shaderProgramRGBA, _shaderModelMatrixRGBA, _shaderProjectionMatrixRGBA, _shaderAlphaRGBA);

/*
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
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSource = "#version 410 core\n"
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

    qDebug() << "[PublishGLMapRenderer]::initializeGL Program: " << _shaderProgram << ", context: " << _targetWidget->context();
    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);
    _shaderModelMatrix = f->glGetUniformLocation(_shaderProgram, "model");
    qDebug() << "[PublishGLMapRenderer] Program: " << _shaderProgram << ", model matrix: " << _shaderModelMatrix;
*/

    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    _scene.deriveSceneRectFromSize(_map->getLayerScene().sceneSize());

    // Create the objects
    //initializeBackground();
    _map->getLayerScene().playerGLInitialize(this, &_scene);
    updateProjectionMatrix();
    updateFoW();

    // Create the party token
    createPartyToken();

    // Create the markers
    _recreateMarkers = true;

    // Check if we need a pointer
    evaluatePointer();

    /*
    qDebug() << "[PublishGLMapRenderer]::initializeGL Program: " << _shaderProgram << ", context: " << _targetWidget->context();
    f->glUseProgram(_shaderProgram);
    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());
    */

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
}

void PublishGLMapRenderer::cleanupGL()
{
    _initialized = false;

    qDeleteAll(_markerTokens);
    _markerTokens.clear();

    delete _lineImage;
    _lineImage = nullptr;

    delete _partyToken;
    _partyToken = nullptr;
    _recreatePartyToken = false;

    if(_map)
        _map->getLayerScene().playerGLUninitialize();

//    delete _fowObject;
//    _fowObject = nullptr;

    _projectionMatrix.setToIdentity();

    /*
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
    */

    //_map->getLayerScene().playerSetShaders(0, 0, 0, 0, 0, 0, 0);
    destroyShaders();

    PublishGLRenderer::cleanupGL();
}

void PublishGLMapRenderer::resizeGL(int w, int h)
{
    _targetSize = QSize(w, h);
    qDebug() << "[PublishGLMapRenderer] Resize w: " << w << ", h: " << h;
//    resizeBackground(w, h);
    _scene.setTargetSize(_targetSize);
    if(_map)
        _map->getLayerScene().playerGLResize(w, h);

    updateProjectionMatrix();

    emit updateWidget();
}

void PublishGLMapRenderer::paintGL()
{
    if((!_initialized) || (!_map) || (!_targetSize.isValid()) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    //qDebug() << "[PublishGLMapRenderer]::paintGL context: " << _targetWidget->context();

    if(_map->getLayerScene().playerGLUpdate())
        updateProjectionMatrix();

        /*
    if(!isBackgroundReady())
    {
        updateBackground();
        if(!isBackgroundReady())
            return;

        updateProjectionMatrix();

        _recreatePartyToken = true;
        _recreateLineToken = true;
        _recreateMarkers = true;
        _updateFow = true;
    }
    QSize sceneSize = getBackgroundSize().toSize();
    */
    QSize sceneSize = _map->getLayerScene().sceneSize().toSize();

    if(_recreatePartyToken)
        createPartyToken();

    if(_updateFow)
        updateFoW();

    if(((_map->getMapItemCount() > 0) && (!_lineImage)) || (_recreateLineToken))
        createLineToken(sceneSize);

    if((_map->getMarkerCount() > 0) && (_recreateMarkers))
        createMarkerTokens(sceneSize);

    evaluatePointer();

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    if(!_scissorRect.isEmpty())
    {
        int pixelRatio = _targetWidget->devicePixelRatio();
        f->glEnable(GL_SCISSOR_TEST);
        f->glScissor(_scissorRect.x()*pixelRatio, _scissorRect.y()*pixelRatio, _scissorRect.width()*pixelRatio, _scissorRect.height()*pixelRatio);
    }

    // Draw the scene
    f->glClearColor(_color.redF(), _color.greenF(), _color.blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the default render program
    //qDebug() << "[PublishGLMapRenderer]::paintGL UseProgram #1: " << _shaderProgramRGB << ", context: " << _targetWidget->context();
    f->glUseProgram(_shaderProgramRGB);
    f->glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, _projectionMatrix.constData());

    //paintBackground(f);
    _map->getLayerScene().playerGLPaint(f, _shaderProgramRGB, _shaderModelMatrixRGB, _projectionMatrix.constData());

    // Set the current program, in case the layers changed the program
    //qDebug() << "[PublishGLMapRenderer]::paintGL UseProgram #2: " << _shaderProgramRGB << ", context: " << _targetWidget->context();
    f->glUseProgram(_shaderProgramRGB);

    /*
    if(_fowObject)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _fowObject->getMatrixData());
        _fowObject->paintGL();
    }
    */

    if(_lineImage)
    {
        f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _lineImage->getMatrixData());
        _lineImage->paintGL();
    }

    if((_partyToken) && (_map->getShowParty()))
    {
        _partyToken->setPosition(_map->getPartyIconPos().x() - (sceneSize.width() / 2), (sceneSize.height() / 2) - _map->getPartyIconPos().y() - _partyToken->getSize().height());
        f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _partyToken->getMatrixData());
        _partyToken->paintGL();
    }

    if((_markerTokens.count() > 0) && (_map->getShowMarkers()))
    {
        for(PublishGLImage* markerToken : _markerTokens)
        {
            if(markerToken)
            {
                f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, markerToken->getMatrixData());
                markerToken->paintGL();
            }
        }
    }

    if(!_scissorRect.isEmpty())
        f->glDisable(GL_SCISSOR_TEST);

    paintPointer(f, sceneSize, _shaderModelMatrixRGB);
}

void PublishGLMapRenderer::updateProjectionMatrix()
{
    if((_shaderProgramRGB == 0) || (!_targetSize.isValid()) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    // Update projection matrix and other size related settings:
    QRectF transformedCamera = _cameraRect;
    QSizeF transformedTarget = _targetSize;
    if((_rotation == 90) || (_rotation == 270))
    {
        transformedCamera = transformedCamera.transposed();
        transformedCamera.moveTo(transformedCamera.topLeft().transposed());
        transformedTarget.transpose();
    }

    QSizeF rectSize = transformedTarget.scaled(_cameraRect.size(), Qt::KeepAspectRatioByExpanding);
    QSizeF halfRect = rectSize / 2.0;
    QPointF cameraTopLeft((rectSize.width() - _cameraRect.width()) / 2.0, (rectSize.height() - _cameraRect.height()) / 2);
    QPointF cameraMiddle(_cameraRect.x() + (_cameraRect.width() / 2.0), _cameraRect.y() + (_cameraRect.height() / 2.0));
    //QSizeF backgroundMiddle = getBackgroundSize() / 2.0;
    QSizeF backgroundMiddle = _map->getLayerScene().sceneSize() / 2.0;

    qDebug() << "[PublishGLMapRenderer] camera rect: " << _cameraRect << ", transformed camera: " << transformedCamera << ", target size: " << _targetSize << ", transformed target: " << transformedTarget;
    qDebug() << "[PublishGLMapRenderer] rectSize: " << rectSize << ", camera top left: " << cameraTopLeft << ", camera middle: " << cameraMiddle << ", background middle: " << backgroundMiddle;

    _projectionMatrix.setToIdentity();
    _projectionMatrix.rotate(_rotation, 0.0, 0.0, -1.0);

    int l = cameraMiddle.x() - backgroundMiddle.width() - halfRect.width();
    int r = cameraMiddle.x() - backgroundMiddle.width() + halfRect.width();
    int t = backgroundMiddle.height() - cameraMiddle.y() - halfRect.height();
    int b = backgroundMiddle.height() - cameraMiddle.y() + halfRect.height();
    qDebug() << "[PublishGLMapRenderer] l: " << l << ", r: " << r << ", t: " << t << ", b: " << b;

    _projectionMatrix.ortho(cameraMiddle.x() - backgroundMiddle.width() - halfRect.width(), cameraMiddle.x() - backgroundMiddle.width() + halfRect.width(),
                            backgroundMiddle.height() - cameraMiddle.y() - halfRect.height(), backgroundMiddle.height() - cameraMiddle.y() + halfRect.height(),
                            0.1f, 1000.f);
    //_projectionMatrix.ortho(-401, 401, -301, 301, 0.1f, 1000.f);

    setPointerScale(rectSize.width() / transformedTarget.width());

    QSizeF scissorSize = transformedCamera.size().scaled(_targetSize, Qt::KeepAspectRatio);
    qDebug() << "[PublishGLMapRenderer] scissor size: " << scissorSize;
    _scissorRect.setX((_targetSize.width() - scissorSize.width()) / 2.0);
    _scissorRect.setY((_targetSize.height() - scissorSize.height()) / 2.0);
    _scissorRect.setWidth(scissorSize.width());
    _scissorRect.setHeight(scissorSize.height());
}

void PublishGLMapRenderer::distanceChanged()
{
    _recreateLineToken = true;
}

void PublishGLMapRenderer::fowChanged(const QImage& fow)
{
    if(fow.isNull())
        return;

//    _fowImage = fow;
    _updateFow = true;
    emit updateWidget();
}

void PublishGLMapRenderer::setCameraRect(const QRectF& cameraRect)
{
    if(_cameraRect != cameraRect)
    {
        _cameraRect = cameraRect;
        updateProjectionMatrix();
        emit updateWidget();
    }
}

void PublishGLMapRenderer::markerChanged()
{
    _recreateMarkers = true;
}

/*
void PublishGLMapRenderer::updateBackground()
{
}
*/

void PublishGLMapRenderer::createPartyToken()
{
    if(_partyToken)
    {
        delete _partyToken;
        _partyToken = nullptr;
    }

    if(_map)
    {
        QImage partyImage = _map->getPartyPixmap().toImage();
        if(!partyImage.isNull())
        {
            _partyToken = new PublishGLImage(partyImage, false);
            _partyToken->setScale(0.04f * static_cast<float>(_map->getPartyScale()));
        }
    }

    _recreatePartyToken = false;
}

void PublishGLMapRenderer::createLineToken(const QSize& sceneSize)
{
    if((!_map) || (_map->getMapItemCount() <= 0) || (sceneSize.isEmpty()))
        return;

    _recreateLineToken = false;

    MapDrawLine* line = dynamic_cast<MapDrawLine*>(_map->getMapItem(0));
    if(line)
    {
        QFont textFont;
        textFont.setPointSize(DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] / 20);
        qreal lineDistance = line->length() * _map->getMapScale() / 1000.0;
        QString distanceText;
        distanceText = QString::number(lineDistance, 'f', 1);

        QSize lineImageSize = line->lineSize();
        QFontMetrics fontMetrics(textFont);
        if((lineImageSize.width() / 2) < fontMetrics.horizontalAdvance(distanceText))
            lineImageSize.setWidth((lineImageSize.width() / 2) + fontMetrics.horizontalAdvance(distanceText));

        QImage lineImage(lineImageSize, QImage::Format_RGBA8888);
        lineImage.fill(Qt::transparent);
        QPainter linePainter;
        linePainter.begin(&lineImage);
            linePainter.setPen(QPen(QBrush(line->penColor()), line->penWidth(), line->penStyle()));
            linePainter.drawLine(line->originLine());

            linePainter.setFont(textFont);
            linePainter.setPen(QPen(QBrush(line->penColor()), line->penWidth(), Qt::SolidLine));
            linePainter.drawText(line->originCenter(), distanceText);
        linePainter.end();

        if(_lineImage)
            _lineImage->setImage(lineImage);
        else
            _lineImage = new PublishGLImage(lineImage, false);

        _lineImage->setPosition(line->origin().x() - (sceneSize.width() / 2), (sceneSize.height() / 2) - line->origin().y() - _lineImage->getSize().height());
        return;
    }

    MapDrawPath* path = dynamic_cast<MapDrawPath*>(_map->getMapItem(0));
    if((path) && (path->points().count() > 0))
    {
        QRect pathRect = path->pathRect();
        QPainterPath painterPath(path->points().first() - pathRect.topLeft());
        for(int i = 1; i < path->points().count(); ++i)
            painterPath.lineTo(path->points().at(i) - pathRect.topLeft());

        QFont textFont;
        textFont.setPointSize(DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] / 20);
        qreal pathDistance = painterPath.length() * _map->getMapScale() / 1000.0;
        QString distanceText;
        distanceText = QString::number(pathDistance, 'f', 1);

        QFontMetrics fontMetrics(textFont);
        int topPosition = pathRect.top();
        if(pathRect.right() < path->points().last().x() + fontMetrics.horizontalAdvance(distanceText))
            pathRect.setRight(path->points().last().x() + fontMetrics.horizontalAdvance(distanceText));
        if(pathRect.top() > path->points().last().y() - fontMetrics.height())
            pathRect.setTop(path->points().last().y() - fontMetrics.height());

        QImage pathImage(pathRect.size(), QImage::Format_RGBA8888);
        pathImage.fill(Qt::transparent);
        QPainter pathPainter;
        pathPainter.begin(&pathImage);
            pathPainter.setPen(QPen(QBrush(path->penColor()), path->penWidth(), path->penStyle()));
            pathPainter.drawPath(painterPath);

            pathPainter.setFont(textFont);
            pathPainter.setPen(QPen(QBrush(path->penColor()), path->penWidth(), Qt::SolidLine));
            pathPainter.drawText(path->points().last() - pathRect.topLeft(), distanceText);
        pathPainter.end();

        if(_lineImage)
            _lineImage->setImage(pathImage);
        else
            _lineImage = new PublishGLImage(pathImage, false);

        _lineImage->setPosition(pathRect.topLeft().x() - (sceneSize.width() / 2), (sceneSize.height() / 2) - topPosition - _lineImage->getSize().height());
        return;
    }
}

void PublishGLMapRenderer::createMarkerTokens(const QSize& sceneSize)
{
    qDeleteAll(_markerTokens);
    _markerTokens.clear();

    if((!_map) || (sceneSize.isEmpty()))
        return;

    _recreateMarkers = false;

    if(!_map->getShowMarkers())
        return;

    // TODO: Layers
    /*
    QUndoStack* stack = _map->getUndoStack();
    if(!stack)
        return;

    for( int i = 0; i < stack->index(); ++i )
    {
        const UndoMarker* markerAction = dynamic_cast<const UndoMarker*>(stack->command(i));
        if((markerAction) && (markerAction->getMarker().isPlayerVisible()))
        {
            MapMarkerGraphicsItem* markerItem = markerAction->getMarkerItem();
            if((markerItem) && (!markerItem->getGraphicsItemPixmap().isNull()))
            {
                QImage markerImage = markerItem->getGraphicsItemPixmap().toImage();
                PublishGLImage* newMarkerItem = new PublishGLImage(markerImage, false);
                QPointF markerTopLeft = markerItem->getTopLeft();
                newMarkerItem->setPosition(markerItem->x() + markerTopLeft.x() - (sceneSize.width() / 2),
                                           (sceneSize.height() / 2) - (markerItem->y() + markerTopLeft.y() + markerImage.height()));

                _markerTokens.append(newMarkerItem);
            }
        }
    }
    */
}

void PublishGLMapRenderer::updateFoW()
{
    //if((!_map) || (_fowImage.isNull()))
    if(!_map)
        return;

    //QSize backgroundSize = getBackgroundSize().toSize();
    QSize backgroundSize = _map->getLayerScene().sceneSize().toSize();
    if(backgroundSize.isEmpty())
        return;

    /*
    if(_fowObject)
        _fowObject->setImage(_fowImage);
    else
        _fowObject = new PublishGLBattleBackground(nullptr, _fowImage, GL_NEAREST);
    */

    _updateFow = false;
}

void PublishGLMapRenderer::updateContents()
{
}

void PublishGLMapRenderer::createShaders()
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

void PublishGLMapRenderer::destroyShaders()
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

void PublishGLMapRenderer::handlePartyChanged(Party* party)
{
    Q_UNUSED(party);
    _recreatePartyToken = true;
    updateRender();
}

void PublishGLMapRenderer::handlePartyIconChanged(const QString& partyIcon)
{
    Q_UNUSED(partyIcon);
    _recreatePartyToken = true;
    updateRender();
}

void PublishGLMapRenderer::handlePartyIconPosChanged(const QPoint& pos)
{
    Q_UNUSED(pos);
    updateRender();
}

void PublishGLMapRenderer::handleShowPartyChanged(bool showParty)
{
    Q_UNUSED(showParty);
    updateRender();
}

void PublishGLMapRenderer::handlePartyScaleChanged(int partyScale)
{
    if(_partyToken)
    {
        _partyToken->setScale(0.04f * static_cast<float>(partyScale));
        updateRender();
    }
}

void PublishGLMapRenderer::layerAdded(Layer* layer)
{
    if((!layer) || (!_initialized))
        return;

    layer->playerSetShaders(_shaderProgramRGB, _shaderModelMatrixRGB, _shaderProjectionMatrixRGB, _shaderProgramRGBA, _shaderModelMatrixRGBA, _shaderProjectionMatrixRGBA, _shaderAlphaRGBA);
    //layer->playerGLInitialize(this, &_scene);
    emit updateWidget();
}
