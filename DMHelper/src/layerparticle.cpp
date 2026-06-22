#include "layerparticle.h"
#include "layerparticlesettings.h"
#include "publishglrenderer.h"
#include "dmh_opengl.h"
#include <QImage>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMatrix4x4>
#include <QTimerEvent>
#include <QtMath>
#include <vector>

const int LAYERPARTICLE_PREVIEWSIZE = 512;
const int LAYERPARTICLE_TIMERPERIOD = 30;

// Vertex shader: 3D particle lines with animated falling
const char *particleVertexShaderSource = "#version 410 core\n"
    "layout (location = 0) in vec3 aBasePos;\n"
    "layout (location = 1) in float aEndpoint;\n"
    "layout (location = 2) in float aSpeedVar;\n"
    "uniform mat4 u_mvp;\n"
    "uniform float u_time;\n"
    "uniform float u_speed;\n"
    "uniform float u_length;\n"
    "uniform float u_movement;\n"
    "void main()\n"
    "{\n"
    "   vec3 pos = aBasePos;\n"
    "   float speed = u_speed * (0.7 + aSpeedVar * 0.6);\n"
    "   pos.y = fract(pos.y - u_time * speed * 0.000005) * 2.0 - 1.0;\n"
    "   pos.xz = pos.xz * 2.0 - 1.0;\n"
    "   if(u_movement > 0.0) {\n"
    "       float phase = u_time * 0.003 + aBasePos.z * 17.0 + aSpeedVar * 50.0;\n"
    "       pos.x += sin(phase) * u_movement * 0.01;\n"
    "       pos.z += cos(phase * 1.3 + 2.0) * u_movement * 0.01;\n"
    "   }\n"
    "   pos.y -= aEndpoint * u_length * 0.01;\n"
    "   gl_Position = u_mvp * vec4(pos, 1.0);\n"
    "}\0";

// Fragment shader: simple color output
const char *particleFragmentShaderSource = "#version 410 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 u_color;\n"
    "uniform float u_opacity;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(u_color.rgb, u_color.a * u_opacity);\n"
    "}\n";

LayerParticle::LayerParticle(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
    _scene(nullptr),
    _timerId(0),
    _milliseconds(0),
    _objectsDirty(true),
    _VAO(0),
    _VBO(0),
    _shaderTime(0),
    _shaderSpeed(0),
    _shaderLength(0),
    _shaderColor(0),
    _shaderOpacity(0),
    _shaderMVP(0),
    _shaderMovement(0),
    _vertexCount(0),
    _particleCount(defaultParticleCount),
    _rainSpeed(defaultRainSpeed),
    _rainDirection(defaultRainDirection),
    _rainAngle(defaultRainAngle),
    _rainColor(QColor(200, 200, 255, 180)),
    _rainLength(defaultRainLength),
    _rainOpacity(defaultRainOpacity),
    _rainWidth(defaultRainWidth),
    _rainMovement(defaultRainMovement)
{
}

LayerParticle::~LayerParticle()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerParticle::inputXML(const QDomElement &element, bool isImport)
{
    _particleCount  = element.attribute("particleCount", QString::number(defaultParticleCount)).toInt();
    _rainSpeed      = element.attribute("rainSpeed", QString::number(defaultRainSpeed)).toInt();
    _rainDirection  = element.attribute("rainDirection", QString::number(defaultRainDirection)).toInt();
    _rainAngle      = element.attribute("rainAngle", QString::number(defaultRainAngle)).toInt();
    _rainLength     = element.attribute("rainLength", QString::number(defaultRainLength)).toInt();
    _rainOpacity    = element.attribute("rainOpacity", QString::number(defaultRainOpacity)).toInt();
    _rainColor      = QColor(element.attribute("rainColor", QColor(200, 200, 255, 180).name(QColor::HexArgb)));
    _rainWidth      = element.attribute("rainWidth", QString::number(defaultRainWidth)).toInt();
    _rainMovement     = element.attribute("rainMovement", QString::number(defaultRainMovement)).toInt();

    Layer::inputXML(element, isImport);
}

QRectF LayerParticle::boundingRect() const
{
    return QRectF();
}

QImage LayerParticle::getLayerIcon() const
{
    return QImage(":/img/data/icon_clouds.png");
}

bool LayerParticle::defaultShader() const
{
    return false;
}

bool LayerParticle::hasSettings() const
{
    return true;
}

DMHelper::LayerType LayerParticle::getType() const
{
    return DMHelper::LayerType_Particle;
}

Layer* LayerParticle::clone() const
{
    LayerParticle* newLayer = new LayerParticle(_name, _order);
    copyBaseValues(newLayer);
    newLayer->_particleCount  = _particleCount;
    newLayer->_rainSpeed      = _rainSpeed;
    newLayer->_rainDirection  = _rainDirection;
    newLayer->_rainAngle      = _rainAngle;
    newLayer->_rainColor      = _rainColor;
    newLayer->_rainLength     = _rainLength;
    newLayer->_rainOpacity    = _rainOpacity;
    newLayer->_rainWidth      = _rainWidth;
    newLayer->_rainMovement     = _rainMovement;
    return newLayer;
}

void LayerParticle::applyOrder(int order)
{
    if(_graphicsItem)
        _graphicsItem->setZValue(order);
}

void LayerParticle::applyLayerVisibleDM(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);
}

