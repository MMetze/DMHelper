#include "encounterbattle.h"
#include "dmconstants.h"
#include "encounterbattleedit.h"
#include "battleframe.h"
#include "combatant.h"
#include "combatantfactory.h"
#include "campaign.h"
#include "bestiary.h"
#include "map.h"
#include "monster.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelmonsterclass.h"
#include "battledialogmodelmonstercombatant.h"
#include "battledialogmodeleffectfactory.h"
#include "audiotrack.h"
#include "encounterfactory.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDomCDATASection>
#include <QTextDocument>
#include <QDebug>

EncounterBattle::EncounterBattle(const QString& encounterName, QObject *parent) :
    EncounterText(encounterName, parent),
    _audioTrackId(),
    _combatantWaves(),
    _battleModel(nullptr)
{
}

EncounterBattle::~EncounterBattle()
{
    while(_combatantWaves.count() > 0)
    {
        CombatantGroupList wave = _combatantWaves.takeFirst();
        while(wave.count() > 0)
        {
            CombatantGroup combatantPair = wave.takeAt(0);
            delete combatantPair.second;
        }
    }
}

void EncounterBattle::inputXML(const QDomElement &element, bool isImport)
{
    QDomElement rootCombatantsElement = element.firstChildElement( "combatants" );
    if(rootCombatantsElement.isNull())
    {
        QDomElement wavesElement = element.firstChildElement( "waves" );
        if( !wavesElement.isNull() )
        {
            int wave = 0;

            QDomElement waveElement = wavesElement.firstChildElement( QString("wave") );
            while( !waveElement.isNull() )
            {
                insertWave(wave);

                QDomElement combatantsElement = waveElement.firstChildElement( "combatants" );
                if( !combatantsElement.isNull() )
                {
                    QDomElement combatantPairElement = combatantsElement.firstChildElement( QString("combatantPair") );
                    while( !combatantPairElement.isNull() )
                    {
                        int combatantCount = combatantPairElement.attribute("count").toInt();
                        QDomElement combatantElement = combatantPairElement.firstChildElement( QString("combatant") );
                        if( !combatantElement.isNull() )
                        {
                            bool ok = false;
                            int combatantType = combatantElement.attribute("type").toInt(&ok);
                            if(ok)
                            {
                                Combatant* newCombatant = CombatantFactory::createCombatant(combatantType, combatantElement, isImport, nullptr);
                                if(newCombatant)
                                {
                                    addCombatant(wave, combatantCount, newCombatant);
                                }
                            }
                        }
                        combatantPairElement = combatantPairElement.nextSiblingElement( QString("combatantPair") );
                    }
                }

                waveElement = waveElement.nextSiblingElement( QString("wave") );
                ++wave;
            }
        }
    }

    extractTextNode(element, isImport);
    if(!getText().isEmpty())
    {
        QString battleName = element.attribute(QString("name"), QString("Battle Text"));
        CampaignObjectBase* encounter = EncounterFactory().createObject(DMHelper::CampaignType_Text, -1, battleName, isImport);
        if(encounter)
        {
            EncounterText* textEncounter = dynamic_cast<EncounterText*>(encounter);
            if(textEncounter)
            {
                textEncounter->setText(getText());
                addObject(textEncounter);
                setText(QString());
            }
            else
            {
                delete encounter;
            }
        }
    }

    CampaignObjectBase::inputXML(element, isImport);

}

void EncounterBattle::copyValues(const CampaignObjectBase* other)
{
    const EncounterBattle* otherBattle = dynamic_cast<const EncounterBattle*>(other);
    if(!otherBattle)
        return;

    _audioTrackId = otherBattle->_audioTrackId;

    EncounterText::copyValues(other);
}

int EncounterBattle::getObjectType() const
{
    return DMHelper::CampaignType_Battle;
}

bool EncounterBattle::hasData() const
{
    return _battleModel != nullptr;
}

AudioTrack* EncounterBattle::getAudioTrack()
{
    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return nullptr;

    return campaign->getTrackById(_audioTrackId);
}

