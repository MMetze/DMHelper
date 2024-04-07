#include "layereffect.h"
#include "publishglrenderer.h"
#include "layereffectsettings.h"
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMatrix4x4>
#include <QTimerEvent>
#include <QOpenGLExtraFunctions>

const qreal LAYEREFFECT_SPEED_FACTOR = 1.0 / 100000.0;
const qreal LAYEREFFECT_MORPH_FACTOR = 1.0 / 1000000.0;
const int LAYEREFFECT_PREVIEWSIZE = 512;
const int LAYEREFFECT_TIMERPERIOD = 30;

const char *vertexShaderSourceRGBColor = "#version 410 core\n"
                                         "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
                                         "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
                                         "layout (location = 2) in vec2 aTexCoord;\n"
                                         "uniform mat4 model;\n"
                                         "uniform mat4 view;\n"
                                         "uniform mat4 projection;\n"
                                         "//uniform float alpha;\n"
                                         "//out vec4 ourColor; // output a color to the fragment shader\n"
                                         "out vec2 TexCoord;\n"
                                         "void main()\n"
                                         "{\n"
                                         "   // note that we read the multiplication from right to left\n"
                                         "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                         "   //ourColor = vec4(aColor, alpha); // set ourColor to the input color we got from the vertex data\n"
                                         "   TexCoord = aTexCoord;\n"
                                         "}\0";

