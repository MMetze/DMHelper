#include "layertokens.h"
#include "battledialogmodel.h"
#include "battledialogmodelcombatant.h"
#include "unselectedpixmap.h"
#include "publishglbattletoken.h"
#include "publishglbattleeffect.h"
#include "campaign.h"
#include "character.h"
#include "bestiary.h"
#include "monster.h"
#include "monsterclass.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodelmonsterclass.h"
#include "battledialogmodelmonstercombatant.h"
#include "battledialogmodeleffectfactory.h"
#include "battledialogmodeleffectobject.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPainter>
#include <QtGlobal>
#include <QDebug>

LayerTokens::LayerTokens(BattleDialogModel* model, const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    _glScene(nullptr),
    _playerInitialized(false),
    _model(),
    _combatants(),
    _combatantTokens(),
    _combatantIconHash(),
    _combatantTokenHash(),
    _effects(),
    _effectTokens(),
    _effectIconHash(),
    _effectTokenHash(),
    _scale(DMHelper::STARTING_GRID_SCALE)
{
    setModel(model);
}

LayerTokens::~LayerTokens()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerTokens::inputXML(const QDomElement &element, bool isImport)
{
    Layer::inputXML(element, isImport);
}

void LayerTokens::postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport)
{
    if((!_model) || (!campaign))
        return;

    QDomElement combatantsElement = element.firstChildElement("combatants");
    if(!combatantsElement.isNull())
    {
        QDomElement combatantElement = combatantsElement.firstChildElement("battlecombatant");
        while(!combatantElement.isNull())
        {
            BattleDialogModelCombatant* combatant = nullptr;
            int combatantType = combatantElement.attribute("type", QString::number(DMHelper::CombatantType_Base)).toInt();
            if(combatantType == DMHelper::CombatantType_Character)
            {
                QUuid combatantId = QUuid(combatantElement.attribute("combatantId"));
                Character* character = campaign->getCharacterById(combatantId);
                if(!character)
                    character = campaign->getNPCById(combatantId);

                if(character)
                    combatant = new BattleDialogModelCharacter(character);
                else
                    qDebug() << "[LayerTokens] Unknown character ID type found: " << combatantId;
            }
            else if(combatantType == DMHelper::CombatantType_Monster)
            {
                int monsterType = combatantElement.attribute("monsterType", QString::number(BattleDialogModelMonsterBase::BattleMonsterType_Base)).toInt();
                if(monsterType == BattleDialogModelMonsterBase::BattleMonsterType_Combatant)
                {
                    QUuid combatantId = QUuid(combatantElement.attribute("combatantId"));
                    Monster* monster = dynamic_cast<Monster*>(_model->getCombatantById(combatantId));
                    if(monster)
                        combatant = new BattleDialogModelMonsterCombatant(monster);
                    else
                        qDebug() << "[LayerTokens] Unknown monster ID type found: " << combatantId << " for battle";// " << battleId;
                }
                else if(monsterType == BattleDialogModelMonsterBase::BattleMonsterType_Class)
                {
                    QString monsterClassName = combatantElement.attribute("monsterClass");
                    MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(monsterClassName);
                    if(monsterClass)
                        combatant = new BattleDialogModelMonsterClass(monsterClass);
                    else
                        qDebug() << "[LayerTokens] Unknown monster class type found: " << monsterClassName;
                }
                else
                {
                    qDebug() << "[LayerTokens] Invalid monster type found: " << monsterType;
                }
            }
            else
            {
                qDebug() << "[LayerTokens] Invalid combatant type found: " << combatantType;
            }

            if(combatant)
            {
                combatant->inputXML(combatantElement, isImport);
                addCombatant(combatant);
            }

            combatantElement = combatantElement.nextSiblingElement("battlecombatant");
        }
    }

    QDomElement effectsElement = element.firstChildElement("effects");
    if(!effectsElement.isNull())
    {
        QDomElement effectElement = effectsElement.firstChildElement();
        while(!effectElement.isNull())
        {
            BattleDialogModelEffect* newEffect = BattleDialogModelEffectFactory::createEffect(effectElement, isImport);
            if(newEffect)
            {
                QDomElement effectChildElement = effectElement.firstChildElement();
                if(!effectChildElement.isNull())
                {
                    BattleDialogModelEffect* childEffect = BattleDialogModelEffectFactory::createEffect(effectChildElement, isImport);
                    if(childEffect)
                        newEffect->addObject(childEffect);
                }
                addEffect(newEffect);
            }

            effectElement = effectElement.nextSiblingElement();
        }
    }

    // Now go through the combatants and effects again to reestablish any existing links
    for(int i = 0; i < _combatants.count(); ++i)
    {
        if((_combatants.at(i)) && (_combatants.at(i)->getLinkedObject()))
            linkedObjectChanged(_combatants.at(i), nullptr);
    }

    for(int i = 0; i < _effects.count(); ++i)
    {
        if((_effects.at(i)) && (_effects.at(i)->getLinkedObject()))
            linkedObjectChanged(_effects.at(i), nullptr);
    }
}

QRectF LayerTokens::boundingRect() const
{
    return QRectF();
}

bool LayerTokens::defaultShader() const
{
    return false;
}

QImage LayerTokens::getLayerIcon() const
{
    return QImage(":/img/data/icon_contentcharacter.png");
}

DMHelper::LayerType LayerTokens::getType() const
{
    return DMHelper::LayerType_Tokens;
}

Layer* LayerTokens::clone() const
{
    LayerTokens* newLayer = new LayerTokens(_model, _name, _order);

    copyBaseValues(newLayer);

    // TODO: Layers - Needs to actually clone lists of combatants and effects...
    return newLayer;
}

