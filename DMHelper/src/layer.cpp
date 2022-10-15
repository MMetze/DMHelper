#include "layer.h"
#include <QRectF>
#include <QImage>

Layer::Layer(const QString& name, int order, QObject *parent) :
    QObject{parent},
    _name(name),
    _order(order),
    _layerVisible(true)
{
}

Layer::~Layer()
{
}

void Layer::outputXML(QDomDocument &doc, QDomElement &parentElement, QDir& targetDirectory, bool isExport)
{
    QDomElement newElement = doc.createElement("layer");
    internalOutputXML(doc, newElement, targetDirectory, isExport);
    parentElement.appendChild(newElement);
}

void Layer::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.hasAttribute("layerName"))
        _name = element.attribute("layerName");
    if(element.hasAttribute("order"))
        _order = element.attribute("order", QString::number(0)).toInt();
    if(element.hasAttribute("visible"))
        _layerVisible = static_cast<bool>(element.attribute("visible",QString::number(1)).toInt());
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

QImage Layer::getLayerIcon() const
{
    return QImage();
}

bool Layer::playerGLUpdate()
{
    return false;
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

    _order = order;
    emit dirty();
    emit orderChanged(order);
}

void Layer::setLayerVisible(bool layerVisible)
{
    if(_layerVisible == layerVisible)
        return;

    _layerVisible = layerVisible;
    emit dirty();
}

void Layer::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("type", getType());
    element.setAttribute("layerName", _name);
    element.setAttribute("order", _order);
    element.setAttribute("visible", _layerVisible);
}