// Simplex 2D noise from https://lygia.xyz/generative/snoise
const char *fragmentShaderSourceSNoise = "#version 410 core\n"
                                         "out vec4 FragColor;\n"
                                         "//in vec4 ourColor;\n"
                                         "in vec2 TexCoord;\n"
                                         "uniform vec2 u_resolution;\n"
                                         "uniform float u_time;\n"
                                         "uniform float u_width;\n"
                                         "uniform float u_height;\n"
                                         "uniform float u_thickness;\n"
                                         "uniform vec3 u_velocity;\n"
                                         "uniform vec4 u_color;\n"
                                         "\n"
                                         "vec3 mod289(const in vec3 x) { return x - floor(x * (1. / 289.)) * 289.; }\n"
                                         "vec4 mod289(const in vec4 x) { return x - floor(x * (1. / 289.)) * 289.; }\n"
                                         "vec4 permute(const in vec4 v) { return mod289(((v * 34.0) + 1.0) * v); }\n"
                                         "vec4 taylorInvSqrt(in vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }\n"
                                         "\n"
                                         "float snoise(in vec3 v) {\n"
                                         "    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;\n"
                                         "    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);\n"
                                         "    \n"
                                         "    // First corner\n"
                                         "    vec3 i  = floor(v + dot(v, C.yyy) );\n"
                                         "    vec3 x0 =   v - i + dot(i, C.xxx) ;\n"
                                         "    \n"
                                         "    // Other corners\n"
                                         "    vec3 g = step(x0.yzx, x0.xyz);\n"
                                         "    vec3 l = 1.0 - g;\n"
                                         "    vec3 i1 = min( g.xyz, l.zxy );\n"
                                         "    vec3 i2 = max( g.xyz, l.zxy );\n"
                                         "    \n"
                                         "    //   x0 = x0 - 0.0 + 0.0 * C.xxx;\n"
                                         "    //   x1 = x0 - i1  + 1.0 * C.xxx;\n"
                                         "    //   x2 = x0 - i2  + 2.0 * C.xxx;\n"
                                         "    //   x3 = x0 - 1.0 + 3.0 * C.xxx;\n"
                                         "    vec3 x1 = x0 - i1 + C.xxx;\n"
                                         "    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y\n"
                                         "    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y\n"
                                         "    \n"
                                         "    // Permutations\n"
                                         "    i = mod289(i);\n"
                                         "    vec4 p = permute( permute( permute(\n"
                                         "                                 i.z + vec4(0.0, i1.z, i2.z, 1.0 ))\n"
                                         "                             + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))\n"
                                         "                     + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));\n"
                                         "    \n"
                                         "    // Gradients: 7x7 points over a square, mapped onto an octahedron.\n"
                                         "    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)\n"
                                         "    float n_ = 0.142857142857; // 1.0/7.0\n"
                                         "    vec3  ns = n_ * D.wyz - D.xzx;\n"
                                         "    \n"
                                         "    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)\n"
                                         "    \n"
                                         "    vec4 x_ = floor(j * ns.z);\n"
                                         "    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)\n"
                                         "    \n"
                                         "    vec4 x = x_ *ns.x + ns.yyyy;\n"
                                         "    vec4 y = y_ *ns.x + ns.yyyy;\n"
                                         "    vec4 h = 1.0 - abs(x) - abs(y);\n"
                                         "    \n"
                                         "    vec4 b0 = vec4( x.xy, y.xy );\n"
                                         "    vec4 b1 = vec4( x.zw, y.zw );\n"
                                         "    \n"
                                         "    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;\n"
                                         "    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;\n"
                                         "    vec4 s0 = floor(b0)*2.0 + 1.0;\n"
                                         "    vec4 s1 = floor(b1)*2.0 + 1.0;\n"
                                         "    vec4 sh = -step(h, vec4(0.0));\n"
                                         "    \n"
                                         "    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;\n"
                                         "    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;\n"
                                         "    \n"
                                         "    vec3 p0 = vec3(a0.xy,h.x);\n"
                                         "    vec3 p1 = vec3(a0.zw,h.y);\n"
                                         "    vec3 p2 = vec3(a1.xy,h.z);\n"
                                         "    vec3 p3 = vec3(a1.zw,h.w);\n"
                                         "    \n"
                                         "    //Normalise gradients\n"
                                         "    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));\n"
                                         "    p0 *= norm.x;\n"
                                         "    p1 *= norm.y;\n"
                                         "    p2 *= norm.z;\n"
                                         "    p3 *= norm.w;\n"
                                         "    \n"
                                         "    // Mix final noise value\n"
                                         "    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);\n"
                                         "    m = m * m;\n"
                                         "    return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));\n"
                                         "}\n"
                                         "\n"
                                         "void main(void) {\n"
                                         "    //vec4 color = vec4(vec3(0.0), 1.0);\n"
                                         "    //vec2 pixel = 0.3/u_resolution.xy;\n"
                                         "    //vec2 pixel = vec2(50.0/u_resolution.x, 150.0/u_resolution.y);\n"
                                         "    vec2 pixel = vec2(u_width/u_resolution.x, u_height/u_resolution.y);\n"
                                         "    vec2 st = TexCoord * pixel;\n"
                                         "    \n"
                                         "    //float d2 = snoise(vec2(st * 5. + u_time/5.0)) * 0.5 + 0.5;\n"
                                         "    //float d3 = snoise(vec3(st * 5. + u_time/10.0, u_time/10.0)) * 0.5 + 0.5;\n"
                                         "    //float d3 = snoise(vec3(st.x + u_time/3000.0, st.y + u_time/9000.0, u_time/30000.0)) * 0.4 + 0.6;\n"
                                         "    float d3 = snoise(vec3(st.x + u_time*u_velocity.x, st.y + u_time*u_velocity.y, u_time*u_velocity.z)) * (1.0 - u_thickness) + u_thickness;\n"
                                         "    \n"
                                         "    //color += mix(d2, d3, step(0.5, st.x));\n"
                                         "    \n"
                                         "    //FragColor = vec4(d3,d3,d3,d3);\n"
                                         "    //FragColor = d3 * u_color;\n"
                                         "    FragColor = vec4(u_color.rgb, d3 * u_color.a);\n"
                                         "}\n";