void LayerTokens::applyOrder(int order)
{
    qreal combatantOrder = getIconOrder(DMHelper::CampaignType_BattleContentCombatant, order);
    foreach(QGraphicsPixmapItem* pixmapItem, _combatantIconHash)
    {
        if(pixmapItem)
            pixmapItem->setZValue(combatantOrder);
    }

    qreal effectOrder = getIconOrder(DMHelper::CampaignType_BattleContentEffect, order);
    foreach(QGraphicsItem* graphicsItem, _effectIconHash)
    {
        if(graphicsItem)
            graphicsItem->setZValue(effectOrder);
    }
}

void LayerTokens::applyLayerVisibleDM(bool layerVisible)
{
    if(!_model)
        return;

    applyCombatantVisibility(layerVisible, _model->getShowAlive(), _model->getShowDead());
    applyEffectVisibility(layerVisible && _model->getShowEffects());
}

void LayerTokens::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible)
}

void LayerTokens::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;

    foreach(QGraphicsPixmapItem* pixmapItem, _combatantIconHash)
    {
        if(pixmapItem)
            pixmapItem->setOpacity(opacity);
    }

    foreach(QGraphicsItem* graphicsItem, _effectIconHash)
    {
        if(graphicsItem)
            graphicsItem->setOpacity(opacity);
    }
}

void LayerTokens::applyPosition(const QPoint& position)
{
    QPoint delta = position - _position;

    foreach(BattleDialogModelCombatant* combatant, _combatants)
    {
        if(combatant)
        {
            QGraphicsPixmapItem* combatantIcon = _combatantIconHash.value(combatant);
            if(combatantIcon)
                combatantIcon->setPos(combatant->getPosition() + delta);
        }
    }

    foreach(BattleDialogModelEffect* effect, _effects)
    {
        if(effect)
        {
            QGraphicsItem* effectIcon = findEffectItem(effect);
            if(effectIcon)
                effectIcon->setPos(effect->getPosition() + delta);
        }
    }
}

void LayerTokens::applySize(const QSize& size)
{
    Q_UNUSED(size);
}

const QList<BattleDialogModelCombatant*> LayerTokens::getCombatants() const
{
    return _combatants;
}

QList<BattleDialogModelCombatant*> LayerTokens::getCombatants()
{
    return _combatants;
}

QList<QGraphicsPixmapItem*> LayerTokens::getCombatantItems()
{
    return _combatantIconHash.values();
}

const QList<BattleDialogModelEffect*> LayerTokens::getEffects() const
{
    return _effects;
}

QList<BattleDialogModelEffect*> LayerTokens::getEffects()
{
    return _effects;
}

QList<QGraphicsItem*> LayerTokens::getEffectItems()
{
    return _effectIconHash.values();
}

BattleDialogModelObject* LayerTokens::getObjectById(QUuid id)
{
    for(int i = 0; i < _combatants.count(); ++i)
    {
        if((_combatants.at(i)) && (_combatants.at(i)->getID() == id))
            return _combatants.at(i);
    }

    for(int i = 0; i < _effects.count(); ++i)
    {
        if((_effects.at(i)) && (_effects.at(i)->getID() == id))
            return _effects.at(i);
    }

    return nullptr;
}

bool LayerTokens::containsObject(BattleDialogModelObject* battleObject)
{
    return ((containsEffect(dynamic_cast<BattleDialogModelEffect*>(battleObject))) ||
            (containsCombatant(dynamic_cast<BattleDialogModelCombatant*>(battleObject))));
}

QGraphicsItem* LayerTokens::getObjectItem(BattleDialogModelObject* battleObject)
{
    return findGraphicsItem(battleObject);
}

int LayerTokens::getScale() const
{
    return _scale;
}

void LayerTokens::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    for(int i = 0; i < _combatants.count(); ++i)
    {
        createCombatantIcon(scene, _combatants.at(i));
    }

    for(int i = 0; i < _effects.count(); ++i)
    {
        createEffectIcon(scene, _effects.at(i));
    }

    Layer::dmInitialize(scene);
}

void LayerTokens::dmUninitialize()
{
    cleanupDM();
}

void LayerTokens::dmUpdate()
{
}

void LayerTokens::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    if(!scene)
        return;

    _glScene = scene;

    for(int i = 0; i < _combatants.count(); ++i)
    {
        BattleDialogModelCombatant* combatant = _combatants.at(i);
        if(combatant)
        {
            PublishGLBattleToken* combatantToken = new PublishGLBattleToken(_glScene, combatant);
            // TODO: Layers - is this needed?
            /*
            BattleDialogModelCharacter* characterCombatant = dynamic_cast<BattleDialogModelCharacter*>(combatant);
            if((characterCombatant) && (characterCombatant->getCharacter()) && (characterCombatant->getCharacter()->isInParty()))
                combatantToken->setPC(true);
            */
            // TODO: Layers - how do selections work?
            /*
            if(combatant->getSelected())
                combatantToken->addEffect(*_selectionToken);
            */

            _combatantTokenHash.insert(combatant, combatantToken);
            // TODO: Layers
            //connect(combatantToken, &PublishGLBattleObject::changed, this, &PublishGLBattleRenderer::updateWidget);
            //connect(combatantToken, &PublishGLBattleToken::selectionChanged, this, &PublishGLBattleRenderer::tokenSelectionChanged);
        }
    }

    for(int i = 0; i < _effects.count(); ++i)
    {
        BattleDialogModelEffect* effect = _effects.at(i);
        if(effect)
        {
            PublishGLBattleEffect* effectToken = new PublishGLBattleEffect(_glScene, effect);
            _effectTokenHash.insert(effect, effectToken);
        }
    }

    _playerInitialized = true;

    Layer::playerGLInitialize(renderer, scene);
}