QUuid EncounterBattle::getAudioTrackId()
{
    return _audioTrackId;
}

void EncounterBattle::setAudioTrack(AudioTrack* track)
{
    QUuid newTrackId = (track == nullptr) ? QUuid() : track->getID();
    if(_audioTrackId != newTrackId)
    {
        _audioTrackId = newTrackId;
        emit dirty();
    }
}

int EncounterBattle::getWaveCount() const
{
    return _combatantWaves.count();
}

void EncounterBattle::insertWave(int wave)
{
    if(wave < 0)
    {
        _combatantWaves.prepend(CombatantGroupList());
    }
    else if(wave >= _combatantWaves.count())
    {
        _combatantWaves.append(CombatantGroupList());
    }
    else
    {
        _combatantWaves.insert(wave, CombatantGroupList());
    }
    emit dirty();
}

void EncounterBattle::removeWave(int wave)
{
    if( (_combatantWaves.count() <= 0 ) ||
        (wave < 0) ||
        (wave >= _combatantWaves.count()) )
        return;

    CombatantGroupList waveCombatants = _combatantWaves.takeAt(wave);
    while(waveCombatants.count() > 0)
    {
        CombatantGroup combatantPair = waveCombatants.takeAt(0);
        delete combatantPair.second;
    }
}

CombatantGroupList EncounterBattle::getCombatants(int wave) const
{
    if((wave < 0)||(wave >= _combatantWaves.count()))
        return CombatantGroupList();
    else
        return _combatantWaves.at(wave);
}

int EncounterBattle::getCombatantCount(int wave) const
{
    if((wave < 0)||(wave >= _combatantWaves.count()))
        return -1;
    else
        return _combatantWaves.at(wave).count();
}

CombatantGroupList EncounterBattle::getCombatantsAllWaves() const
{
    CombatantGroupList results;

    for(int i = 0; i < _combatantWaves.count(); ++i)
    {
        results.append(_combatantWaves.at(i));
    }

    return results;
}

int EncounterBattle::getCombatantCountAllWaves() const
{
    int result = 0;

    for(int i = 0; i < _combatantWaves.count(); ++i)
    {
        result += _combatantWaves.at(i).count();
    }

    return result;
}

void EncounterBattle::addCombatant(int wave, int count, Combatant* combatant)
{
    if((combatant == nullptr) || (wave < 0) || (wave >= _combatantWaves.count()))
        return;

    _combatantWaves[wave].append(CombatantGroup(count, combatant));
    emit dirty();
}

void EncounterBattle::editCombatant(int wave, int index, int count, Combatant* combatant)
{
    if((combatant == nullptr) || (wave < 0) || (wave >= _combatantWaves.count()) || ((index < 0) || index >= _combatantWaves.at(wave).count()))
        return;

    _combatantWaves[wave].replace(index, CombatantGroup(count, combatant));
    emit dirty();
}

void EncounterBattle::removeCombatant(int wave, int index)
{
    if((wave < 0) || (wave >= _combatantWaves.count()) || ((index < 0) || index >= _combatantWaves.at(wave).count()))
        return;

    CombatantGroup combatantPair = _combatantWaves[wave].takeAt(index);
    delete combatantPair.second;
    emit dirty();
}

Combatant* EncounterBattle::getCombatantById(QUuid combatantId, int combatantIntId) const
{
    if(combatantId.isNull())
    {
        for(CombatantGroupList combatantGroupList : _combatantWaves)
        {
            for(CombatantGroup combatantGroup : combatantGroupList)
            {
                if((combatantGroup.second) && (combatantGroup.second->getIntID() == combatantIntId))
                    return combatantGroup.second;
            }
        }
    }
    else
    {
        for(CombatantGroupList combatantGroupList : _combatantWaves)
        {
            for(CombatantGroup combatantGroup : combatantGroupList)
            {
                if((combatantGroup.second) && (combatantGroup.second->getID() == combatantId))
                    return combatantGroup.second;
            }
        }
    }

    return nullptr;
}

