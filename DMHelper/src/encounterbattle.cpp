#include "encounterbattle.h"
#include "dmconstants.h"
#include "encounterbattleedit.h"
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
#include <QDomDocument>
#include <QDomElement>
#include <QDomCDATASection>
#include <QTextDocument>
#include <QDebug>

EncounterBattle::EncounterBattle(const QString& encounterName, QObject *parent) :
    Encounter(encounterName, parent),
    _text(),
    _audioTrackId(-1),
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

void EncounterBattle::inputXML(const QDomElement &element)
{
    Encounter::inputXML(element);

    if( ( !element.firstChild().isNull() ) && ( element.firstChild().isCDATASection() ) )
    {
        QDomCDATASection cdata = element.firstChild().toCDATASection();
        setText(cdata.data());
    }

    _audioTrackId = element.attribute("audiotrack").toInt();

    // Backwards compatibility; TODO: can be removed eventually
    QDomElement rootCombatantsElement = element.firstChildElement( "combatants" );
    if(!rootCombatantsElement.isNull())
    {
        insertWave(0); // By default, put everything in a single wave

        QDomElement combatantPairElement = rootCombatantsElement.firstChildElement( QString("combatantPair") );
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
                    Combatant* newCombatant = CombatantFactory::createCombatant(combatantType, combatantElement);
                    if(newCombatant)
                    {
                        addCombatant(0, combatantCount, newCombatant);
                    }
                }
            }
            combatantPairElement = combatantPairElement.nextSiblingElement( QString("combatantPair") );
        }
    }
    else
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
                                Combatant* newCombatant = CombatantFactory::createCombatant(combatantType, combatantElement, this);
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
}

void EncounterBattle::postProcessXML(const QDomElement &element)
{
    // Read the battle
    inputXMLBattle(element);
}

void EncounterBattle::widgetActivated(QWidget* widget)
{
    EncounterBattleEdit* battleEdit = dynamic_cast<EncounterBattleEdit*>(widget);
    if(!battleEdit)
        return;

    if(battleEdit->getBattle() != nullptr)
    {
        qDebug() << "[EncounterBattle] ERROR: Battle not deactivated: " << battleEdit->getBattle()->getID() << " """ << battleEdit->getBattle()->getName();
        qDebug() << "[EncounterBattle] ERROR: Previous battle will now be deactivated. This should happen previously!";
        battleEdit->getBattle()->widgetDeactivated(widget);
    }

    qDebug() << "[EncounterBattle] Widget Activated " << getID() << " """ << _name;

    battleEdit->setBattle(this);

    _widget = widget;
}

void EncounterBattle::widgetDeactivated(QWidget* widget)
{
    EncounterBattleEdit* battleEdit = dynamic_cast<EncounterBattleEdit*>(widget);
    if(!battleEdit)
        return;

    qDebug() << "[EncounterBattle] Widget Deactivated " << getID() << " """ << _name;

    battleEdit->unsetBattle(this);

    _widget = nullptr;
}

int EncounterBattle::getType() const
{
    return DMHelper::EncounterType_Battle;
}

bool EncounterBattle::hasData() const
{
    return _battleModel != nullptr;
}

QString EncounterBattle::getText() const
{
    return _text;
}

AudioTrack* EncounterBattle::getAudioTrack()
{
    Campaign* campaign = getCampaign();
    if(!campaign)
        return nullptr;

    return campaign->getTrackById(_audioTrackId);
}

int EncounterBattle::getAudioTrackId()
{
    return _audioTrackId;
}