void LayerTokens::playerGLUninitialize()
{
    _playerInitialized = false;
    cleanupPlayer();
}

void LayerTokens::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if(!_model)
        return;

    QMatrix4x4 localMatrix;

    functions->glUseProgram(_shaderProgramRGBA);
    functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    // Draw Effects first
    if(_model->getShowEffects())
    {
        foreach(BattleDialogModelEffect* effect, _effects)
        {
            if((effect) && (effect->getEffectVisible()))
            {
                PublishGLBattleEffect* effectToken = _effectTokenHash.value(effect);
                if(!effectToken)
                {
                    effectToken = new PublishGLBattleEffect(_glScene, effect);
                    _effectTokenHash.insert(effect, effectToken);
                }

                localMatrix = effectToken->getMatrix();
                localMatrix.translate(_position.x(), _position.y());
                functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, localMatrix.constData());
                functions->glUniform1f(_shaderAlphaRGBA, effectToken->getEffectAlpha() * _opacityReference);
                effectToken->paintGL();
            }
        }
    }

    // Draw the combatants
    foreach(BattleDialogModelCombatant* combatant, _combatants)
    {
        if((combatant) && (combatant->getKnown()) && (combatant->getShown()) &&
           ((_model->getShowDead()) || (combatant->getHitPoints() > 0)) &&
           ((_model->getShowAlive()) || (combatant->getHitPoints() <= 0)))
        {
            PublishGLBattleToken* combatantToken = _combatantTokenHash.value(combatant);
            if(!combatantToken)
            {
                combatantToken = new PublishGLBattleToken(_glScene, combatant);
                _combatantTokenHash.insert(combatant, combatantToken);
            }

            localMatrix = combatantToken->getMatrix();
            localMatrix.translate(_position.x(), _position.y());
            functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, localMatrix.constData());
            functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);
            combatantToken->paintGL();
            combatantToken->paintEffects(_shaderModelMatrixRGBA);

            emit postCombatantDrawGL(functions, combatant, combatantToken);
        }
    }

    functions->glUseProgram(_shaderProgramRGB);
}

void LayerTokens::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

bool LayerTokens::playerIsInitialized()
{
    return _playerInitialized;
}

void LayerTokens::initialize(const QSize& sceneSize)
{
    Q_UNUSED(sceneSize);
}

void LayerTokens::uninitialize()
{
}

void LayerTokens::setScale(int scale)
{
    if(_scale == scale)
        return;

    _scale = scale;

    // Trigger each combatant to be rescaled
    qreal scaleFactor;
    foreach(BattleDialogModelCombatant* combatant, _combatants)
    {
        if(combatant)
        {
            PublishGLBattleToken* combatantToken = _combatantTokenHash.value(combatant);
            if(combatantToken)
                combatantToken->combatantMoved();

            QGraphicsPixmapItem* combatantIcon = _combatantIconHash.value(combatant);
            if(combatantIcon)
            {
                scaleFactor = (static_cast<qreal>(scale-2)) * combatant->getSizeFactor() / static_cast<qreal>(qMax(combatantIcon->pixmap().width(), combatantIcon->pixmap().height()));
                combatantIcon->setScale(scaleFactor);
            }
        }
    }

    // Trigger each effect to be rescaled
    foreach(BattleDialogModelEffect* effect, _effects)
    {
        BattleDialogModelEffect* effectKey = findEffectKey(effect);
        if(effectKey)
        {
            PublishGLBattleEffect* effectToken = _effectTokenHash.value(effect);
            if(effectToken)
                effectToken->effectMoved();

            effectChanged(effectKey);
        }
    }
}

void LayerTokens::setModel(BattleDialogModel* model)
{
    if((_model) || (!model))
        return;

    _model = model;
    connect(_model, &BattleDialogModel::showEffectsChanged, this, &LayerTokens::effectVisibilityChanged);
    connect(_model, &BattleDialogModel::showAliveChanged, this, &LayerTokens::aliveVisibilityChanged);
    connect(_model, &BattleDialogModel::showDeadChanged, this, &LayerTokens::deadVisibilityChanged);
}

void LayerTokens::addCombatant(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    if(_combatants.contains(combatant))
        return;

    combatant->setLayer(this);
    _combatants.append(combatant);
    _model->appendCombatantToList(combatant);
    connect(combatant, &BattleDialogModelObject::linkChanged, this, &LayerTokens::linkedObjectChanged);
    connect(combatant, &BattleDialogModelCombatant::dirty, this, &LayerTokens::dirty);
    connect(combatant, &BattleDialogModelEffect::objectMoved, this, &LayerTokens::combatantMoved);
    connect(combatant, &BattleDialogModelCombatant::conditionsChanged, this, &LayerTokens::combatantConditionChanged);
    connect(this, &LayerTokens::objectRemoved, combatant, &BattleDialogModelObject::objectRemoved);

    if((getLayerScene()) && (getLayerScene()->getDMScene()))
    {
        QGraphicsPixmapItem* combatantItem = createCombatantIcon(getLayerScene()->getDMScene(), combatant);
        combatantItem->setZValue(getIconOrder(DMHelper::CampaignType_BattleContentCombatant, getOrder()));
        combatantItem->setVisible(getLayerVisibleDM());
        combatantItem->setOpacity(_opacityReference);
    }
}