void EncounterBattle::createBattleDialogModel()
{
    qDebug() << "[EncounterBattle] Creating new battle model for encounter " << getID();
    setBattleDialogModel(createNewBattle(QPointF(0.0, 0.0)));
}

void EncounterBattle::setBattleDialogModel(BattleDialogModel* model)
{
    qDebug() << "[EncounterBattle] Setting new battle model to " << model->getID() << " for encounter " << getID();

    if(_battleModel)
    {
        qDebug() << "[EncounterBattle] Setting new battle model, found old battle model.";
        removeBattleDialogModel();
    }

    _battleModel = model;

    emit changed();
    emit dirty();
}

BattleDialogModel* EncounterBattle::getBattleDialogModel() const
{
    return _battleModel;
}

void EncounterBattle::removeBattleDialogModel()
{
    if(!_battleModel)
        return;

    qDebug() << "[EncounterBattle] Removing battle model " << _battleModel->getID() << " from encounter " << getID();
    BattleDialogModel* battleModel = _battleModel;
    _battleModel = nullptr;
    delete battleModel;

    emit changed();
    emit dirty();
}

void EncounterBattle::inputXMLBattle(const QDomElement &element, bool isImport)
{
    if((_battleModel) || (isImport))
        return;

    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return;

    QDomElement rootBattleElement = element.firstChildElement("battle");
    if(rootBattleElement.isNull())
    {
        _battleModel = createNewBattle(QPointF(0.0, 0.0));
        return;
    }

    _battleModel = new BattleDialogModel(this);
    _battleModel->inputXML(rootBattleElement, isImport);

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

void EncounterBattle::inputXMLEffects(const QDomElement &parentElement, bool isImport)
{
    if((!_battleModel)||(isImport))
        return;

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

QDomElement EncounterBattle::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("battle-object");
}

void EncounterBattle::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("audiotrack", _audioTrackId.toString());

    QDomElement wavesElement = doc.createElement("waves");
    element.appendChild(wavesElement);
    for(int wave = 0; wave < _combatantWaves.count(); ++wave)
    {
        QDomElement waveElement = doc.createElement(QString("wave"));
        wavesElement.appendChild(waveElement);

        QDomElement combatantsElement = doc.createElement("combatants");
        waveElement.appendChild(combatantsElement);
        for(int i = 0; i < _combatantWaves.at(wave).count(); ++i)
        {
            CombatantGroup combatantPair = _combatantWaves.at(wave).at(i);
            QDomElement combatantPairElement = doc.createElement(QString("combatantPair"));

            combatantPairElement.setAttribute("count", combatantPair.first);
            combatantPair.second->outputXML(doc, combatantPairElement, targetDirectory, isExport);

            combatantsElement.appendChild(combatantPairElement);
        }
    }

    //if(_battleModel && !isExport)
    if(_battleModel)
        _battleModel->outputXML(doc, element, targetDirectory, isExport);

    EncounterText::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool EncounterBattle::belongsToObject(QDomElement& element)
{
    if((element.tagName() == QString("combatants")) || (element.tagName() == QString("waves")) || (element.tagName() == QString("battle")))
        return true;
    else
        return EncounterText::belongsToObject(element);
}

void EncounterBattle::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    _audioTrackId = parseIdString(element.attribute("audiotrack"));

    // Read the battle
    if(!isImport)
        inputXMLBattle(element, isImport);

    EncounterText::internalPostProcessXML(element, isImport);
}

BattleDialogModel* EncounterBattle::createNewBattle(QPointF combatantPos)
{
    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return nullptr;

    BattleDialogModel* battleModel = new BattleDialogModel(this);

    // Add the active characters
    QList<Character*> activeCharacters = campaign->getActiveCharacters();
    for(int i = 0; i < activeCharacters.count(); ++i)
    {
        BattleDialogModelCharacter* newCharacter = new BattleDialogModelCharacter(activeCharacters.at(i));
        newCharacter->setPosition(combatantPos);
        battleModel->appendCombatant(newCharacter);
    }

    connect(battleModel,SIGNAL(destroyed(QObject*)),this,SLOT(completeBattle()));

    return battleModel;
}