void LayerParticle::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerParticle::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;
    if(_graphicsItem)
        _graphicsItem->setOpacity(opacity);
}

void LayerParticle::applyPosition(const QPoint& position)
{
    if(_graphicsItem)
        _graphicsItem->setPos(position);
}

void LayerParticle::applySize(const QSize& size)
{
    Q_UNUSED(size);
    _objectsDirty = true;
    refreshDMPreview();
}

// DM Window Interface

void LayerParticle::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    if(_graphicsItem)
    {
        qDebug() << "[LayerParticle] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    QImage preview = createRainPreview(_size.isEmpty() ? QSize(LAYERPARTICLE_PREVIEWSIZE, LAYERPARTICLE_PREVIEWSIZE) : _size);
    _graphicsItem = scene->addPixmap(QPixmap::fromImage(preview));

    if(_graphicsItem)
    {
        _graphicsItem->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
        _graphicsItem->setPos(_position);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());
        _graphicsItem->setOpacity(_opacityReference);
    }

    Layer::dmInitialize(scene);
}

void LayerParticle::dmUninitialize()
{
    cleanupDM();
}

void LayerParticle::dmUpdate()
{
}

// Player Window Interface

void LayerParticle::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    _scene = scene;
    Layer::playerGLInitialize(renderer, scene);

    if((_timerId == 0) && (renderer))
    {
        connect(this, &LayerParticle::update, renderer, &PublishGLRenderer::updateWidget);
        _timerId = startTimer(LAYERPARTICLE_TIMERPERIOD);
    }
}

