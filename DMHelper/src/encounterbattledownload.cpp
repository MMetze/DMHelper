#include "encounterbattledownload.h"
#include "battledialogmodel.h"

EncounterBattleDownload::EncounterBattleDownload(const QString& cacheDirectory, QObject *parent) :
    EncounterBattle(QString(), parent),
    _cacheDirectory(cacheDirectory)
{

}

void EncounterBattleDownload::inputXML(const QDomElement &element, bool isImport)
{
    EncounterBattle::inputXML(element, false);
    postProcessXML(element, false);

    inputXMLBattle(element, false);
}

bool EncounterBattleDownload::isComplete()
{

}

void EncounterBattleDownload::fileReceived(const QString& md5String, const QByteArray& data)
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
        return;
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
            BattleDialogModelCombatant* combatant = nullptr;
            int combatantIntId = DMH_GLOBAL_INVALID_ID;
            QUuid combatantId;
            int combatantType = combatantElement.attribute("type",QString::number(DMHelper::CombatantType_Base)).toInt();
            if(combatantType == DMHelper::CombatantType_Character)
            {
                combatantId = parseIdString(combatantElement.attribute("combatantId"), &combatantIntId);
                Character* character = campaign->getCharacterById(combatantId);
                if(!character)
                    character = campaign->getNPCById(combatantId);

                if(character)
                    combatant = new BattleDialogModelCharacter(character);
                else
                    qDebug() << "[Battle Dialog Manager] Unknown character ID type found: " << combatantId;
            }
            else if(combatantType == DMHelper::CombatantType_Monster)
            {
                int monsterType = combatantElement.attribute("monsterType",QString::number(BattleDialogModelMonsterBase::BattleMonsterType_Base)).toInt();
                if(monsterType == BattleDialogModelMonsterBase::BattleMonsterType_Combatant)
                {
                    combatantId = parseIdString(combatantElement.attribute("combatantId"), &combatantIntId, true);
                    Monster* monster = dynamic_cast<Monster*>(getCombatantById(combatantId, combatantIntId));
                    if(monster)
                        combatant = new BattleDialogModelMonsterCombatant(monster);
                    else
                        qDebug() << "[Battle Dialog Manager] Unknown monster ID type found: " << combatantId << " for battle";// " << battleId;
                }
                else if(monsterType == BattleDialogModelMonsterBase::BattleMonsterType_Class)
                {
                    QString monsterClassName = combatantElement.attribute("monsterClass");
                    MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(monsterClassName);
                    if(monsterClass)
                        combatant = new BattleDialogModelMonsterClass(monsterClass);
                    else
                        qDebug() << "[Battle Dialog Manager] Unknown monster class type found: " << monsterClassName;
                }
                else
                {
                    qDebug() << "[Battle Dialog Manager] Invalid monster type found: " << monsterType;
                }
            }
            else
            {
                qDebug() << "[Battle Dialog Manager] Invalid combatant type found: " << combatantType;
            }

            if(combatant)
            {
                combatant->inputXML(combatantElement, isImport);
                _battleModel->appendCombatant(combatant);
                if( ((!activeId.isNull()) && (combatant->getID() == activeId)) ||
                    (( activeId.isNull()) && (combatant->getIntID() == activeIdInt)) )
                {
                    _battleModel->setActiveCombatant(combatant);
                }
            }

            combatantElement = combatantElement.nextSiblingElement();
        }
    }

    inputXMLEffects(rootBattleElement.firstChildElement("effects"), isImport);
}

void EncounterBattleDownload::inputXMLEffects(const QDomElement &parentElement, bool isImport)
{

}