void LayerTokens::removeCombatant(BattleDialogModelCombatant* combatant)
{
    if((!combatant) || (!_model))
        return;

    disconnect(combatant, &BattleDialogModelObject::linkChanged, this, &LayerTokens::linkedObjectChanged);
    disconnect(combatant, &BattleDialogModelCombatant::dirty, this, &LayerTokens::dirty);
    disconnect(combatant, &BattleDialogModelEffect::objectMoved, this, &LayerTokens::combatantMoved);
    disconnect(combatant, &BattleDialogModelCombatant::conditionsChanged, this, &LayerTokens::combatantConditionChanged);
    disconnect(this, &LayerTokens::objectRemoved, combatant, &BattleDialogModelObject::objectRemoved);

    if(!_combatants.removeOne(combatant))
        return;

    combatant->setLayer(nullptr);
    _model->removeCombatantFromList(combatant);
    emit objectRemoved(combatant);

    QGraphicsPixmapItem* combatantIcon = _combatantIconHash.take(combatant);
    if(combatantIcon)
    {
        QGraphicsScene* scene = combatantIcon->scene();
        if(scene)
            scene->removeItem(combatantIcon);

        delete combatantIcon;
    }

    PublishGLBattleToken* combatantToken = _combatantTokenHash.take(combatant);
    delete combatantToken;
}

bool LayerTokens::containsCombatant(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return false;

    return _combatants.contains(combatant);
}

QGraphicsItem* LayerTokens::getCombatantItem(BattleDialogModelCombatant* combatant)
{
    return findCombatantItem(combatant);
}

PublishGLBattleToken* LayerTokens::getCombatantToken(BattleDialogModelCombatant* combatant)
{
    return combatant ? _combatantTokenHash.value(combatant) : nullptr;
}

BattleDialogModelCombatant* LayerTokens::getCombatantFromItem(QGraphicsPixmapItem* item)
{
    return _combatantIconHash.key(item, nullptr);
}

void LayerTokens::addEffect(BattleDialogModelEffect* effect)
{
    if((!effect) || (!_model))
        return;

    if(_effects.contains(effect))
        return;

    effect->setLayer(this);
    _effects.append(effect);
    connect(effect, &BattleDialogModelEffect::effectChanged, this, &LayerTokens::effectChanged);
    connect(effect, &BattleDialogModelEffect::objectMoved, this, &LayerTokens::effectMoved);
    connect(this, &LayerTokens::objectRemoved, effect, &BattleDialogModelObject::objectRemoved);
    connect(effect, &BattleDialogModelObject::linkChanged, this, &LayerTokens::linkedObjectChanged);
    connect(effect, &BattleDialogModelEffect::dirty, this, &LayerTokens::dirty);
    _model->appendEffectToList(effect);

    if((getLayerScene()) && (getLayerScene()->getDMScene()))
    {
        QGraphicsItem* effectIcon = createEffectIcon(getLayerScene()->getDMScene(), effect);
        if(effectIcon)
        {
            effectIcon->setZValue(getIconOrder(DMHelper::CampaignType_BattleContentEffect, getOrder()));
            effectIcon->setVisible(getLayerVisibleDM());
            effectIcon->setOpacity(_opacityReference);
            effectIcon->setPos(effect->getPosition());
        }
    }
}

void LayerTokens::removeEffect(BattleDialogModelEffect* effect)
{
    if((!effect) || (!_model))
        return;

    disconnect(effect, &BattleDialogModelEffect::effectChanged, this, &LayerTokens::effectChanged);
    disconnect(effect, &BattleDialogModelEffect::objectMoved, this, &LayerTokens::effectMoved);
    disconnect(this, &LayerTokens::objectRemoved, effect, &BattleDialogModelObject::objectRemoved);
    disconnect(effect, &BattleDialogModelObject::linkChanged, this, &LayerTokens::linkedObjectChanged);

    if(!_effects.removeOne(effect))
        return;

    effect->setLayer(nullptr);

    _model->removeEffectFromList(effect);
    emit objectRemoved(effect);

    QList<Layer*> tokenLayers = _layerScene->getLayers(DMHelper::LayerType_Tokens);
    for(int i = 0; i < tokenLayers.count(); ++i)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(tokenLayers.at(i));
        if(tokenLayer)
        {
            QList<BattleDialogModelCombatant*> combatants = tokenLayer->getCombatants();
            foreach(BattleDialogModelCombatant* combatant, combatants)
            {
                QGraphicsPixmapItem* combatantItem = dynamic_cast<QGraphicsPixmapItem*>(tokenLayer->getCombatantItem(combatant));
                if(combatantItem)
                    removeSpecificEffectFromItem(combatantItem, effect);
            }
        }
    }

    BattleDialogModelEffect* effectKey = findEffectKey(effect);
    if(!effectKey)
        return;

    QGraphicsItem* effectIcon = _effectIconHash.take(effectKey);
    if(effectIcon)
    {
        QGraphicsScene* scene = effectIcon->scene();
        if(scene)
            scene->removeItem(effectIcon);

        delete effectIcon;
    }

    PublishGLBattleEffect* effectToken = _effectTokenHash.take(effectKey);
    delete effectToken;
}

void LayerTokens::effectReady(BattleDialogModelEffect* effect)
{
    Q_UNUSED(effect);
}

bool LayerTokens::containsEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return false;

    return _effects.contains(effect);
}

QGraphicsItem* LayerTokens::getEffectItem(BattleDialogModelEffect* effect)
{
    return findEffectItem(effect);
}

BattleDialogModelEffect* LayerTokens::getEffectFromItem(QGraphicsItem* item)
{
    return _effectIconHash.key(item, nullptr);
}

void LayerTokens::refreshEffects()
{
    foreach(BattleDialogModelEffect* effect, _effects)
    {
        effectMoved(effect);
    }
}

