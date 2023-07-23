#include "layer.h"
#include <QRectF>
#include <QImage>
#include <QDebug>

Layer::Layer(const QString& name, int order, QObject *parent) :
    DMHObjectBase{parent},
    _layerScene(nullptr),
    _name(name),
    _order(order),
    _layerVisible(true),
    _layerOpacity(1.0),
    _opacityReference(1.0),
    _position(),
    _size(),
    _shaderProgramRGB(0),
    _shaderProgramRGBA(0),
    _shaderModelMatrixRGBA(0),
    _shaderProjectionMatrixRGBA(0),
    _shaderAlphaRGBA(0)
{
}

Layer::~Layer()
{
    emit layerDestroyed(this);
}

QDomElement Layer::outputXML(QDomDocument &doc, QDomElement &parentElement, QDir& targetDirectory, bool isExport)
{
    QDomElement newElement = doc.createElement("layer");
    internalOutputXML(doc, newElement, targetDirectory, isExport);
    parentElement.appendChild(newElement);
    return newElement;
}

void Layer::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _name = element.attribute("layerName", QString("Layer"));
    _order = element.attribute("order", QString::number(0)).toInt();
    _layerVisible = static_cast<bool>(element.attribute("visible", QString::number(1)).toInt());
    _layerOpacity = element.attribute(QString("opacity"), QString::number(1.0)).toDouble();
    _opacityReference = 1.0;
    _position = QPoint(element.attribute("x", QString::number(0)).toInt(),
                       element.attribute("y", QString::number(0)).toInt());
    _size = QSize(element.attribute("w", QString::number(0)).toInt(),
                  element.attribute("h", QString::number(0)).toInt());

    DMHObjectBase::inputXML(element, isImport);
}

void Layer::postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport)
{
    Q_UNUSED(campaign);
    Q_UNUSED(element);
    Q_UNUSED(isImport);
}

QRectF Layer::boundingRect() const
{
    return QRectF();
}

LayerScene* Layer::getLayerScene() const
{
    return _layerScene;
}

QString Layer::getName() const
{
    return _name;
}

int Layer::getOrder() const
{
    return _order;
}

bool Layer::getLayerVisible() const
{
    return _layerVisible;
}

qreal Layer::getOpacity() const
{
    return _layerOpacity;
}

QPoint Layer::getPosition() const
{
    return _position;
}

QSize Layer::getSize() const
{
    return _size;
}

QImage Layer::getLayerIcon() const
{
    return QImage();
}

bool Layer::defaultShader() const
{
    return true;
}

DMHelper::LayerType Layer::getFinalType() const
{
    return getType();
}

void Layer::copyBaseValues(Layer *other) const
{
    other->_name = _name;
    other->_order = _order;
    other->_layerVisible = _layerVisible;
    other->_layerOpacity = _layerOpacity;
    other->_opacityReference = _opacityReference;
    other->_position = _position;
    other->_size = _size;
}

void Layer::dmInitialize(QGraphicsScene* scene)
{
    Q_UNUSED(scene);

    applyOrder(_order);
    applyLayerVisible(_layerVisible);
    applyOpacity(_layerOpacity);
    applyPosition(_position);
    applySize(_size);
}

void Layer::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    Q_UNUSED(renderer);
    Q_UNUSED(scene);

    applyOrder(_order);
    applyLayerVisible(_layerVisible);
    applyOpacity(_layerOpacity);
    applyPosition(_position);
    applySize(_size);
}

bool Layer::playerGLUpdate()
{
    return false;
}

void Layer::playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA)
{
    Q_UNUSED(modelMatrixRGB);
    Q_UNUSED(projectionMatrixRGB);

    _shaderProgramRGB = programRGB;
    _shaderProgramRGBA = programRGBA;
    _shaderProjectionMatrixRGBA = projectionMatrixRGBA;
    _shaderModelMatrixRGBA = modelMatrixRGBA;
    _shaderAlphaRGBA = alphaRGBA;
}

void Layer::aboutToDelete()
{
}

void Layer::setScale(int scale)
{
    Q_UNUSED(scale);
}

void Layer::setName(const QString& name)
{
    if(_name == name)
        return;

    _name = name;
    emit dirty();
}

void Layer::setLayerScene(LayerScene* layerScene)
{
    if(_layerScene == layerScene)
        return;

    _layerScene = layerScene;
    emit dirty();
}

void Layer::setOrder(int order)
{
    if(_order == order)
        return;

    applyOrder(order);
    _order = order;
    emit dirty();
    emit orderChanged(order);
}

void Layer::setLayerVisible(bool layerVisible)
{
    if(_layerVisible == layerVisible)
        return;

    applyLayerVisible(layerVisible);
    _layerVisible = layerVisible;
    emit dirty();
}

void Layer::setOpacity(qreal opacity)
{
    if(_layerOpacity == opacity)
        return;

    applyOpacity(opacity);
    _layerOpacity = opacity;
    emit dirty();
}

void Layer::setPosition(const QPoint& position)
{
    if(_position == position)
        return;

    applyPosition(position);
    _position = position;
    emit layerMoved(_position);
    emit dirty();
}

void Layer::setPosition(int x, int y)
{
    setPosition(QPoint(x, y));
}

void Layer::setSize(const QSize& size)
{
    if(_size == size)
        return;

    applySize(size);
    _size = size;
    emit layerResized(_size);
    emit dirty();
}

void Layer::setSize(int w, int h)
{
    setSize(QSize(w, h));
}

void Layer::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("type", getType());
    if(_name != QString("Layer"))
        element.setAttribute("layerName", _name);
    if(_order > 0)
        element.setAttribute("order", _order);
    if(!_layerVisible)
        element.setAttribute("visible", _layerVisible);
    if(_layerOpacity < 1.0)
        element.setAttribute("opacity", _layerOpacity);

    element.setAttribute("x", _position.x());
    element.setAttribute("y", _position.y());
    element.setAttribute("w", _size.width());
    element.setAttribute("h", _size.height());

    DMHObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}