void LayerParticle::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerParticle::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);
    Q_UNUSED(projectionMatrix);

    if(!functions)
        return;

    DMH_DEBUG_OPENGL_PAINTGL();

    if(_shaderProgramRGBA == 0)
        createShaders();

    if((_VAO == 0) || _objectsDirty)
        createObjects();

    if((!QOpenGLContext::currentContext()) || (!functions) || (_shaderProgramRGBA == 0) || (_VAO == 0))
        return;

    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if(!e)
        return;

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    functions->glUseProgram(_shaderProgramRGBA);

    // Build perspective MVP from drop angle and direction
    float aspect = _size.isEmpty() ? 1.0f : static_cast<float>(_size.width()) / static_cast<float>(_size.height());

    QMatrix4x4 projection;
    projection.perspective(60.0f, aspect, 0.1f, 100.0f);

    // Camera position from drop angle (elevation)
    // angle=0: side view (drops fall top-to-bottom)
    // angle=90: top-down (drops fall into screen)
    float elevRad = qDegreesToRadians(static_cast<float>(_rainAngle));
    float camDist = 2.0f;

    float camY = camDist * qSin(elevRad);
    float camZ = camDist * qCos(elevRad);

    QVector3D eye(0.0f, camY, camZ);
    QVector3D center(0.0f, 0.0f, 0.0f);

    // Up vector: avoid degeneracy when looking straight up/down
    QVector3D forward = (center - eye).normalized();
    QVector3D worldUp(0.0f, 1.0f, 0.0f);
    if(qAbs(QVector3D::dotProduct(forward, worldUp)) > 0.99f)
        worldUp = QVector3D(0.0f, 0.0f, -1.0f);

    QMatrix4x4 view;
    view.lookAt(eye, center, worldUp);

    // Direction rotates the view around the look axis (screen rotation)
    // 0=top-to-bottom, 180=bottom-to-top, 90=left-to-right
    view.rotate(static_cast<float>(_rainDirection), forward);

    QMatrix4x4 mvp = projection * view;

    functions->glUniformMatrix4fv(_shaderMVP, 1, GL_FALSE, mvp.constData());

    functions->glEnable(GL_BLEND);
    functions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    functions->glUniform1f(_shaderTime, static_cast<float>(_milliseconds));
    functions->glUniform1f(_shaderSpeed, static_cast<float>(_rainSpeed));
    functions->glUniform1f(_shaderLength, static_cast<float>(_rainLength));
    functions->glUniform1f(_shaderOpacity, static_cast<float>(_rainOpacity) / 100.f);
    functions->glUniform1f(_shaderMovement, static_cast<float>(_rainMovement));
    functions->glUniform4f(_shaderColor,
                           _rainColor.redF(), _rainColor.greenF(), _rainColor.blueF(),
                           _opacityReference);

    functions->glLineWidth(static_cast<float>(_rainWidth));

    e->glBindVertexArray(_VAO);
    functions->glDrawArrays(GL_LINES, 0, _vertexCount);
}

void LayerParticle::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    _objectsDirty = true;
}

void LayerParticle::playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA)
{
    Q_UNUSED(programRGB);
    Q_UNUSED(modelMatrixRGB);
    Q_UNUSED(projectionMatrixRGB);
    Q_UNUSED(programRGBA);
    Q_UNUSED(modelMatrixRGBA);
    Q_UNUSED(projectionMatrixRGBA);
    Q_UNUSED(alphaRGBA);
}

bool LayerParticle::playerIsInitialized()
{
    return _scene != nullptr;
}

void LayerParticle::initialize(const QSize& sceneSize)
{
    if(getSize().isEmpty())
        setSize(sceneSize);
}

void LayerParticle::uninitialize()
{
}

void LayerParticle::editSettings()
{
    LayerParticleSettings* dlg = new LayerParticleSettings();

    // Use lambdas to update values directly during dialog interaction.
    // The timer-driven animation already picks up changes every 30ms
    // for live preview, so we don't need to emit update() per slider tick.
    // Emit dirty() once when the dialog closes to avoid signal spam.
    connect(dlg, &LayerParticleSettings::particleCountChanged,  this, [this](int v) { _particleCount = v; _objectsDirty = true; });
    connect(dlg, &LayerParticleSettings::rainSpeedChanged,      this, [this](int v) { _rainSpeed = v; });
    connect(dlg, &LayerParticleSettings::rainDirectionChanged,  this, [this](int v) { _rainDirection = v; });
    connect(dlg, &LayerParticleSettings::rainAngleChanged,      this, [this](int v) { _rainAngle = v; });
    connect(dlg, &LayerParticleSettings::rainColorChanged,      this, [this](const QColor& c) { _rainColor = c; });
    connect(dlg, &LayerParticleSettings::rainLengthChanged,     this, [this](int v) { _rainLength = v; });
    connect(dlg, &LayerParticleSettings::rainOpacityChanged,    this, [this](int v) { _rainOpacity = v; });
    connect(dlg, &LayerParticleSettings::rainWidthChanged,      this, [this](int v) { _rainWidth = v; });
    connect(dlg, &LayerParticleSettings::rainMovementChanged,     this, [this](int v) { _rainMovement = v; });

    dlg->setParticleCount(_particleCount);
    dlg->setRainSpeed(_rainSpeed);
    dlg->setRainDirection(_rainDirection);
    dlg->setRainAngle(_rainAngle);
    dlg->setRainColor(_rainColor);
    dlg->setRainLength(_rainLength);
    dlg->setRainOpacity(_rainOpacity);
    dlg->setRainWidth(_rainWidth);
    dlg->setRainMovement(_rainMovement);

    dlg->exec();

    emit dirty();
    refreshDMPreview();

    dlg->deleteLater();
}

