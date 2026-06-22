#include "layerdrawstate.h"
#include "dmconstants.h"
#include <QDomDocument>
#include <QDebug>

LayerDrawState::LayerDrawState(QObject *parent) :
    QObject{parent},
    _drawObjects{}
{}

LayerDrawState::~LayerDrawState()
{
    qDeleteAll(_drawObjects);
}

void LayerDrawState::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _drawObjects.clear();

    QDomElement objectElement = element.firstChildElement("draw-object");
    while(!objectElement.isNull())
    {
        int type = objectElement.attribute("type").toInt();
        LayerDrawObject* object = nullptr;

        // Create object based on type
        switch(type)
        {
            case DMHelper::ActionType_Path:
                object = new LayerDrawObjectPath();
                break;
            case DMHelper::ActionType_Line:
                object = new LayerDrawObjectLine();
                break;
            case DMHelper::ActionType_Rect:
                object = new LayerDrawObjectRect();
                break;
            case DMHelper::ActionType_Ellipse:
                object = new LayerDrawObjectEllipse();
                break;
            case DMHelper::ActionType_Text:
                object = new LayerDrawObjectText();
                break;
            default:
                qDebug() << "Unknown draw object type:" << type;
                break;
        }

        if(object)
        {
            object->inputXML(objectElement, isImport);
            appendObject(object);
        }

        objectElement = objectElement.nextSiblingElement("draw-object");
    }
}

void LayerDrawState::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    for(const auto& objectPtr : std::as_const(_drawObjects))
    {
        QDomElement objectElement = doc.createElement("draw-object");
        objectPtr->outputXML(doc, objectElement, targetDirectory, isExport);
        element.appendChild(objectElement);
    }
}

int LayerDrawState::indexOf(const QUuid& id) const
{
    for(int i = 0; i < _drawObjects.size(); ++i)
    {
        if((_drawObjects.at(i)) && (_drawObjects.at(i)->getId() == id))
            return i;
    }

    return -1;
}

int LayerDrawState::indexOf(LayerDrawObject* object) const
{
    if(!object)
        return -1;

    for(int i = 0; i < _drawObjects.size(); ++i)
    {
        if(_drawObjects.at(i) == object)
            return i;
    }

    return -1;
}

int LayerDrawState::appendObject(LayerDrawObject* object)
{
    if(!object)
        return -1;

    int index = _drawObjects.size();

    _drawObjects.append(object);

    emit objectAdded(object, index);
    return index;
}

int LayerDrawState::insertObject(int index, LayerDrawObject* object)
{
    if(!object)
        return -1;

    if(index < 0)
        index = 0;
    else if(index >= _drawObjects.size())
        index = _drawObjects.size();

    _drawObjects.insert(index, object);
    emit objectAdded(object, index);

    return index;
}

LayerDrawObject* LayerDrawState::takeObject(const QUuid& id)
{
    int index = indexOf(id);
    if(index < 0)
        return nullptr;

    LayerDrawObject* object = _drawObjects.at(index);

    _drawObjects.removeAt(index);

    emit objectRemoved(object, index);

    return object;
}

QList<LayerDrawObject*> LayerDrawState::getObjects() const
{
    return _drawObjects;
}