LayerEffect::LayerEffect(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
    _scene(nullptr),
    _shaderFragmentResolution(0),
    _shaderFragmentTime(0),
    _shaderFragmentWidth(0),
    _shaderFragmentHeight(0),
    _shaderFragmentThickness(0),
    _shaderFragmentVelocity(0),
    _shaderFragmentColor(0),
    _timerId(0),
    _milliseconds(0),
    _xVelocity(0.f),
    _yVelocity(0.f),
    _morphVelocity(0.f)
{
    setEffectWidth(LayerEffectSettings::defaultWidth);
    setEffectHeight(LayerEffectSettings::defaultHeight);
    setEffectThickness(LayerEffectSettings::defaultThickness);
    setEffectDirection(LayerEffectSettings::defaultDirection);
    setEffectSpeed(LayerEffectSettings::defaultSpeed);
    setEffectMorph(LayerEffectSettings::defaultMorph);
    setEffectColor(QColor(LayerEffectSettings::defaultColor));
}

LayerEffect::~LayerEffect()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerEffect::inputXML(const QDomElement &element, bool isImport)
{
    setEffectWidth(element.attribute("effectWidth", QString::number(LayerEffectSettings::defaultWidth)).toInt());
    setEffectHeight(element.attribute("effectHeight", QString::number(LayerEffectSettings::defaultHeight)).toInt());
    setEffectThickness(element.attribute("effectThickness", QString::number(LayerEffectSettings::defaultThickness)).toInt());
    setEffectDirection(element.attribute("effectDirection", QString::number(LayerEffectSettings::defaultDirection)).toInt());
    setEffectSpeed(element.attribute("effectSpeed", QString::number(LayerEffectSettings::defaultSpeed)).toInt());
    setEffectMorph(element.attribute("effectMorph", QString::number(LayerEffectSettings::defaultMorph)).toInt());
    setEffectColor(QColor(element.attribute("effectColor", QColor(LayerEffectSettings::defaultColor).name())));

    Layer::inputXML(element, isImport);
}

QRectF LayerEffect::boundingRect() const
{
    return QRectF();
}

QImage LayerEffect::getLayerIcon() const
{
    return QImage(":/img/data/icon_clouds.png");
}

bool LayerEffect::defaultShader() const
{
    return false;
}

DMHelper::LayerType LayerEffect::getType() const
{
    return DMHelper::LayerType_Effect;
}

Layer* LayerEffect::clone() const
{
    LayerEffect* newLayer = new LayerEffect(_name, _order);

    copyBaseValues(newLayer);

    return newLayer;
}

void LayerEffect::applyOrder(int order)
{
    if(_graphicsItem)
        _graphicsItem->setZValue(order);
}

void LayerEffect::applyLayerVisibleDM(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);
}

void LayerEffect::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerEffect::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;

    if(_graphicsItem)
        _graphicsItem->setOpacity(opacity);
}

void LayerEffect::applyPosition(const QPoint& position)
{
    if(_graphicsItem)
        _graphicsItem->setPos(position);
}

void LayerEffect::applySize(const QSize& size)
{
    if(_graphicsItem)
    {
        QImage effectImage = LayerEffectSettings::createNoiseImage(QSize(LAYEREFFECT_PREVIEWSIZE, LAYEREFFECT_PREVIEWSIZE),
                                                                   static_cast<qreal>(_effectWidth) / 10.f,
                                                                   static_cast<qreal>(_effectHeight) / 10.f,
                                                                   _effectColor,
                                                                   static_cast<qreal>(_effectThickness) / 100.f);
        _graphicsItem->setPixmap(QPixmap::fromImage(effectImage.scaled(size)));
    }
}

void LayerEffect::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    if(_graphicsItem)
    {
        qDebug() << "[LayerEffect] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    QImage effectImage = LayerEffectSettings::createNoiseImage(QSize(LAYEREFFECT_PREVIEWSIZE, LAYEREFFECT_PREVIEWSIZE),
                                                               static_cast<qreal>(_effectWidth) / 10.f,
                                                               static_cast<qreal>(_effectHeight) / 10.f,
                                                               _effectColor,
                                                               static_cast<qreal>(_effectThickness) / 100.f);
    _graphicsItem = scene->addPixmap(QPixmap::fromImage(effectImage.scaled(_size)));

    if(_graphicsItem)
    {
        _graphicsItem->setPos(_position);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());
    }

    Layer::dmInitialize(scene);
}

void LayerEffect::dmUninitialize()
{
    cleanupDM();
}

void LayerEffect::dmUpdate()
{
}

