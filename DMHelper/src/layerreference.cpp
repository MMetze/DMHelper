#include "layerreference.h"
#include "campaign.h"
#include "map.h"
#include "encounterbattle.h"
#include "battledialogmodel.h"
#include <QPainter>
#include <QDebug>

LayerReference::LayerReference(CampaignObjectBase* referenceObject, Layer* referenceLayer, int order, QObject *parent) :
    Layer{QString(), order, parent},
    _referenceObject(referenceObject),
    _referenceObjectId(),
    _referenceLayer(nullptr),
    _referenceLayerId()
{
    setReferenceLayer(referenceLayer);
}

LayerReference::~LayerReference()
{
    setReferenceLayer(nullptr); // make sure no further destroy messages are propagated
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
                setReferenceLayer(map->getLayerScene().findLayer(_referenceLayerId));
        }
        else if(_referenceObject->getObjectType() == DMHelper::CampaignType_Battle)
        {
            EncounterBattle* battle = dynamic_cast<EncounterBattle*>(_referenceObject);
            if((battle) && (battle->getBattleDialogModel()))
                setReferenceLayer(battle->getBattleDialogModel()->getLayerScene().findLayer(_referenceLayerId));
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

    copyReferenceValues();
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
    QImage linkImage(":/img/data/icon_linkglow.png");

    if(!_referenceLayer)
        return linkImage;

    QImage layerIcon = _referenceLayer->getLayerIcon().scaled(linkImage.size());
    QPainter p;
    p.begin(&layerIcon);
        p.drawImage(0, 0, linkImage);
    p.end();

    return layerIcon;
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

Layer* LayerReference::getFinalLayer()
{
    return getReferenceLayer();
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
    if(_referenceLayer)
        _referenceLayer->applyPosition(position);
}

void LayerReference::applySize(const QSize& size)
{
    if(_referenceLayer)
        _referenceLayer->applySize(size);
}

void LayerReference::applyLayerVisibleDM(bool layerVisible)
{
    if(_referenceLayer)
        _referenceLayer->applyLayerVisibleDM(layerVisible);
}

void LayerReference::applyLayerVisiblePlayer(bool layerVisible)
{
    if(_referenceLayer)
        _referenceLayer->applyLayerVisiblePlayer(layerVisible);
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

Layer* LayerReference::getReferenceLayer() const
{
    return _referenceLayer;
}

CampaignObjectBase* LayerReference::getReferenceObject() const
{
    return _referenceObject;
}

void LayerReference::clearReference()
{
    disconnect(_referenceLayer, &Layer::layerDestroyed, this, &LayerReference::handleReferenceDestroyed);
    _referenceLayer = nullptr;
    _referenceObject = nullptr;
}

void LayerReference::dmInitialize(QGraphicsScene* scene)
{
    if(_referenceLayer)
        _referenceLayer->dmInitialize(scene);

    copyReferenceValues();

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

void LayerReference::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    if(_referenceLayer)
        _referenceLayer->playerGLInitialize(renderer, scene);

    Layer::playerGLInitialize(renderer, scene);
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
    DMH_DEBUG_OPENGL_PAINTGL();

    if(_referenceLayer)
        _referenceLayer->playerGLPaint(functions, defaultModelMatrix, projectionMatrix);
}

void LayerReference::playerGLResize(int w, int h)
{
    if(_referenceLayer)
        _referenceLayer->playerGLResize(w, h);
}

void LayerReference::playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA)
{
    if(_referenceLayer)
        _referenceLayer->playerSetShaders(programRGB, modelMatrixRGB, projectionMatrixRGB, programRGBA, modelMatrixRGBA, projectionMatrixRGBA, alphaRGBA);
}

bool LayerReference::playerIsInitialized()
{
    return _referenceLayer ? _referenceLayer->playerIsInitialized() : false;
}

void LayerReference::initialize(const QSize& sceneSize)
{
    if(_referenceLayer)
        _referenceLayer->initialize(sceneSize);
}

void LayerReference::uninitialize()
{
    if(_referenceLayer)
        _referenceLayer->uninitialize();
}

void LayerReference::aboutToDelete()
{
    if(_referenceLayer)
        disconnect(_referenceLayer, &Layer::layerDestroyed, this, &LayerReference::handleReferenceDestroyed);
}

void LayerReference::setScale(int scale)
{
    if(_referenceLayer)
        _referenceLayer->setScale(scale);
}

void LayerReference::handleReferenceDestroyed(Layer *layer)
{
    if((layer) && (layer == _referenceLayer))
    {
        clearReference();
        emit referenceDestroyed(this, layer);
    }
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

    // Reference Layers shouldn't retain size and position, those come from the reference object
    if(element.hasAttribute("x"))
        element.removeAttribute("x");
    if(element.hasAttribute("y"))
        element.removeAttribute("y");
    if(element.hasAttribute("w"))
        element.removeAttribute("w");
    if(element.hasAttribute("h"))
        element.removeAttribute("h");
}

void LayerReference::setReferenceLayer(Layer* layer)
{
    if(layer)
    {
        _referenceLayer = layer;
        connect(_referenceLayer, &Layer::layerDestroyed, this, &LayerReference::handleReferenceDestroyed);
    }
    else if(_referenceLayer)
    {
        disconnect(_referenceLayer, &Layer::layerDestroyed, this, &LayerReference::handleReferenceDestroyed);
        _referenceLayer = nullptr;
    }
}

void LayerReference::copyReferenceValues()
{
    if(!_referenceLayer)
        return;

    _position = _referenceLayer->getPosition();
    _size = _referenceLayer->getSize();
}