void LayerTokens::handleCombatantSelected(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    QGraphicsPixmapItem* item = _combatantIconHash.value(combatant);
    if(item)
        item->setSelected(combatant->getSelected());
}

void LayerTokens::combatantMoved(BattleDialogModelObject* object)
{
    if(!_layerScene)
        return;

    BattleDialogModelCombatant* combatant = dynamic_cast<BattleDialogModelCombatant*>(object);
    if(!combatant)
        return;

    QGraphicsPixmapItem* combatantItem = _combatantIconHash.value(combatant);
    if(!combatantItem)
        return;

    PublishGLBattleToken* combatantToken = _combatantTokenHash.value(combatant);

    QList<Layer*> tokenLayers = _layerScene->getLayers(DMHelper::LayerType_Tokens);
    for(int i = 0; i < tokenLayers.count(); ++i)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(tokenLayers.at(i));
        if(tokenLayer)
        {
            QList<BattleDialogModelEffect*> effects = tokenLayer->getEffects();
            foreach(BattleDialogModelEffect* effect, effects)
            {
                QGraphicsItem* effectItem = dynamic_cast<QGraphicsItem*>(tokenLayer->getEffectItem(effect));
                if(effectItem)
                {
                    QGraphicsItem* collisionEffect = effectItem;
                    foreach(QGraphicsItem* childEffect, effectItem->childItems())
                    {
                        if((childEffect) && (childEffect->data(BATTLE_DIALOG_MODEL_EFFECT_ROLE).toInt() == BattleDialogModelEffect::BattleDialogModelEffectRole_Area))
                            collisionEffect = childEffect;
                    }

                    bool collision = isItemInEffectArea(combatantItem, collisionEffect);
                    if(!collision)
                    {
                        removeSpecificEffectFromItem(combatantItem, effect);
                        removeEffectFromToken(combatantToken, effect);
                    }
                    else
                    {
                        applyEffectToItem(combatantItem, effect);
                        applyEffectToToken(combatantToken, effect);
                    }
                }
            }
        }
    }
}

void LayerTokens::combatantConditionChanged(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    QGraphicsPixmapItem* item = _combatantIconHash.value(combatant);
    if(!item)
        return;

    QPixmap pix = generateCombatantPixmap(combatant);
    if(pix.isNull())
        return;

    item->setPixmap(pix);
    item->setOffset(-static_cast<qreal>(pix.width())/2.0, -static_cast<qreal>(pix.height())/2.0);
    applyCombatantTooltip(item, combatant);
}

void LayerTokens::aliveVisibilityChanged(bool showAlive)
{
    if(!_model)
        return;

    applyCombatantVisibility(getLayerVisibleDM(), showAlive, _model->getShowDead());
}

void LayerTokens::deadVisibilityChanged(bool showDead)
{
    if(!_model)
        return;

    applyCombatantVisibility(getLayerVisibleDM(), _model->getShowAlive(), showDead);
}

void LayerTokens::effectChanged(BattleDialogModelEffect* effect)
{
    if((!effect) || (!_model))
        return;

    // Changes to the player item will be directly handled through the signal
    QGraphicsItem* graphicsItem = _effectIconHash.value(effect);
    if(graphicsItem)
        effect->applyEffectValues(*graphicsItem, _scale);

    // Remove current effect markers from all combatants
    QList<Layer*> tokenLayers = _layerScene->getLayers(DMHelper::LayerType_Tokens);
    for(int i = 0; i < tokenLayers.count(); ++i)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(tokenLayers.at(i));
        if(tokenLayer)
        {
            QList<BattleDialogModelCombatant*> combatants = tokenLayer->getCombatants();
            foreach(BattleDialogModelCombatant* combatant, combatants)
            {
                QGraphicsPixmapItem* combatantItem = dynamic_cast<QGraphicsPixmapItem*>(tokenLayer->getCombatantItem(combatant));
                if(combatantItem)
                {
                    removeSpecificEffectFromItem(combatantItem, effect);
                    removeEffectFromToken(tokenLayer->getCombatantToken(combatant), effect);
                }
            }
        }
    }

    // Refresh the effect markers
    effectMoved(effect);
}

void LayerTokens::effectMoved(BattleDialogModelObject* object)
{
    if(!_layerScene)
        return;

    BattleDialogModelEffect* movedEffect = dynamic_cast<BattleDialogModelEffect*>(object);
    BattleDialogModelEffect* keyEffect = findEffectKey(movedEffect);
    if(!keyEffect)
        return;

    QGraphicsItem* effectItem = _effectIconHash.value(keyEffect);
    if(!effectItem)
        return;

    QGraphicsItem* collisionEffect = effectItem;
    foreach(QGraphicsItem* childEffect, effectItem->childItems())
    {
        if((childEffect) && (childEffect->data(BATTLE_DIALOG_MODEL_EFFECT_ROLE).toInt() == BattleDialogModelEffect::BattleDialogModelEffectRole_Area))
            collisionEffect = childEffect;
    }

    QList<Layer*> tokenLayers = _layerScene->getLayers(DMHelper::LayerType_Tokens);
    for(int i = 0; i < tokenLayers.count(); ++i)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(tokenLayers.at(i));
        if(tokenLayer)
        {
            QList<BattleDialogModelCombatant*> combatants = tokenLayer->getCombatants();
            foreach(BattleDialogModelCombatant* combatant, combatants)
            {
                QGraphicsPixmapItem* combatantItem = dynamic_cast<QGraphicsPixmapItem*>(tokenLayer->getCombatantItem(combatant));
                if(combatantItem)
                {
                    if((!movedEffect->getEffectActive()) || (!isItemInEffectArea(combatantItem, collisionEffect)))
                    {
                        removeSpecificEffectFromItem(combatantItem, movedEffect);
                        removeEffectFromToken(tokenLayer->getCombatantToken(combatant), movedEffect);
                    }
                    else
                    {
                        applyEffectToItem(combatantItem, movedEffect);
                        applyEffectToToken(tokenLayer->getCombatantToken(combatant), movedEffect);
                    }
                }
            }
        }
    }
}