void LayerEffect::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    _scene = scene;
    Layer::playerGLInitialize(renderer, scene);

    if((_timerId == 0) && (renderer))
    {
        connect(this, &LayerEffect::update, renderer, &PublishGLRenderer::updateWidget);
        _timerId = startTimer(30);
    }
}

void LayerEffect::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerEffect::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);
    Q_UNUSED(projectionMatrix);

    if(_shaderProgramRGBA == 0)
        createShaders();

    if(_VAO == 0)
        createObjects();

    if((!QOpenGLContext::currentContext()) || (!functions))
        return;

    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if(!e)
        return;

    functions->glUseProgram(_shaderProgramRGBA);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);

    QMatrix4x4 modelMatrix;
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());

    functions->glUniform2f(_shaderFragmentResolution, 20.f, 20.f);
    functions->glUniform1f(_shaderFragmentTime, _milliseconds);
    functions->glUniform1f(_shaderFragmentWidth, static_cast<qreal>(_effectWidth) / 10.f);
    functions->glUniform1f(_shaderFragmentHeight, static_cast<qreal>(_effectHeight) / 10.f);
    functions->glUniform1f(_shaderFragmentThickness, static_cast<qreal>(_effectThickness) / 100.f);
    functions->glUniform3f(_shaderFragmentVelocity, _xVelocity, _yVelocity, _morphVelocity);
    functions->glUniform4f(_shaderFragmentColor, _effectColor.redF(), _effectColor.greenF(), _effectColor.blueF(), _opacityReference);

    e->glBindVertexArray(_VAO);
    functions->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void LayerEffect::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    destroyObjects();
    destroyShaders();
}

void LayerEffect::playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA)
{
    Q_UNUSED(programRGB);
    Q_UNUSED(modelMatrixRGB);
    Q_UNUSED(projectionMatrixRGB);
    Q_UNUSED(programRGBA);
    Q_UNUSED(modelMatrixRGBA);
    Q_UNUSED(projectionMatrixRGBA);
    Q_UNUSED(alphaRGBA);
}

bool LayerEffect::playerIsInitialized()
{
    return _scene != nullptr;
}

void LayerEffect::initialize(const QSize& sceneSize)
{
    if(getSize().isEmpty())
        setSize(sceneSize);
}

void LayerEffect::uninitialize()
{
}

void LayerEffect::editSettings()
{
    LayerEffectSettings* dlg = new LayerEffectSettings();

    connect(dlg, &LayerEffectSettings::effectWidthChanged, this, &LayerEffect::setEffectWidth);
    connect(dlg, &LayerEffectSettings::effectHeightChanged, this, &LayerEffect::setEffectHeight);
    connect(dlg, &LayerEffectSettings::effectThicknessChanged, this, &LayerEffect::setEffectThickness);
    connect(dlg, &LayerEffectSettings::effectDirectionChanged, this, &LayerEffect::setEffectDirection);
    connect(dlg, &LayerEffectSettings::effectSpeedChanged, this, &LayerEffect::setEffectSpeed);
    connect(dlg, &LayerEffectSettings::effectMorphChanged, this, &LayerEffect::setEffectMorph);
    connect(dlg, &LayerEffectSettings::effectColorChanged, this, &LayerEffect::setEffectColor);

    dlg->setEffectWidth(_effectWidth);
    dlg->setEffectHeight(_effectHeight);
    dlg->setEffectThickness(_effectThickness);
    dlg->setEffectDirection(_effectDirection);
    dlg->setEffectSpeed(_effectSpeed);
    dlg->setEffectMorph(_effectMorph);
    dlg->setEffectColor(_effectColor);

    dlg->exec();

    if(_graphicsItem)
    {
        QImage effectImage = LayerEffectSettings::createNoiseImage(QSize(LAYEREFFECT_PREVIEWSIZE, LAYEREFFECT_PREVIEWSIZE),
                                                                   static_cast<qreal>(_effectWidth) / 10.f,
                                                                   static_cast<qreal>(_effectHeight) / 10.f,
                                                                   _effectColor,
                                                                   static_cast<qreal>(_effectThickness) / 100.f);
        _graphicsItem->setPixmap(QPixmap::fromImage(effectImage.scaled(_size)));
    }

    dlg->deleteLater();
}

