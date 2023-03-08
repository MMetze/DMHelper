#include "layerreference.h"
#include "campaign.h"
#include "map.h"
#include "encounterbattle.h"
#include "battledialogmodel.h"
#include <QDebug>

LayerReference::LayerReference(CampaignObjectBase* referenceObject, Layer* referenceLayer, int order, QObject *parent) :
    Layer{QString(), order, parent},
    _referenceObject(referenceObject),
    _referenceObjectId(),
    _referenceLayer(referenceLayer),
    _referenceLayerId()
{
}

LayerReference::~LayerReference()
{
    _referenceLayer = nullptr; // make sure no further destroy messages are propagated
}

void LayerReference::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _referenceObjectId = QUuid(element.attribute("objectId"));
    _referenceLayerId = QUuid(element.attribute("layerId"));

    Layer::inputXML(element, isImport);
}

void LayerReference::postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport)
{
    Q_UNUSED(element);
    Q_UNUSED(isImport);

    if(!campaign)
        return;

    if(!_referenceObject)
    {
        if(_referenceObjectId.isNull())
        {
            qDebug() << "[LayerReference] Reference Object and Object ID null for object " << getID().toString();
            return;
        }

        _referenceObject = campaign->getObjectById(_referenceObjectId);
        if(!_referenceObject)
        {
            qDebug() << "[LayerReference] Not able to find reference object " << _referenceObjectId << " for object " << getID().toString();
            return;
        }
    }

    if(!_referenceLayer)
    {
        if(_referenceLayerId.isNull())
        {
            qDebug() << "[LayerReference] Reference Layer and Layer ID null for object " << getID().toString();
            return;
        }

        if(_referenceObject->getObjectType() == DMHelper::CampaignType_Map)
        {
            Map* map = dynamic_cast<Map*>(_referenceObject);
            if(map)
                _referenceLayer = map->getLayerScene().findLayer(_referenceLayerId);
        }
        else if(_referenceObject->getObjectType() == DMHelper::CampaignType_Battle)
        {
            EncounterBattle* battle = dynamic_cast<EncounterBattle*>(_referenceObject);
            if((battle) && (battle->getBattleDialogModel()))
                _referenceLayer = battle->getBattleDialogModel()->getLayerScene().findLayer(_referenceLayerId);
        }
        else
        {
            qDebug() << "[LayerReference] Unexpected type of reference object " << _referenceObjectId << " for object " << getID().toString();
            return;
        }

        if(!_referenceLayer)
        {
            qDebug() << "[LayerReference] Not able to find reference layer " << _referenceLayerId << " for object " << getID().toString() << " in reference object " << _referenceObjectId;
            return;
        }
    }

    connect(_referenceLayer, &Layer::layerDestroyed, this, &LayerReference::handleReferenceDestroyed);
}

QRectF LayerReference::boundingRect() const
{
    return _referenceLayer ? _referenceLayer->boundingRect() : QRectF();
}

QString LayerReference::getName() const
{
    return _referenceLayer ? _referenceLayer->getName() : QString();
}

QImage LayerReference::getLayerIcon() const
{
    return QImage(":/img/data/icon_link.png");
}

bool LayerReference::defaultShader() const
{
    return _referenceLayer ? _referenceLayer->defaultShader() : true;
}

DMHelper::LayerType LayerReference::getType() const
{
    return DMHelper::LayerType_Reference;
}

DMHelper::LayerType LayerReference::getFinalType() const
{
    return getReferencedType();
}

Layer* LayerReference::clone() const
{
    LayerReference* newLayer = new LayerReference(_referenceObject, _referenceLayer, _order);

    copyBaseValues(newLayer);
    newLayer->_referenceObjectId = _referenceObjectId;
    newLayer->_referenceLayerId = _referenceLayerId;

    return newLayer;
}

void LayerReference::applyOrder(int order)
{
    if(_referenceLayer)
        _referenceLayer->applyOrder(order);
}

void LayerReference::applyPosition(const QPoint& position)
{

}

void LayerReference::applySize(const QSize& size)
{

}

void LayerReference::applyLayerVisible(bool layerVisible)
{
    if(_referenceLayer)
        _referenceLayer->applyLayerVisible(layerVisible);
}

void LayerReference::applyOpacity(qreal opacity)
{
    if(_referenceLayer)
        _referenceLayer->applyOpacity(opacity);
}

DMHelper::LayerType LayerReference::getReferencedType() const
{
    return _referenceLayer ? _referenceLayer->getType() : DMHelper::LayerType_Unknown;
}

Layer* LayerReference::getReferenceLayer()
{
    return _referenceLayer;
}

CampaignObjectBase* LayerReference::getReferenceObject()
{
    return _referenceObject;
}

void LayerReference::dmInitialize(QGraphicsScene* scene)
{
    if(_referenceLayer)
        _referenceLayer->dmInitialize(scene);

    Layer::dmInitialize(scene);
}

void LayerReference::dmUninitialize()
{
    if(_referenceLayer)
        _referenceLayer->dmUninitialize();
}

void LayerReference::dmUpdate()
{
    if(_referenceLayer)
        _referenceLayer->dmUpdate();
}

void LayerReference::playerGLInitialize(PublishGLScene* scene)
{
    if(_referenceLayer)
        _referenceLayer->playerGLInitialize(scene);

    Layer::playerGLInitialize(scene);
}

void LayerReference::playerGLUninitialize()
{
    if(_referenceLayer)
        _referenceLayer->playerGLUninitialize();
}

bool LayerReference::playerGLUpdate()
{
    return _referenceLayer ? _referenceLayer->playerGLUpdate() : false;
}

void LayerReference::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    if(_referenceLayer)
        _referenceLayer->playerGLPaint(functions, defaultModelMatrix, projectionMatrix);
}

void LayerReference::playerGLResize(int w, int h)
{
    if(_referenceLayer)
        _referenceLayer->playerGLResize(w, h);
}

bool LayerReference::playerIsInitialized()
{
    return _referenceLayer ? _referenceLayer->playerIsInitialized() : false;
}

void LayerReference::initialize(const QSize& layerSize)
{
    if(_referenceLayer)
        _referenceLayer->initialize(layerSize);
}

void LayerReference::uninitialize()
{
    if(_referenceLayer)
        _referenceLayer->uninitialize();
}

void LayerReference::setScale(int scale)
{
    if(_referenceLayer)
        _referenceLayer->setScale(scale);
}

void LayerReference::handleReferenceDestroyed(Layer *layer)
{
    if((layer) && (layer == _referenceLayer))
        emit referenceDestroyed(this);
}

void LayerReference::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    if((_referenceObjectId.isNull()) && (_referenceObject))
        _referenceObjectId = _referenceObject->getID();

    if(!_referenceObjectId.isNull())
        element.setAttribute("objectId", _referenceObjectId.toString());

    if((_referenceLayerId.isNull()) && (_referenceLayer))
        _referenceLayerId = _referenceLayer->getID();

    if(!_referenceLayerId.isNull())
        element.setAttribute("layerId", _referenceLayerId.toString());

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}