void LayerParticle::setParticleCount(int count)
{
    if(_particleCount == count)
        return;
    _particleCount = count;
    _objectsDirty = true;
    emit dirty();
    emit update();
}

void LayerParticle::setRainSpeed(int speed)
{
    if(_rainSpeed == speed)
        return;
    _rainSpeed = speed;
    emit dirty();
    emit update();
}

void LayerParticle::setRainDirection(int direction)
{
    if(_rainDirection == direction)
        return;
    _rainDirection = direction;
    emit dirty();
    emit update();
}

void LayerParticle::setRainAngle(int angle)
{
    if(_rainAngle == angle)
        return;
    _rainAngle = angle;
    emit dirty();
    emit update();
}

void LayerParticle::setRainColor(const QColor& color)
{
    if(_rainColor == color)
        return;
    _rainColor = color;
    emit dirty();
    emit update();
}

void LayerParticle::setRainLength(int length)
{
    if(_rainLength == length)
        return;
    _rainLength = length;
    emit dirty();
    emit update();
}

void LayerParticle::setRainOpacity(int opacity)
{
    if(_rainOpacity == opacity)
        return;
    _rainOpacity = opacity;
    emit dirty();
    emit update();
}

void LayerParticle::setRainWidth(int width)
{
    if(_rainWidth == width)
        return;
    _rainWidth = width;
    emit dirty();
    emit update();
}

void LayerParticle::setRainMovement(int movement)
{
    if(_rainMovement == movement)
        return;
    _rainMovement = movement;
    emit dirty();
    emit update();
}

void LayerParticle::timerEvent(QTimerEvent *event)
{
    if((event) && (event->timerId() > 0) && (event->timerId() == _timerId))
    {
        _milliseconds += LAYERPARTICLE_TIMERPERIOD;
        emit update();
    }
}

void LayerParticle::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    if(_particleCount != defaultParticleCount)
        element.setAttribute("particleCount", _particleCount);

    if(_rainSpeed != defaultRainSpeed)
        element.setAttribute("rainSpeed", _rainSpeed);

    if(_rainDirection != defaultRainDirection)
        element.setAttribute("rainDirection", _rainDirection);

    if(_rainAngle != defaultRainAngle)
        element.setAttribute("rainAngle", _rainAngle);

    if(_rainLength != defaultRainLength)
        element.setAttribute("rainLength", _rainLength);

    if(_rainOpacity != defaultRainOpacity)
        element.setAttribute("rainOpacity", _rainOpacity);

    if(_rainWidth != defaultRainWidth)
        element.setAttribute("rainWidth", _rainWidth);

    if(_rainMovement != defaultRainMovement)
        element.setAttribute("rainMovement", _rainMovement);

    QColor defaultColor(200, 200, 255, 180);
    if(_rainColor != defaultColor)
        element.setAttribute("rainColor", _rainColor.name(QColor::HexArgb));

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerParticle::cleanupDM()
{
    if(!_graphicsItem)
        return;

    if(_graphicsItem->scene())
        _graphicsItem->scene()->removeItem(_graphicsItem);

    delete _graphicsItem;
    _graphicsItem = nullptr;
}

void LayerParticle::cleanupPlayer()
{
    disconnect(this, &LayerParticle::update, nullptr, nullptr);

    if(_timerId > 0)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    destroyObjects();
    destroyShaders();

    _scene = nullptr;
}