void LayerEffect::setEffectWidth(int width)
{
    if(_effectWidth == width)
        return;

    _effectWidth = width;
    emit dirty();
    emit update();
}

void LayerEffect::setEffectHeight(int height)
{
    if(_effectHeight == height)
        return;

    _effectHeight = height;
    emit dirty();
    emit update();
}

void LayerEffect::setEffectThickness(int thickness)
{
    if(_effectThickness == thickness)
        return;

    _effectThickness = thickness;
    emit dirty();
    emit update();
}

void LayerEffect::setEffectDirection(int direction)
{
    if(_effectDirection == direction)
        return;

    _effectDirection = direction;
    velocityChanged();
    emit dirty();
    emit update();
}

void LayerEffect::setEffectSpeed(int speed)
{
    if(_effectSpeed == speed)
        return;

    _effectSpeed = speed;
    velocityChanged();
    emit dirty();
    emit update();
}

void LayerEffect::setEffectMorph(int morph)
{
    if(_effectMorph == morph)
        return;

    _effectMorph = morph;
    velocityChanged();
    emit dirty();
    emit update();
}

void LayerEffect::setEffectColor(const QColor& color)
{
    if(_effectColor == color)
        return;

    _effectColor = color;
    emit dirty();
    emit update();
}

void LayerEffect::timerEvent(QTimerEvent *event)
{
    if((event) && (event->timerId() > 0) && (event->timerId() == _timerId))
    {
        _milliseconds += LAYEREFFECT_TIMERPERIOD;
        emit update();
    }
}

void LayerEffect::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    if(_effectWidth != LayerEffectSettings::defaultWidth)
        element.setAttribute("effectWidth", _effectWidth);

    if(_effectHeight != LayerEffectSettings::defaultHeight)
        element.setAttribute("effectHeight", _effectHeight);

    if(_effectThickness != LayerEffectSettings::defaultThickness)
        element.setAttribute("effectThickness", _effectThickness);

    if(_effectDirection != LayerEffectSettings::defaultDirection)
        element.setAttribute("effectDirection", _effectDirection);

    if(_effectSpeed != LayerEffectSettings::defaultSpeed)
        element.setAttribute("effectSpeed", _effectSpeed);

    if(_effectMorph != LayerEffectSettings::defaultMorph)
        element.setAttribute("effectMorph", _effectMorph);

    if(_effectColor != QColor(LayerEffectSettings::defaultColor))
        element.setAttribute("effectColor", _effectColor.name());

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerEffect::velocityChanged()
{
    _xVelocity = -qSin(qDegreesToRadians(_effectDirection)) * _effectSpeed * LAYEREFFECT_SPEED_FACTOR;
    _yVelocity = -qCos(qDegreesToRadians(_effectDirection)) * _effectSpeed * LAYEREFFECT_SPEED_FACTOR;
    _morphVelocity = _effectMorph * LAYEREFFECT_MORPH_FACTOR;
}

void LayerEffect::cleanupDM()
{
    if(!_graphicsItem)
        return;

    if(_graphicsItem->scene())
        _graphicsItem->scene()->removeItem(_graphicsItem);

    delete _graphicsItem;
    _graphicsItem = nullptr;
}

void LayerEffect::cleanupPlayer()
{
    disconnect(this, &LayerEffect::update, nullptr, nullptr);

    if(_timerId > 0)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    destroyObjects();
    destroyShaders();

    _scene = nullptr;
}

