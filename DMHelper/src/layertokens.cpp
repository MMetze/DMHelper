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
#include <QDebug>

LayerTokens::LayerTokens(BattleDialogModel* model, const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    _glScene(nullptr),
    _playerInitialized(false),
    _model(model),
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
            int combatantType = combatantElement.attribute("type",QString::number(DMHelper::CombatantType_Base)).toInt();
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
                int monsterType = combatantElement.attribute("monsterType",QString::number(BattleDialogModelMonsterBase::BattleMonsterType_Base)).toInt();
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
                //_model->appendCombatantToList(combatant);
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
}

QRectF LayerTokens::boundingRect() const
{
    return QRectF();
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

    return newLayer;
}

void LayerTokens::applyOrder(int order)
{
    foreach(QGraphicsPixmapItem* pixmapItem, _combatantIconHash)
    {
        if(pixmapItem)
            pixmapItem->setZValue(order);
    }

    foreach(QGraphicsItem* graphicsItem, _effectIconHash)
    {
        if(graphicsItem)
            graphicsItem->setZValue(order);
    }
}

void LayerTokens::applyLayerVisible(bool layerVisible)
{
    foreach(QGraphicsPixmapItem* pixmapItem, _combatantIconHash)
    {
        if(pixmapItem)
            pixmapItem->setVisible(layerVisible);
    }

    foreach(QGraphicsItem* graphicsItem, _effectIconHash)
    {
        if(graphicsItem)
            graphicsItem->setVisible(layerVisible);
    }
}

void LayerTokens::applyOpacity(qreal opacity)
{
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

}

void LayerTokens::applySize(const QSize& size)
{

}

const QList<BattleDialogModelCombatant*> LayerTokens::getCombatants() const
{
    return _combatants;
}

QList<BattleDialogModelCombatant*> LayerTokens::getCombatants()
{
    return _combatants;
}

void LayerTokens::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    for(int i = 0; i < _combatants.count(); ++i)
    {
        createCombatantIcon(scene, _combatants.at(i));
    }

    QList<BattleDialogModelEffect*> effects = _model->getEffectList();
    for(BattleDialogModelEffect* effect : qAsConst(effects))
    {
        createEffectIcon(scene, effect);
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

void LayerTokens::playerGLInitialize(PublishGLScene* scene)
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

    Layer::playerGLInitialize(scene);
}

void LayerTokens::playerGLUninitialize()
{
    _playerInitialized = false;
    cleanupPlayer();
}

void LayerTokens::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    if(!_model)
        return;

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

            functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, combatantToken->getMatrixData());
            combatantToken->paintGL();
            combatantToken->paintEffects(defaultModelMatrix);
        }
    }

    if(_model->getShowEffects())
    {
        functions->glUseProgram(_shaderProgramRGBA);
        functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
        functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

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

                functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, effectToken->getMatrixData());
                functions->glUniform1f(_shaderAlphaRGBA, effectToken->getEffectAlpha());
                effectToken->paintGL();
            }
        }

        functions->glUseProgram(_shaderProgramRGB);
    }
}

void LayerTokens::playerGLResize(int w, int h)
{
}

bool LayerTokens::playerIsInitialized()
{
    return _playerInitialized;
}

void LayerTokens::initialize(const QSize& layerSize)
{
}

void LayerTokens::uninitialize()
{
}

void LayerTokens::setScale(int scale)
{
    if(_scale != scale)
    {
        // TODO: if the scale is changed, update the token scales
        _scale = scale;
    }
}

void LayerTokens::setModel(BattleDialogModel* model)
{
    if(!_model)
        _model = model;
}

void LayerTokens::addCombatant(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    if(_combatants.contains(combatant))
        return;

    _combatants.append(combatant);
    _model->appendCombatantToList(combatant);

    if((getLayerScene()) && (getLayerScene()->getDMScene()))
        createCombatantIcon(getLayerScene()->getDMScene(), combatant);
}

void LayerTokens::removeCombatant(BattleDialogModelCombatant* combatant)
{
    if((!combatant) || (!_model))
        return;

    if(!_combatants.removeOne(combatant))
        return;

    _model->removeCombatantFromList(combatant);

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

void LayerTokens::addEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    if(_effects.contains(effect))
        return;

    _effects.append(effect);
    connect(effect, &BattleDialogModelEffect::effectChanged, this, &LayerTokens::effectChanged);
    _model->appendEffectToList(effect);

    if((getLayerScene()) && (getLayerScene()->getDMScene()))
        createEffectIcon(getLayerScene()->getDMScene(), effect);
}

