#include "encounterbattledownload.h"
#include "battledialogmodel.h"
#include "battledialogmodeleffectfactory.h"
#include "battledialogmodelcombatantdownload.h"
#include "battledialogmodeleffectobjectdownload.h"
#include <QDebug>

EncounterBattleDownload::EncounterBattleDownload(const QString& cacheDirectory, QObject *parent) :
    EncounterBattle(QString(), parent),
    _cacheDirectory(cacheDirectory)
{

}

void EncounterBattleDownload::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    EncounterBattle::inputXML(element, true);
    postProcessXML(element, false);
    inputXMLBattle(element, false);
}

void EncounterBattleDownload::updateXML(const QDomElement &element)
{
    if(!_battleModel)
        return;

    QDomElement rootBattleElement = element.firstChildElement("battle");
    if(rootBattleElement.isNull())
        return;

    int activeIdInt = DMH_GLOBAL_INVALID_ID;
    QUuid activeId = parseIdString(rootBattleElement.attribute("activeId"), &activeIdInt, true);
    QUuid selectedId = parseIdString(rootBattleElement.attribute("selectedId"), &activeIdInt, true);

    QDomElement combatantsElement = rootBattleElement.firstChildElement("combatants");
    if(!combatantsElement.isNull())
    {
        QDomElement combatantElement = combatantsElement.firstChildElement("battlecombatant");
        while(!combatantElement.isNull())
        {
            if(combatantElement.hasAttribute("_baseID"))
            {
                QUuid combatantId(combatantElement.attribute("_baseID"));
                BattleDialogModelCombatantDownload* combatant = findCombatantById(combatantId);
                if(combatant)
                {
                    combatant->updateXML(combatantElement);
                    if(activeId == combatantId)
                        _battleModel->setActiveCombatant(combatant);
                    if(selectedId == combatantId)
                        _battleModel->setSelectedCombatant(combatant);
                }
            }

            combatantElement = combatantElement.nextSiblingElement("battlecombatant");
        }
    }

    QDomElement effectsElement = rootBattleElement.firstChildElement("effects");
    if(!effectsElement.isNull())
    {
        QDomElement effectElement = effectsElement.firstChildElement("battleeffect");
        while(!effectElement.isNull())
        {
            if(effectElement.hasAttribute("_baseID"))
            {
                QUuid effectId(effectElement.attribute("_baseID"));
                BattleDialogModelEffect* effect = findEffectById(effectId);
                if(effect)
                    effect->inputXML(effectElement, false);

                QDomElement effectChildElement = effectElement.firstChildElement("battleeffect");
                if(!effectChildElement.isNull())
                {
                    QUuid effectChildId(effectChildElement.attribute("_baseID"));
                    BattleDialogModelEffect* effectChild = findEffectById(effectChildId);
                    if(effectChild)
                        effectChild->inputXML(effectChildElement, false);
                }
            }

            effectElement = effectElement.nextSiblingElement("battleeffect");
        }
    }
}

bool EncounterBattleDownload::isComplete()
{
    for(int i = 0; i < _battleModel->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatantDownload* combatant = dynamic_cast<BattleDialogModelCombatantDownload*>(_battleModel->getCombatant(i));
        if((combatant) && (!combatant->isTokenValid()))
            return false;
    }

    for(int i = 0; i < _battleModel->getEffectCount(); ++i)
    {
        BattleDialogModelEffect* baseEffect = _battleModel->getEffect(i);
        if(baseEffect)
        {
            BattleDialogModelEffectObjectDownload* effect = dynamic_cast<BattleDialogModelEffectObjectDownload*>(baseEffect);
            if((effect) && (effect->getImageFile().isEmpty()))
                return false;

            QList<BattleDialogModelEffectObjectDownload*> children = baseEffect->findChildren<BattleDialogModelEffectObjectDownload*>();
            for(BattleDialogModelEffectObjectDownload* childEffect : children)
            {
                if((childEffect) && (childEffect->getImageFile().isEmpty()))
                    return false;
            }
        }
    }

    return true;
}

void EncounterBattleDownload::fileReceived(const QString& md5, const QString& uuid, const QByteArray& data)
{
    if((md5.isEmpty()) || (data.isEmpty()))
    {
        qDebug() << "[EncounterBattleDownload] ERROR: Invalid file received! MD5: " << md5 << ", UUID: " << uuid << ", data size: " << data.size();
        return;
    }

    if(!_battleModel)
    {
        qDebug() << "[EncounterBattleDownload] ERROR: Unexpected file received without a valid model! MD5: " << md5 << ", UUID: " << uuid;
        return;
    }

    for(int i = 0; i < _battleModel->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatantDownload* combatant = dynamic_cast<BattleDialogModelCombatantDownload*>(_battleModel->getCombatant(i));
        if((combatant) && (combatant->getMD5() == md5))
        {
            combatant->fileReceived(md5, uuid, data);
            checkComplete();
            return;
        }
    }

    for(int i = 0; i < _battleModel->getEffectCount(); ++i)
    {
        BattleDialogModelEffect* baseEffect = _battleModel->getEffect(i);
        if(baseEffect)
        {
            BattleDialogModelEffectObjectDownload* effect = dynamic_cast<BattleDialogModelEffectObjectDownload*>(_battleModel->getEffect(i));
            if((effect) && (effect->getMD5() == md5))
            {
                effect->setFileName(_cacheDirectory + QString("/") + _md5);
                checkComplete();
                return;
            }

            QList<BattleDialogModelEffectObjectDownload*> children = baseEffect->findChildren<BattleDialogModelEffectObjectDownload*>();
            for(BattleDialogModelEffectObjectDownload* childEffect : children)
            {
                if((childEffect) && (childEffect->getMD5() == md5))
                {
                    childEffect->setImageFile(_cacheDirectory + QString("/") + md5);
                    checkComplete();
                    return;
                }
            }
        }
    }
}