void LayerEffect::createShaders()
{
    // Create the local shader program
    if((!QOpenGLContext::currentContext()) || (_shaderProgramRGBA != 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    int  success;
    char infoLog[512];

    const char * vertexSource = vertexShaderSourceRGBColor;
    unsigned int vertexShaderRGBA;
    vertexShaderRGBA = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShaderRGBA, 1, &vertexSource, NULL);
    f->glCompileShader(vertexShaderRGBA);

    f->glGetShaderiv(vertexShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerEffect] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char * fragmentSource = fragmentShaderSourceSNoise;
    unsigned int fragmentShaderRGBA;
    fragmentShaderRGBA = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGBA, 1, &fragmentSource, NULL);
    f->glCompileShader(fragmentShaderRGBA);

    f->glGetShaderiv(fragmentShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerEffect] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGBA = f->glCreateProgram();

    f->glAttachShader(_shaderProgramRGBA, vertexShaderRGBA);
    f->glAttachShader(_shaderProgramRGBA, fragmentShaderRGBA);
    f->glLinkProgram(_shaderProgramRGBA);

    f->glGetProgramiv(_shaderProgramRGBA, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgramRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerEffect] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgramRGBA);
    f->glDeleteShader(vertexShaderRGBA);
    f->glDeleteShader(fragmentShaderRGBA);
    _shaderModelMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "model");
    _shaderProjectionMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "projection");
    _shaderFragmentResolution = f->glGetUniformLocation(_shaderProgramRGBA, "u_resolution");
    _shaderFragmentTime = f->glGetUniformLocation(_shaderProgramRGBA, "u_time");
    _shaderFragmentWidth = f->glGetUniformLocation(_shaderProgramRGBA, "u_width");
    _shaderFragmentHeight = f->glGetUniformLocation(_shaderProgramRGBA, "u_height");
    _shaderFragmentThickness = f->glGetUniformLocation(_shaderProgramRGBA, "u_thickness");
    _shaderFragmentVelocity = f->glGetUniformLocation(_shaderProgramRGBA, "u_velocity");
    _shaderFragmentColor = f->glGetUniformLocation(_shaderProgramRGBA, "u_color");

    qDebug() << "[LayerEffect] _shaderProgramRGBA: " << _shaderProgramRGBA << " _shaderModelMatrixRGBA: " << _shaderModelMatrixRGBA << " _shaderProjectionMatrixRGBA: " << _shaderProjectionMatrixRGBA; // << " _shaderAlphaRGBA: " << _shaderAlphaRGBA;
    qDebug() << "[LayerEffect] _shaderFragmentResolution: " << _shaderFragmentResolution << " _shaderFragmentTime: " << _shaderFragmentTime;

    QMatrix4x4 modelMatrix;
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    f->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBA, "view"), 1, GL_FALSE, viewMatrix.constData());
}

void LayerEffect::destroyShaders()
{
    if(QOpenGLContext::currentContext())
    {
        QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();

        if((e) && (_shaderProgramRGBA > 0))
            e->glDeleteProgram(_shaderProgramRGBA);
    }

    _shaderProgramRGBA = 0;
    _shaderModelMatrixRGBA = 0;
    _shaderProjectionMatrixRGBA = 0;
    _shaderFragmentResolution = 0;
    _shaderFragmentTime = 0;
    _shaderFragmentWidth = 0;
    _shaderFragmentHeight = 0;
    _shaderFragmentThickness = 0;
    _shaderFragmentVelocity = 0;
    _shaderFragmentColor = 0;
}

void LayerEffect::createObjects()
{
    if(!QOpenGLContext::currentContext())
        return;

    qDebug() << "[LayerEffect] Creating basic objects, size: " << _size;

    if(_size.isEmpty())
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    float vertices[] = {
        // positions                                                   // colors           // texture coords
        (float)_size.width() / 2,  (float)_size.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   // top right
        (float)_size.width() / 2, -(float)_size.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // bottom right
        -(float)_size.width() / 2, -(float)_size.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)_size.width() / 2,  (float)_size.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f    // top left
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);
    f->glGenBuffers(1, &_EBO);

    e->glBindVertexArray(_VAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // color attribute
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // texture attribute
    f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    f->glEnableVertexAttribArray(2);
}

void LayerEffect::destroyObjects()
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

            if(_EBO > 0)
                f->glDeleteBuffers(1, &_EBO);
        }
    }

    _VAO = 0;
    _VBO = 0;
    _EBO = 0;
}