void LayerParticle::createShaders()
{
    if((!QOpenGLContext::currentContext()) || (_shaderProgramRGBA != 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    int success;
    char infoLog[512];

    // Vertex shader
    unsigned int vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShader, 1, &particleVertexShaderSource, NULL);
    f->glCompileShader(vertexShader);

    f->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "[LayerParticle] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    // Fragment shader
    unsigned int fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShader, 1, &particleFragmentShaderSource, NULL);
    f->glCompileShader(fragmentShader);

    f->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "[LayerParticle] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    // Link program
    _shaderProgramRGBA = f->glCreateProgram();
    DMH_DEBUG_OPENGL_glCreateProgram(_shaderProgramRGBA, "_shaderProgramRGBA");

    f->glAttachShader(_shaderProgramRGBA, vertexShader);
    f->glAttachShader(_shaderProgramRGBA, fragmentShader);
    f->glLinkProgram(_shaderProgramRGBA);

    f->glGetProgramiv(_shaderProgramRGBA, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgramRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerParticle] ERROR::SHADER::PROGRAM::LINK_FAILED: " << infoLog;
        return;
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    f->glUseProgram(_shaderProgramRGBA);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);

    // Get uniform locations
    _shaderMVP     = f->glGetUniformLocation(_shaderProgramRGBA, "u_mvp");
    _shaderTime    = f->glGetUniformLocation(_shaderProgramRGBA, "u_time");
    _shaderSpeed   = f->glGetUniformLocation(_shaderProgramRGBA, "u_speed");
    _shaderLength  = f->glGetUniformLocation(_shaderProgramRGBA, "u_length");
    _shaderColor   = f->glGetUniformLocation(_shaderProgramRGBA, "u_color");
    _shaderOpacity = f->glGetUniformLocation(_shaderProgramRGBA, "u_opacity");
    _shaderMovement  = f->glGetUniformLocation(_shaderProgramRGBA, "u_movement");

    qDebug() << "[LayerParticle] Shaders created. Program:" << _shaderProgramRGBA;
}

void LayerParticle::destroyShaders()
{
    if(QOpenGLContext::currentContext())
    {
        QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
        if((e) && (_shaderProgramRGBA > 0))
        {
            DMH_DEBUG_OPENGL_Singleton::removeProgram(_shaderProgramRGBA);
            e->glDeleteProgram(_shaderProgramRGBA);
        }
    }

    _shaderProgramRGBA = 0;
    _shaderModelMatrixRGBA = 0;
    _shaderProjectionMatrixRGBA = 0;
    _shaderTime = 0;
    _shaderSpeed = 0;
    _shaderLength = 0;
    _shaderColor = 0;
    _shaderOpacity = 0;
    _shaderMVP = 0;
    _shaderMovement = 0;
}

void LayerParticle::createObjects()
{
    if(!QOpenGLContext::currentContext())
        return;

    destroyObjects();

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    // Generate particle data: 2 vertices per particle (top + bottom of line)
    // Each vertex: baseX, baseY, baseZ, endpoint, speedVar = 5 floats
    _vertexCount = _particleCount * 2;
    int floatsPerVertex = 5;
    std::vector<float> vertices(_vertexCount * floatsPerVertex);

    // Deterministic pseudo-random
    uint seed = 12345;
    auto nextRand = [&seed]() -> float {
        seed = seed * 1103515245 + 12345;
        return static_cast<float>((seed >> 16) & 0x7FFF) / 32767.0f;
    };

    for(int i = 0; i < _particleCount; ++i)
    {
        float bx = nextRand();
        float by = nextRand();
        float bz = nextRand();
        float speedVar = nextRand();

        int base0 = i * 2 * floatsPerVertex;
        int base1 = base0 + floatsPerVertex;

        // Top vertex (endpoint = 0)
        vertices[base0 + 0] = bx;
        vertices[base0 + 1] = by;
        vertices[base0 + 2] = bz;
        vertices[base0 + 3] = 0.0f;
        vertices[base0 + 4] = speedVar;

        // Bottom vertex (endpoint = 1)
        vertices[base1 + 0] = bx;
        vertices[base1 + 1] = by;
        vertices[base1 + 2] = bz;
        vertices[base1 + 3] = 1.0f;
        vertices[base1 + 4] = speedVar;
    }

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);

    e->glBindVertexArray(_VAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, static_cast<int>(vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);

    // aBasePos (location 0): vec3
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, floatsPerVertex * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // aEndpoint (location 1): float
    f->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, floatsPerVertex * sizeof(float), (void*)(3 * sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // aSpeedVar (location 2): float
    f->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, floatsPerVertex * sizeof(float), (void*)(4 * sizeof(float)));
    f->glEnableVertexAttribArray(2);

    _objectsDirty = false;

    qDebug() << "[LayerParticle] Objects created, particles:" << _particleCount;
}

