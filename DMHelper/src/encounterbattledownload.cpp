#include "encounterbattledownload.h"
#include "battledialogmodel.h"
#include "battledialogmodeleffectfactory.h"
#include "battledialogmodelcombatantdownload.h"

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

bool EncounterBattleDownload::isComplete()
{
    return true;
}

void EncounterBattleDownload::fileReceived(const QString& md5, const QString& uuid, const QByteArray& data)
{
    if(isComplete())
        emit encounterComplete();
}

void EncounterBattleDownload::inputXMLBattle(const QDomElement &element, bool isImport)
{
    if((_battleModel)||(isImport))
        return;

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
        QDomElement combatantElement = combatantsElement.firstChildElement();
        while(!combatantElement.isNull())
        {
            BattleDialogModelCombatantDownload* combatant = new BattleDialogModelCombatantDownload(this);
            combatant->inputXML(combatantElement, false);
            _battleModel->appendCombatant(combatant);
            if( ((!activeId.isNull()) && (combatant->getID() == activeId)) ||
                (( activeId.isNull()) && (combatant->getIntID() == activeIdInt)) )
            {
                _battleModel->setActiveCombatant(combatant);
            }

            combatantElement = combatantElement.nextSiblingElement();
        }
    }

    inputXMLEffects(rootBattleElement.firstChildElement("effects"), isImport);
}

void EncounterBattleDownload::inputXMLEffects(const QDomElement &parentElement, bool isImport)
{
    if(parentElement.isNull())
        return;

    QDomElement effectElement = parentElement.firstChildElement();
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
            _battleModel->appendEffect(newEffect);
        }

        effectElement = effectElement.nextSiblingElement();
    }
}