void EncounterBattle::setAudioTrack(AudioTrack* track)
{
    int newTrackId = (track == nullptr) ? -1 : track->getID();
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

Combatant* EncounterBattle::getCombatantById(int combatantId) const
{
    for(CombatantGroupList combatantGroupList : _combatantWaves)
    {
        for(CombatantGroup combatantGroup : combatantGroupList)
        {
            if((combatantGroup.second) && (combatantGroup.second->getID() == combatantId))
                return combatantGroup.second;
        }
    }

    return nullptr;
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
    _battleModel->setParent(this);

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

void EncounterBattle::setText(const QString& newText)
{
    QTextDocument doc;
    doc.setHtml(newText);
    // qDebug() << "[EncounterBattle] " << getID() << " """ << _name << """ text set to: " << doc.toPlainText();

    if(_text != newText)
    {
        _text = newText;
        emit dirty();
    }
}

void EncounterBattle::widgetChanged()
{
    if(!_widget)
        return;

    qDebug() << "[EncounterBattle] Widget Changed " << getID() << " """ << _name;

    EncounterBattleEdit* battleEdit = dynamic_cast<EncounterBattleEdit*>(_widget);
    if(!battleEdit)
        return;

    battleEdit->setBattle(this);
}

void EncounterBattle::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory)
{
    Q_UNUSED(targetDirectory);

    QDomCDATASection cdata = doc.createCDATASection(getText());
    element.appendChild(cdata);

    element.setAttribute( "audiotrack", _audioTrackId );

    QDomElement wavesElement = doc.createElement( "waves" );
    element.appendChild(wavesElement);
    for( int wave = 0; wave < _combatantWaves.count(); ++wave )
    {
        QDomElement waveElement = doc.createElement( QString("wave") );
        wavesElement.appendChild(waveElement);

        QDomElement combatantsElement = doc.createElement( "combatants" );
        waveElement.appendChild(combatantsElement);
        for( int i = 0; i < _combatantWaves.at(wave).count(); ++i )
        {
            CombatantGroup combatantPair = _combatantWaves.at(wave).at(i);
            QDomElement combatantPairElement = doc.createElement( QString("combatantPair") );

            combatantPairElement.setAttribute( "count", combatantPair.first );
            combatantPair.second->outputXML(doc, combatantPairElement, targetDirectory);

            combatantsElement.appendChild(combatantPairElement);
        }
    }

    if(_battleModel)
        _battleModel->outputXML(doc, element, targetDirectory);
}

void EncounterBattle::inputXMLBattle(const QDomElement &element)
{
    if(_battleModel)
        return;

    Campaign* campaign = getCampaign();
    if(!campaign)
        return;

    QDomElement rootBattleElement = element.firstChildElement( "battle" );
    if(rootBattleElement.isNull())
        return;

    /*
    int battleId = rootBattleElement.attribute("battleID",QString::number(DMH_GLOBAL_INVALID_ID)).toInt();
    if(battleId != getID())
        return;
    */

    _battleModel = new BattleDialogModel();
    _battleModel->inputXML(rootBattleElement);
    //_battleModel->setBattle(this);

    int mapId = rootBattleElement.attribute("mapID",QString::number(DMH_GLOBAL_INVALID_ID)).toInt();
    Map* battleMap = dynamic_cast<Map*>(campaign->getObjectbyId(mapId));
    if(battleMap)
    {
        QRect mapRect(rootBattleElement.attribute("mapRectX",QString::number(0)).toInt(),
                      rootBattleElement.attribute("mapRectY",QString::number(0)).toInt(),
                      rootBattleElement.attribute("mapRectWidth",QString::number(0)).toInt(),
                      rootBattleElement.attribute("mapRectHeight",QString::number(0)).toInt());

        _battleModel->setMap(battleMap, mapRect);
    }

    int activeId = rootBattleElement.attribute("activeId",QString::number(DMH_GLOBAL_INVALID_ID)).toInt();

    QDomElement combatantsElement = rootBattleElement.firstChildElement("combatants");
    if(!combatantsElement.isNull())
    {
        QDomElement combatantElement = combatantsElement.firstChildElement();
        while(!combatantElement.isNull())
        {
            BattleDialogModelCombatant* combatant = nullptr;
            int combatantId = combatantElement.attribute("combatantId",QString::number(DMH_GLOBAL_INVALID_ID)).toInt();
            int combatantType = combatantElement.attribute("type",QString::number(DMHelper::CombatantType_Base)).toInt();
            if(combatantType == DMHelper::CombatantType_Character)
            {
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
                    Monster* monster = dynamic_cast<Monster*>(getCombatantById(combatantId));
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
                combatant->inputXML(combatantElement);
                _battleModel->appendCombatant(combatant);
                if(combatant->getID() == activeId)
                    _battleModel->setActiveCombatant(combatant);
            }

            combatantElement = combatantElement.nextSiblingElement();
        }
    }

    QDomElement effectsElement = rootBattleElement.firstChildElement("effects");
    if(!effectsElement.isNull())
    {
        QDomElement effectElement = effectsElement.firstChildElement();
        while(!effectElement.isNull())
        {
            BattleDialogModelEffect* newEffect = BattleDialogModelEffectFactory::createEffect(effectElement);
            if(newEffect)
                _battleModel->appendEffect(newEffect);

            effectElement = effectElement.nextSiblingElement();
        }
    }
}