void LayerTokens::removeEffect(BattleDialogModelEffect* effect)
{
    if((!effect) || (!_model))
        return;

    if(!_effects.removeOne(effect))
        return;

    _model->removeEffectFromList(effect);

    QGraphicsItem* effectIcon = _effectIconHash.take(effect);
    if(effectIcon)
    {
        QGraphicsScene* scene = effectIcon->scene();
        if(scene)
            scene->removeItem(effectIcon);

        delete effectIcon;
    }

    PublishGLBattleEffect* effectToken = _effectTokenHash.take(effect);
    delete effectToken;
}

bool LayerTokens::containsEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return false;

    return _effects.contains(effect);
}

void LayerTokens::effectChanged(BattleDialogModelEffect* effect)
{
    if((!effect) || (!_model))
        return;

    // Changes to the player item will be directly handled through the signal
    QGraphicsItem* graphicsItem = _effectIconHash.value(effect);
    if(graphicsItem)
        effect->applyEffectValues(*graphicsItem, _model->getGridScale());
}

void LayerTokens::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    QDomElement combatantsElement = doc.createElement("combatants");
    for(BattleDialogModelCombatant* combatant : _combatants)
    {
        if(combatant)
            combatant->outputXML(doc, combatantsElement, targetDirectory, isExport);
    }
    element.appendChild(combatantsElement);

    QDomElement effectsElement = doc.createElement("effects");
    for(BattleDialogModelEffect* effect : _effects)
    {
        if(effect)
            effect->outputXML(doc, effectsElement, targetDirectory, isExport);
    }
    element.appendChild(effectsElement);

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerTokens::cleanupDM()
{
    if(!_combatantIconHash.isEmpty())
    {
        foreach(QGraphicsPixmapItem* pixmapItem, _combatantIconHash)
        {
            QGraphicsScene* scene = pixmapItem->scene();
            if(scene)
                scene->removeItem(pixmapItem);

            delete pixmapItem;
        }

        _combatantIconHash.clear();
    }

    if(!_effectIconHash.isEmpty())
    {
        foreach(QGraphicsItem* graphicsItem, _effectIconHash)
        {
            QGraphicsScene* scene = graphicsItem->scene();
            if(scene)
                scene->removeItem(graphicsItem);

            delete graphicsItem;
        }

        _effectIconHash.clear();
    }
}

void LayerTokens::createCombatantIcon(QGraphicsScene* scene, BattleDialogModelCombatant* combatant)
{
    if((!combatant) || (_combatantIconHash.contains(combatant)))
        return;

    if(combatant)
    {
        QPixmap pix = combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
        if(combatant->hasCondition(Combatant::Condition_Unconscious))
        {
            QImage originalImage = pix.toImage();
            QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
            pix = QPixmap::fromImage(grayscaleImage);
        }
        Combatant::drawConditions(&pix, combatant->getConditions());

        QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(pix, combatant);
        scene->addItem(pixmapItem);
        pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
        pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        pixmapItem->setPos(combatant->getPosition());
        pixmapItem->setOffset(-static_cast<qreal>(pix.width())/2.0, -static_cast<qreal>(pix.height())/2.0);
        qreal sizeFactor = combatant->getSizeFactor();
        qreal scaleFactor = (static_cast<qreal>(_scale-2)) * sizeFactor / static_cast<qreal>(qMax(pix.width(),pix.height()));
        pixmapItem->setScale(scaleFactor);

        QString itemTooltip = QString("<b>") + combatant->getName() + QString("</b>");
        QStringList conditionString = Combatant::getConditionString(combatant->getConditions());
        if(conditionString.count() > 0)
            itemTooltip += QString("<p>") + conditionString.join(QString("<br/>"));
        pixmapItem->setToolTip(itemTooltip);

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

        // TODO: Layers
        //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(handleCombatantMoved(BattleDialogModelCombatant*)), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
        //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(updateHighlights()), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
        //connect(combatant, SIGNAL(combatantSelected(BattleDialogModelCombatant*)), this, SLOT(handleCombatantSelected(BattleDialogModelCombatant*)));
    }
}

void LayerTokens::createEffectIcon(QGraphicsScene* scene, BattleDialogModelEffect* effect)
{
    if((!effect) || (!scene))
        return;

    if((effect->children().count() != 1) || (addSpellEffect(scene, effect) == nullptr))
        addEffectShape(scene, effect);
}

QGraphicsItem* LayerTokens::addEffectShape(QGraphicsScene* scene, BattleDialogModelEffect* effect)
{
    if((!_model) || (!effect) || (_effectIconHash.contains(effect)))
        return nullptr;

    QGraphicsItem* shape = effect->createEffectShape(_model->getGridScale());
    if(shape)
    {
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