void LayerTokens::effectVisibilityChanged(bool showEffects)
{
    if(!_model)
        return;

    applyEffectVisibility(getLayerVisibleDM() && showEffects);
}

void LayerTokens::linkedObjectChanged(BattleDialogModelObject* object, BattleDialogModelObject* previousLink)
{
    if(!object)
        return;

    if(object->getLinkedObject())
    {
        connect(object->getLinkedObject(), &BattleDialogModelObject::objectMovedDelta, object, &BattleDialogModelObject::parentMoved);
        connect(object, &BattleDialogModelObject::objectMoved, this, &LayerTokens::linkedObjectMoved);
    }
    else
    {
        disconnect(previousLink, &BattleDialogModelObject::objectMovedDelta, object, &BattleDialogModelObject::parentMoved);
        disconnect(object, &BattleDialogModelObject::objectMoved, this, &LayerTokens::linkedObjectMoved);
    }
}

void LayerTokens::linkedObjectMoved(BattleDialogModelObject* object)
{
    if(!object)
        return;

    QGraphicsItem* objectItem = findGraphicsItem(object);
    if((!objectItem) || (objectItem->pos() == object->getPosition()))
        return;

    objectItem->setPos(object->getPosition());
}

void LayerTokens::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    if(_combatants.count() > 0)
    {
        QDomElement combatantsElement = doc.createElement("combatants");
        foreach(BattleDialogModelCombatant* combatant, _combatants)
        {
            if(combatant)
                combatant->outputXML(doc, combatantsElement, targetDirectory, isExport);
        }
        element.appendChild(combatantsElement);
    }

    if(_effects.count() > 0)
    {
        QDomElement effectsElement = doc.createElement("effects");
        foreach(BattleDialogModelEffect* effect, _effects)
        {
            if(effect)
                effect->outputXML(doc, effectsElement, targetDirectory, isExport);
        }
        element.appendChild(effectsElement);
    }

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerTokens::cleanupDM()
{
    if(!_combatantIconHash.isEmpty())
    {
        foreach(QGraphicsPixmapItem* pixmapItem, _combatantIconHash)
        {
            if(pixmapItem)
                pixmapItem->setParentItem(nullptr);
        }
    }

    if(!_effectIconHash.isEmpty())
    {
        foreach(QGraphicsItem* graphicsItem, _effectIconHash)
        {
            if(graphicsItem)
                graphicsItem->setParentItem(nullptr);
        }
    }

    if(!_combatantIconHash.isEmpty())
    {
        foreach(QGraphicsPixmapItem* pixmapItem, _combatantIconHash)
        {
            delete pixmapItem;
        }

        _combatantIconHash.clear();
    }

    if(!_effectIconHash.isEmpty())
    {
        foreach(QGraphicsItem* graphicsItem, _effectIconHash)
        {
            delete graphicsItem;
        }

        _effectIconHash.clear();
    }
}

QGraphicsPixmapItem* LayerTokens::createCombatantIcon(QGraphicsScene* scene, BattleDialogModelCombatant* combatant)
{
    if((!combatant) || (_combatantIconHash.contains(combatant)))
        return nullptr;

    QPixmap pix = generateCombatantPixmap(combatant);
    if(pix.isNull())
        return nullptr;

    QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(pix, combatant);
    scene->addItem(pixmapItem);
    pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
    pixmapItem->setPos(combatant->getPosition() + _position);
    pixmapItem->setRotation(combatant->getRotation());
    pixmapItem->setOffset(-static_cast<qreal>(pix.width())/2.0, -static_cast<qreal>(pix.height())/2.0);
    qreal sizeFactor = combatant->getSizeFactor();
    qreal scaleFactor = (static_cast<qreal>(_scale-2)) * sizeFactor / static_cast<qreal>(qMax(pix.width(), pix.height()));
    pixmapItem->setScale(scaleFactor);
    applyCombatantTooltip(pixmapItem, combatant);

    qDebug() << "[LayerTokens] combatant icon added " << combatant->getName() << ", scale " << scaleFactor;

    qreal gridSize = (static_cast<qreal>(_scale)) / scaleFactor;
    qreal gridOffset = gridSize * static_cast<qreal>(sizeFactor) / 2.0;
    QGraphicsRectItem* rect = new QGraphicsRectItem(0, 0, gridSize * sizeFactor, gridSize * static_cast<qreal>(sizeFactor));
    rect->setPos(-gridOffset, -gridOffset);
    // TODO: Layers
    //rect->setData(BattleDialogItemChild_Index, BattleDialogItemChild_Area);
    rect->setParentItem(pixmapItem);
    rect->setVisible(false);
    qDebug() << "[LayerTokens] created " << pixmapItem << " with area child " << rect;

    // TODO: Layers
    // applyPersonalEffectToItem(pixmapItem);

    _combatantIconHash.insert(combatant, pixmapItem);
    //linkedObjectChanged(combatant, nullptr);

    // TODO: Layers
    //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(handleCombatantMoved(BattleDialogModelCombatant*)), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
    //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(updateHighlights()), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
    //connect(combatant, SIGNAL(combatantSelected(BattleDialogModelCombatant*)), this, SLOT(handleCombatantSelected(BattleDialogModelCombatant*)));
    connect(combatant, &BattleDialogModelCombatant::combatantSelected, this, &LayerTokens::handleCombatantSelected);

    return pixmapItem;
}