void EncounterBattleDownload::inputXMLBattle(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    if(_battleModel)
    {
        qDebug() << "[EncounterBattleDownload] ERROR: Unexpected attempt to input an XML battle without a valid model!";
        return;
    }

    /*
    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return;
        */

    QDomElement rootBattleElement = element.firstChildElement("battle");
    if(rootBattleElement.isNull())
    {
        _battleModel = createNewBattle(QPointF(0.0, 0.0));
        return;
    }

    _battleModel = new BattleDialogModel();
    _battleModel->inputXML(rootBattleElement, isImport);

    /*
    int mapIdInt = DMH_GLOBAL_INVALID_ID;
    QUuid mapId = parseIdString(rootBattleElement.attribute("mapID"), &mapIdInt);
    Map* battleMap = dynamic_cast<Map*>(campaign->getObjectById(mapId));
    if(battleMap)
    {
        QRect mapRect(rootBattleElement.attribute("mapRectX",QString::number(0)).toInt(),
                      rootBattleElement.attribute("mapRectY",QString::number(0)).toInt(),
                      rootBattleElement.attribute("mapRectWidth",QString::number(0)).toInt(),
                      rootBattleElement.attribute("mapRectHeight",QString::number(0)).toInt());

        _battleModel->setMap(battleMap, mapRect);
    }
    */

    int activeIdInt = DMH_GLOBAL_INVALID_ID;
    QUuid activeId = parseIdString(rootBattleElement.attribute("activeId"), &activeIdInt, true);

    QDomElement combatantsElement = rootBattleElement.firstChildElement("combatants");
    if(!combatantsElement.isNull())
    {
        QDomElement combatantElement = combatantsElement.firstChildElement("battlecombatant");
        while(!combatantElement.isNull())
        {
            BattleDialogModelCombatantDownload* combatant = new BattleDialogModelCombatantDownload(this);
            connect(combatant, &BattleDialogModelCombatantDownload::requestFile, this, &EncounterBattleDownload::requestFile);
            combatant->inputXML(combatantElement, false);
            _battleModel->appendCombatant(combatant);
            //if(((!activeId.isNull()) && (combatant->getID() == activeId)) ||
            //   (( activeId.isNull()) && (combatant->getIntID() == activeIdInt)) )
            if((!activeId.isNull()) && (combatant->getID() == activeId))
                _battleModel->setActiveCombatant(combatant);

            combatantElement = combatantElement.nextSiblingElement("battlecombatant");
        }
    }

    inputXMLEffects(rootBattleElement.firstChildElement("effects"), isImport);
}

void EncounterBattleDownload::inputXMLEffects(const QDomElement &parentElement, bool isImport)
{
    if(parentElement.isNull())
        return;

    QDomElement effectElement = parentElement.firstChildElement("battleeffect");
    while(!effectElement.isNull())
    {
        BattleDialogModelEffect* newEffect = createEffect(effectElement, isImport);
        if(newEffect)
        {
            QDomElement effectChildElement = effectElement.firstChildElement("battleeffect");
            if(!effectChildElement.isNull())
            {
                BattleDialogModelEffect* childEffect = createEffect(effectChildElement, isImport);
                if(childEffect)
                    newEffect->addObject(childEffect);
            }
            _battleModel->appendEffect(newEffect);
        }

        effectElement = effectElement.nextSiblingElement("battleeffect");
    }
}

BattleDialogModelEffect* EncounterBattleDownload::createEffect(const QDomElement& element, bool isImport)
{
    int effectType = element.attribute("type", QString::number(BattleDialogModelEffect::BattleDialogModelEffect_Base)).toInt();

    if(effectType == BattleDialogModelEffect::BattleDialogModelEffect_Object)
    {
        BattleDialogModelEffectObjectDownload* result = new BattleDialogModelEffectObjectDownload();
        connect(result, &BattleDialogModelEffectObjectDownload::requestFile, this, &EncounterBattleDownload::requestFile);
        result->inputXML(element, isImport);
        return result;
    }
    else
    {
        return BattleDialogModelEffectFactory::createEffect(element, isImport);
    }
}

void EncounterBattleDownload::checkComplete()
{
    if(isComplete())
        emit encounterComplete();
}

BattleDialogModelCombatantDownload* EncounterBattleDownload::findCombatantById(QUuid combatantId)
{
    if(_battleModel)
    {
        QList<BattleDialogModelCombatant*> combatants = _battleModel->getCombatantList();
        for(BattleDialogModelCombatant* combatant : combatants)
        {
            if((combatant) && (combatant->getID() == combatantId))
                return dynamic_cast<BattleDialogModelCombatantDownload*>(combatant);
        }
    }

    return nullptr;
}

BattleDialogModelEffect* EncounterBattleDownload::findEffectById(QUuid effectId)
{
    if(_battleModel)
    {
        QList<BattleDialogModelEffect*> effects = _battleModel->getEffectList();
        for(BattleDialogModelEffect* effect : effects)
        {
            if(effect)
            {
                if(effect->getID() == effectId)
                    return effect;

                CampaignObjectBase* childObject = effect->searchChildrenById(effectId);
                if(childObject)
                {
                    BattleDialogModelEffect* childEffect = dynamic_cast<BattleDialogModelEffect*>(childObject);
                    if(childEffect)
                        return childEffect;
                }
            }
        }
    }

    return nullptr;
}