void LayerParticle::destroyObjects()
{
    if(QOpenGLContext::currentContext())
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();

        if((f) && (e))
        {
            if(_VAO > 0)
                e->glDeleteVertexArrays(1, &_VAO);
            if(_VBO > 0)
                f->glDeleteBuffers(1, &_VBO);
        }
    }

    _VAO = 0;
    _VBO = 0;
}

void LayerParticle::refreshDMPreview()
{
    if(!_graphicsItem)
        return;

    QSize previewSize = _size.isEmpty() ? QSize(LAYERPARTICLE_PREVIEWSIZE, LAYERPARTICLE_PREVIEWSIZE) : _size;
    QImage preview = createRainPreview(previewSize);
    _graphicsItem->setPixmap(QPixmap::fromImage(preview));
}

QImage LayerParticle::createRainPreview(const QSize& size) const
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor dropColor = _rainColor;
    dropColor.setAlphaF(static_cast<qreal>(_rainOpacity) / 100.0);
    QPen pen(dropColor);
    pen.setWidthF(static_cast<qreal>(_rainWidth));
    painter.setPen(pen);

    // Build the same MVP as the GL path
    float aspect = static_cast<float>(size.width()) / static_cast<float>(size.height());
    float elevRad = qDegreesToRadians(static_cast<float>(_rainAngle));
    float camDist = 2.0f;

    float camY = camDist * qSin(elevRad);
    float camZ = camDist * qCos(elevRad);

    QVector3D eye(0.0f, camY, camZ);
    QVector3D center(0.0f, 0.0f, 0.0f);
    QVector3D forward = (center - eye).normalized();
    QVector3D worldUp(0.0f, 1.0f, 0.0f);
    if(qAbs(QVector3D::dotProduct(forward, worldUp)) > 0.99f)
        worldUp = QVector3D(0.0f, 0.0f, -1.0f);

    QMatrix4x4 proj;
    proj.perspective(60.0f, aspect, 0.1f, 100.0f);
    QMatrix4x4 view;
    view.lookAt(eye, center, worldUp);
    view.rotate(static_cast<float>(_rainDirection), forward);
    QMatrix4x4 mvp = proj * view;

    float halfW = size.width() / 2.0f;
    float halfH = size.height() / 2.0f;
    float streakLen = _rainLength * 0.01f;

    // Project a 3D point to screen coordinates
    auto project = [&](const QVector3D& worldPos) -> QPointF {
        QVector4D clip = mvp * QVector4D(worldPos, 1.0f);
        if(clip.w() <= 0.001f)
            return QPointF(-1, -1);
        float ndcX = clip.x() / clip.w();
        float ndcY = clip.y() / clip.w();
        float sx = (ndcX * 0.5f + 0.5f) * size.width();
        float sy = (1.0f - (ndcY * 0.5f + 0.5f)) * size.height();
        return QPointF(sx, sy);
    };

    // Deterministic pseudo-random particles in a [-1,1] cube
    uint seed = 12345;
    auto nextRand = [&seed]() -> float {
        seed = seed * 1103515245 + 12345;
        return static_cast<float>((seed >> 16) & 0x7FFF) / 32767.0f;
    };

    for(int i = 0; i < _particleCount; ++i)
    {
        float bx = nextRand() * 2.0f - 1.0f;
        float by = nextRand() * 2.0f - 1.0f;
        float bz = nextRand() * 2.0f - 1.0f;

        QVector3D top(bx, by, bz);
        QVector3D bottom(bx, by - streakLen, bz);

        QPointF p0 = project(top);
        QPointF p1 = project(bottom);

        if(p0.x() < -50 || p0.x() > size.width() + 50 || p0.y() < -50 || p0.y() > size.height() + 50)
            continue;
        if(p1.x() < -50 || p1.x() > size.width() + 50 || p1.y() < -50 || p1.y() > size.height() + 50)
            continue;

        painter.drawLine(p0, p1);
    }

    painter.end();
    return image;
}