QGraphicsItem* LayerTokens::createEffectIcon(QGraphicsScene* scene, BattleDialogModelEffect* effect)
{
    if((!effect) || (!scene))
        return nullptr;

    QGraphicsItem* result = nullptr;

    if(effect->children().count() == 1)
        result = addSpellEffect(scene, effect);

    if(!result)
        result = addEffectShape(scene, effect);

    if(result)
        effectMoved(effect);

    return result;
}

QGraphicsItem* LayerTokens::addEffectShape(QGraphicsScene* scene, BattleDialogModelEffect* effect)
{
    if((!_model) || (!scene) || (!effect) || (_effectIconHash.contains(effect)))
        return nullptr;

    QGraphicsItem* shape = effect->createEffectShape(_scale);
    if(shape)
    {
        shape->setPos(effect->getPosition() + _position);
        shape->setToolTip(QString("<b>") + effect->getName() + QString("</b> (") + getName() + QString(")"));
        scene->addItem(shape);
        _effectIconHash.insert(effect, shape);
    }

    return shape;
}

QGraphicsItem* LayerTokens::addSpellEffect(QGraphicsScene* scene, BattleDialogModelEffect* effect)
{
    if((!_model) || (!effect))
        return nullptr;

    QList<CampaignObjectBase*> childEffects = effect->getChildObjects();
    if(childEffects.count() != 1)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: cannot add spell effect because it does not have exactly one child object!";
        return nullptr;
    }

    BattleDialogModelEffectObject* tokenEffect = dynamic_cast<BattleDialogModelEffectObject*>(childEffects.at(0));
    if(!tokenEffect)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: cannot add spell effect because it's child is not an effect!";
        return nullptr;
    }

    QGraphicsPixmapItem* tokenItem = dynamic_cast<QGraphicsPixmapItem*>(addEffectShape(scene, tokenEffect));
    if(!tokenItem)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to add the spell effect's token object to the scene!";
        return nullptr;
    }

    QGraphicsItem* effectItem = effect->createEffectShape(1.0);
    if(!effectItem)
    {
        qDebug() << "[Battle Dialog Scene] ERROR: unable to create the spell effect's basic shape!";
        return nullptr;
    }

    if((effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Cone) ||
       (effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Line))
    {
        tokenItem->setOffset(QPointF(-tokenItem->boundingRect().width() / 2.0, 0.0));
    }
    else if(effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Cube)
    {
        tokenItem->setOffset(QPointF(0.0, 0.0));
    }

    if(effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Radius)
    {
        tokenItem->setOffset(QPointF(-tokenItem->boundingRect().width() / 2.0,
                                     -tokenItem->boundingRect().height() / 2.0));
        effectItem->setScale(1.0 / (2.0 * tokenEffect->getImageScaleFactor()));
    }
    else
    {
        effectItem->setScale(1.0 / tokenEffect->getImageScaleFactor());
    }

    effectItem->setParentItem(tokenItem);
    effectItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    effectItem->setFlag(QGraphicsItem::ItemIsMovable, false);
    effectItem->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
    effectItem->setData(BATTLE_DIALOG_MODEL_EFFECT_ROLE, BattleDialogModelEffect::BattleDialogModelEffectRole_Area);
    effectItem->setPos(QPointF(0.0, 0.0));

    return tokenItem;
}

QGraphicsItem* LayerTokens::findCombatantItem(BattleDialogModelCombatant* combatant)
{
    return combatant ? _combatantIconHash.value(combatant) : nullptr;
}

BattleDialogModelEffect* LayerTokens::findEffectKey(BattleDialogModelEffect* effect)
{
    if(!effect)
        return nullptr;

    QList<CampaignObjectBase*> childObjects = effect->getChildObjects();
    if(childObjects.count() == 1)
        return dynamic_cast<BattleDialogModelEffect*>(childObjects.at(0));
    else
        return effect;
}

QGraphicsItem* LayerTokens::findEffectItem(BattleDialogModelEffect* effect)
{
    if(!effect)
        return nullptr;

    BattleDialogModelEffect* effectKey = findEffectKey(effect);

    return effectKey ? _effectIconHash.value(effectKey) : nullptr;
}

QGraphicsItem* LayerTokens::findGraphicsItem(BattleDialogModelObject* object)
{
    if(!object)
        return nullptr;

    QGraphicsItem* combatantItem = findCombatantItem(dynamic_cast<BattleDialogModelCombatant*>(object));
    if(combatantItem)
        return combatantItem;

    return findEffectItem(dynamic_cast<BattleDialogModelEffect*>(object));
}

qreal LayerTokens::getTotalScale(QGraphicsItem& item)
{
    qreal result = item.scale();

    QGraphicsItem* parentItem = item.parentItem();
    while(parentItem)
    {
        result *= parentItem->scale();
        parentItem = parentItem->parentItem();
    }

    return result;
}

bool LayerTokens::isItemInEffect(QGraphicsPixmapItem* item, QGraphicsItem* effect)
{
    if((!item) || (!effect))
        return false;

    QGraphicsItem* collisionEffect = effect;
    foreach(QGraphicsItem* childEffect, effect->childItems())
    {
        if(childEffect->data(BATTLE_DIALOG_MODEL_EFFECT_ROLE).toInt() == BattleDialogModelEffect::BattleDialogModelEffectRole_Area)
            collisionEffect = childEffect;
    }

    return isItemInEffectArea(item, collisionEffect);
}

