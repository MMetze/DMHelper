#include "layer.h"
#include <QRectF>
#include <QImage>
#include <QDebug>

Layer::Layer(const QString& name, int order, QObject *parent) :
    DMHObjectBase{parent},
    _name(name),
    _order(order),
    _layerVisible(true),
    _opacity(1.0)
{
}

Layer::~Layer()
{
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
    _opacity = element.attribute(QString("opacity"), QString::number(1.0)).toDouble();

    DMHObjectBase::inputXML(element, isImport);
}

QRectF Layer::boundingRect() const
{
    return QRectF();
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
    return _opacity;
}

QImage Layer::getLayerIcon() const
{
    return QImage();
}

bool Layer::defaultShader() const
{
    return true;
}

void Layer::copyBaseValues(Layer *other) const
{
    other->_name = _name;
    other->_order = _order;
    other->_layerVisible = _layerVisible;
    other->_opacity = _opacity;
}

void Layer::dmInitialize(QGraphicsScene& scene)
{
    Q_UNUSED(scene);
    applyOrder(_order);
    applyLayerVisible(_layerVisible);
    applyOpacity(_opacity);
}

bool Layer::playerGLUpdate()
{
    return false;
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
    if(_opacity == opacity)
        return;

    applyOpacity(opacity);
    _opacity = opacity;
    emit dirty();
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
    if(_opacity < 1.0)
        element.setAttribute("opacity", _opacity);

    DMHObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}
