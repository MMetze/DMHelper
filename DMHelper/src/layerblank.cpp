#include "layerblank.h"
#include "dmconstants.h"
#include "publishglscene.h"
#include "publishglrect.h"
#include "layerscene.h"
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QDebug>

unsigned int LayerBlank::_shaderProgramRGBColorBlank = 0;
int LayerBlank::_shaderModelMatrixRGBColorBlank = -1;
int LayerBlank::_shaderProjectionMatrixRGBColorBlank = -1;
int LayerBlank::_shaderRGBColorBlank = -1;

LayerBlank::LayerBlank(const QString& name, const QColor& color, int order, QObject *parent) :
    Layer(name, order, parent),
    _graphicsItem(nullptr),
    _publishRect(nullptr),
    _scene(nullptr),
    _color(color)
{
}

LayerBlank::~LayerBlank()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerBlank::inputXML(const QDomElement &element, bool isImport)
{
    _color.setNamedColor(element.attribute("color", QString("#000000")));
    Layer::inputXML(element, isImport);
}

QRectF LayerBlank::boundingRect() const
{
    return QRectF(_position, _size);
}

QImage LayerBlank::getLayerIcon() const
{
    return QImage(":/img/data/icon_rectangle.png");
}

DMHelper::LayerType LayerBlank::getType() const
{
    return DMHelper::LayerType_Blank;
}

Layer* LayerBlank::clone() const
{
    LayerBlank* newLayer = new LayerBlank(_name, _color, _order);

    copyBaseValues(newLayer);

    return newLayer;
}

void LayerBlank::applyOrder(int order)
{
    if(_graphicsItem)
        _graphicsItem->setZValue(order);
}

void LayerBlank::applyLayerVisibleDM(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);
}

void LayerBlank::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerBlank::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;

    if(_graphicsItem)
        _graphicsItem->setOpacity(opacity);
}

void LayerBlank::applyPosition(const QPoint& position)
{
    if(_graphicsItem)
        _graphicsItem->setPos(position);

    if(_publishRect)
    {
        QPoint pointTopLeft = _scene ? _scene->getSceneRect().toRect().topLeft() : QPoint();
        _publishRect->setPosition(QPoint(pointTopLeft.x() + position.x(), -pointTopLeft.y() - position.y()));
    }
}

void LayerBlank::applySize(const QSize& size)
{
    if(_graphicsItem)
        _graphicsItem->setRect(0.0, 0.0, size.width(), size.height());

    if(_publishRect)
        _publishRect->setSize(size);
}

void LayerBlank::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    if(_graphicsItem)
    {
        qDebug() << "[LayerBlank] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    _graphicsItem = scene->addRect(QRectF(0.0, 0.0, _size.width(), _size.height()), QPen(), QBrush(_color));
    if(_graphicsItem)
    {
        _graphicsItem->setPos(_position);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());
    }

    Layer::dmInitialize(scene);
}

void LayerBlank::dmUninitialize()
{
    cleanupDM();
}

void LayerBlank::dmUpdate()
{
}

void LayerBlank::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    if((_shaderProgramRGBColorBlank == 0) || (_shaderModelMatrixRGBColorBlank == -1) || (_shaderProjectionMatrixRGBColorBlank == -1)/* || (_shaderRGBColorBlank == -1)*/)
    {
        if(!createShadersGL())
            return;
    }

    _scene = scene;

    if(!_publishRect)
    {
        _publishRect = new PublishGLRect(_color, QRectF(0.0, 0.0, _size.width(), _size.height()));
        connect(_publishRect, &PublishGLRect::changed, this, &LayerBlank::dirty);
    }

    Layer::playerGLInitialize(renderer, scene);
}

void LayerBlank::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerBlank::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if(!functions)
        return;

    functions->glUseProgram(_shaderProgramRGBColorBlank);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBColorBlank, 1, GL_FALSE, projectionMatrix);
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBColorBlank, 1, GL_FALSE, _publishRect->getMatrixData());
    functions->glUniform4f(_shaderRGBColorBlank, _color.redF(), _color.greenF(), _color.blueF(), _opacityReference);

    _publishRect->paintGL();

    functions->glUseProgram(_shaderProgramRGB);
}

void LayerBlank::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

bool LayerBlank::playerIsInitialized()
{
    return ((_shaderProgramRGBColorBlank != 0) && (_publishRect != nullptr));
}

void LayerBlank::initialize(const QSize& sceneSize)
{
    Q_UNUSED(sceneSize);
}

void LayerBlank::uninitialize()
{
}

void LayerBlank::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("color", _color.name());
    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool LayerBlank::createShadersGL()
{
    int  success;
    char infoLog[512];

    if(!QOpenGLContext::currentContext())
        return false;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return false;

    const char *vertexShaderSourceRGB = "#version 410 core\n"
        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "uniform vec4 inColor; // input RGBA color\n"
        "out vec4 ourColor; // output a color to the fragment shader\n"
        "void main()\n"
        "{\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0); \n"
        "   ourColor = inColor; // set ourColor to the input color\n"
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
        return false;
    }

    const char *fragmentShaderSourceRGB = "#version 410 core\n"
        "out vec4 FragColor;\n"
        "in vec4 ourColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = ourColor;\n"
        "}\0";

    unsigned int fragmentShaderRGB;
    fragmentShaderRGB = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGB, 1, &fragmentShaderSourceRGB, NULL);
    f->glCompileShader(fragmentShaderRGB);

    f->glGetShaderiv(fragmentShaderRGB, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGB, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return false;
    }

    _shaderProgramRGBColorBlank = f->glCreateProgram();

    f->glAttachShader(_shaderProgramRGBColorBlank, vertexShaderRGB);
    f->glAttachShader(_shaderProgramRGBColorBlank, fragmentShaderRGB);
    f->glLinkProgram(_shaderProgramRGBColorBlank);

    f->glGetProgramiv(_shaderProgramRGBColorBlank, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgramRGBColorBlank, 512, NULL, infoLog);
        qDebug() << "[LayerBlank] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return false;
    }

    f->glUseProgram(_shaderProgramRGBColorBlank);
    f->glDeleteShader(vertexShaderRGB);
    f->glDeleteShader(fragmentShaderRGB);
    _shaderModelMatrixRGBColorBlank = f->glGetUniformLocation(_shaderProgramRGBColorBlank, "model");
    _shaderProjectionMatrixRGBColorBlank = f->glGetUniformLocation(_shaderProgramRGBColorBlank, "projection");
    _shaderRGBColorBlank = f->glGetUniformLocation(_shaderProgramRGBColorBlank, "inColor");

    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    f->glUseProgram(_shaderProgramRGBColorBlank);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGBColorBlank, "texture1"), 0); // set it manually
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBColorBlank, "view"), 1, GL_FALSE, viewMatrix.constData());


    return true;
}

void LayerBlank::cleanupDM()
{
    if(!_graphicsItem)
        return;

    if(_graphicsItem->scene())
        _graphicsItem->scene()->removeItem(_graphicsItem);

    delete _graphicsItem;
    _graphicsItem = nullptr;
}

void LayerBlank::cleanupPlayer()
{
    delete _publishRect;
    _publishRect = nullptr;

    _scene = nullptr;
}