bool LayerTokens::isItemInEffectArea(QGraphicsPixmapItem* item, QGraphicsItem* collisionEffect)
{
    if((!item) || (!collisionEffect))
        return false;

    if(item->childItems().count() > 0)
    {
        foreach(QGraphicsItem* childItem, item->childItems())
        {
            if((childItem) && (childItem->data(BATTLE_CONTENT_CHILD_INDEX).toInt() == BattleDialogItemChild_Area))
                return childItem->collidesWithItem(collisionEffect);
        }
    }

    return item->collidesWithItem(collisionEffect);
}

void LayerTokens::removeSpecificEffectFromItem(QGraphicsPixmapItem* item, BattleDialogModelEffect* effect)
{
    if((!item) || (!effect))
        return;

    qulonglong effectId = reinterpret_cast<qulonglong>(effect);

    foreach(QGraphicsItem* childItem, item->childItems())
    {
        if((childItem) &&
           (childItem->data(BATTLE_CONTENT_CHILD_INDEX).toInt() == BattleDialogItemChild_AreaEffect) &&
           (childItem->data(BATTLE_CONTENT_CHILD_ID).toULongLong() == effectId))
        {
            childItem->setParentItem(nullptr);
            delete childItem;
        }
    }
}

void LayerTokens::applyEffectToItem(QGraphicsPixmapItem* item, BattleDialogModelEffect* effect)
{
    if((!item) || (!effect))
        return;

    qulonglong effectId = reinterpret_cast<qulonglong>(effect);

    foreach(QGraphicsItem* childItem, item->childItems())
    {
        if((childItem) &&
           (childItem->data(BATTLE_CONTENT_CHILD_INDEX).toInt() == BattleDialogItemChild_AreaEffect) &&
           (childItem->data(BATTLE_CONTENT_CHILD_ID).toULongLong() == effectId))
        {
            // This effect is already applied to this object
            return;
        }
    }

    QColor ellipseColor = effect->getColor();
    if(!ellipseColor.isValid())
        return;

    QRect itemRect = item->boundingRect().toRect();
    int maxSize = qMax(itemRect.width(), itemRect.height());

    QGraphicsEllipseItem* effectItem = new QGraphicsEllipseItem(-maxSize/2, -maxSize/2, maxSize, maxSize);
    effectItem->setPen(QPen(ellipseColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    ellipseColor.setAlpha(128);
    effectItem->setBrush(QBrush(ellipseColor));
    effectItem->setData(BATTLE_CONTENT_CHILD_INDEX, BattleDialogItemChild_AreaEffect);
    effectItem->setData(BATTLE_CONTENT_CHILD_ID, effectId);
    effectItem->setParentItem(item);
}

void LayerTokens::applyEffectToToken(PublishGLBattleToken* token, BattleDialogModelEffect* effect)
{
    if((!token) || (!effect))
        return;

    // Don't re-add the effect to the token
    if(token->hasEffectHighlight(effect))
        return;

    token->addEffectHighlight(effect);
}

void LayerTokens::removeEffectFromToken(PublishGLBattleToken* token, BattleDialogModelEffect* effect)
{
    if((!token) || (!effect))
        return;

    token->removeEffectHighlight(effect);
}

QPixmap LayerTokens::generateCombatantPixmap(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return QPixmap();

    QPixmap result = combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
    if(combatant->hasCondition(Combatant::Condition_Unconscious))
    {
        QImage originalImage = result.toImage();
        QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
        result = QPixmap::fromImage(grayscaleImage);
    }

    applySingleCombatantVisibility(combatant, getLayerVisibleDM(), _model->getShowAlive(), _model->getShowDead());

    Combatant::drawConditions(&result, combatant->getConditions());

    return result;
}

void LayerTokens::applyCombatantTooltip(QGraphicsItem* item, BattleDialogModelCombatant* combatant)
{
    if((!item) || (!combatant))
        return;

    QString itemTooltip = QString("<b>") + combatant->getName() + QString("</b> (") + getName() + QString(")");
    QStringList conditionString = Combatant::getConditionString(combatant->getConditions());
    if(conditionString.count() > 0)
        itemTooltip += QString("<p>") + conditionString.join(QString("<br/>"));

    item->setToolTip(itemTooltip);
}

void LayerTokens::applyCombatantVisibility(bool layerVisible, bool aliveVisible, bool deadVisible)
{

    foreach(BattleDialogModelCombatant* combatant, _combatants)
    {
        applySingleCombatantVisibility(combatant, layerVisible, aliveVisible, deadVisible);
    }
}

void LayerTokens::applySingleCombatantVisibility(BattleDialogModelCombatant* combatant, bool layerVisible, bool aliveVisible, bool deadVisible)
{
    if(!combatant)
        return;

    QGraphicsPixmapItem* pixmapItem = _combatantIconHash.value(combatant);
    if(!pixmapItem)
        return;

    bool combatantVisible = layerVisible && (((combatant->getHitPoints() > 0) || (combatant->getCombatantType() == DMHelper::CombatantType_Character)) ? aliveVisible : deadVisible);
    pixmapItem->setVisible(combatantVisible);
}

void LayerTokens::applyEffectVisibility(bool visible)
{
    foreach(QGraphicsItem* graphicsItem, _effectIconHash)
    {
        if(graphicsItem)
            graphicsItem->setVisible(visible);
    }
}

void LayerTokens::cleanupPlayer()
{
    qDeleteAll(_combatantTokens);
    _combatantTokens.clear();
    _combatantTokenHash.clear();

    qDeleteAll(_effectTokens);
    _effectTokens.clear();
    _effectTokenHash.clear();

    _glScene = nullptr;
}

qreal LayerTokens::getIconOrder(int iconType, qreal order)
{
    return (iconType == DMHelper::CampaignType_BattleContentEffect) ? order - 0.5 : order;
}
