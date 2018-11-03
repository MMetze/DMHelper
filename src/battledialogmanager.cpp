#include "battledialogmanager.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodelmonsterclass.h"
#include "battledialogmodelmonstercombatant.h"
#include "bestiary.h"
#include "campaign.h"
#include "map.h"
#include "encounterbattle.h"
#include "adventure.h"
#include "addmonstersdialog.h"
#include "addbattlewavedialog.h"
#include "battledialog.h"
#include "combatant.h"
#include "monster.h"
#include "monsterclass.h"
#include "character.h"
#include "itemselectdialog.h"
#include "mapselectdialog.h"
#include "selectzoom.h"
#include "networkcontroller.h"
#include <QMessageBox>
#include <QDebug>

BattleDialogManager::BattleDialogManager(QWidget *parent) :
    QObject(parent),
    _dlg(nullptr),
    _encounterBattle(nullptr),
    _campaign(nullptr),
    _networkManager(nullptr),
    _targetSize(),
    _showOnDeck(true),
    _showCountdown(true),
    _countdownDuration(15)
{
}

BattleDialogManager::~BattleDialogManager()
{
    delete _dlg;
}

QList<BattleDialogModelCombatant*> BattleDialogManager::getLivingMonsters() const
{
    if(_dlg)
    {
        return _dlg->getLivingMonsters();
    }
    else
    {
        return QList<BattleDialogModelCombatant*>();
    }
}

void BattleDialogManager::setNetworkManager(NetworkController* networkManager)
{
    _networkManager = networkManager;
}

void BattleDialogManager::setCampaign(Campaign* campaign)
{
    if(campaign == _campaign)
        return;

    if(_dlg)
        completeBattle();

    _campaign = campaign;
}

void BattleDialogManager::showBattleDialog()
{
    if(_dlg)
    {
        _dlg->show();
        _dlg->activateWindow();
        emit dirty();
    }
}

void BattleDialogManager::startNewBattle(EncounterBattle* battleEncounter)
{
    if(!_campaign)
        return;

    if(_dlg)
    {
        QMessageBox::StandardButton result = QMessageBox::critical(_dlg, QString("Confirm Close Battle"), QString("There is an existing battle still ongoing. Are you sure you wish to end that battle to start a new one? All changes will be discarded."), QMessageBox::Yes | QMessageBox::No);
        if(result == QMessageBox::No)
            return;

        completeBattle();
    }

    BattleDialogModel* battleModel = new BattleDialogModel();
    //battleModel->setBattle(battleEncounter);

    QPointF combatantPos;
    if(_dlg)
        combatantPos = _dlg->viewportCenter();

    // Add the active characters
    QList<Character*> activeCharacters = _campaign->getActiveCharacters();
    for(int i = 0; i < activeCharacters.count(); ++i)
    {
        BattleDialogModelCharacter* newCharacter = new BattleDialogModelCharacter(activeCharacters.at(i));
        newCharacter->setPosition(combatantPos);
        battleModel->appendCombatant(newCharacter);
    }

    if(battleEncounter)
    {
        connect(battleEncounter,SIGNAL(destroyed(QObject*)),this,SLOT(completeBattle()));

        // Add wave zero of monsters
        battleModel->appendCombatants( createWaveMonsters(battleEncounter, 0) );

        // Register the model with the encounter
        battleEncounter->setBattleDialogModel(battleModel);
    }

    _encounterBattle = battleEncounter;
    _dlg = createBattleDialog(battleModel);
    if(!_dlg)
        return;

    connect(battleModel,SIGNAL(destroyed(QObject*)),this,SLOT(completeBattle()));

    emit battleActive(true);

    showBattleDialog();

    uploadBattleModel();
}

void BattleDialogManager::loadBattle(EncounterBattle* battleEncounter)
{
    if((!_campaign) || (!battleEncounter) || (!battleEncounter->getBattleDialogModel()))
        return;

    if(_dlg)
    {
        if(_encounterBattle == battleEncounter)
        {
            // If this battle was already open, just switch to the open dialog
            showBattleDialog();
            return;
        }

        QMessageBox::StandardButton result = QMessageBox::critical(_dlg, QString("Confirm Close Battle"), QString("There is an existing battle still ongoing. Are you sure you wish to end that battle to load a new one? All changes will be discarded."), QMessageBox::Yes | QMessageBox::No);
        if(result == QMessageBox::No)
            return;

        completeBattle();
    }

    BattleDialogModel* battleModel = battleEncounter->getBattleDialogModel();

    _encounterBattle = battleEncounter;
    _dlg = createBattleDialog(battleModel);
    if(!_dlg)
        return;

    connect(battleModel,SIGNAL(destroyed(QObject*)),this,SLOT(completeBattle()));
    _dlg->setBattleMap();
    emit battleActive(true);

    showBattleDialog();

    uploadBattleModel();
}

void BattleDialogManager::deleteBattle(EncounterBattle* battleEncounter)
{
    if((!_campaign) || (!battleEncounter) || (!battleEncounter->getBattleDialogModel()))
        return;

    if((_dlg) && (_encounterBattle == battleEncounter))
    {
        QMessageBox::StandardButton result = QMessageBox::critical(_dlg, QString("Confirm Close Battle"), QString("The selected battle is ongoing. Are you sure you wish to close and remove that battle? All changes will be discarded."), QMessageBox::Yes | QMessageBox::No);
        if(result == QMessageBox::No)
            return;

        completeBattle();
    }

    battleEncounter->removeBattleDialogModel();
    emit dirty();
}

void BattleDialogManager::addMonsters()
{
    if(!_dlg)
        return;

    qDebug() << "[Battle Dialog Manager] Adding monsters ...";

    QPointF combatantPos = _dlg->viewportCenter();

    AddMonstersDialog monsterDlg(_dlg);
    int result = monsterDlg.exec();
    if(result == QDialog::Accepted)
    {
        MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(monsterDlg.getMonsterType());
        int monsterCount = monsterDlg.getMonsterCount();
        if(!monsterClass)
        {
            qDebug() << "[Battle Dialog Manager] ... not able to find the selected class";
            return;
        }

        QString baseName = monsterDlg.getMonsterName().isEmpty() ? monsterClass->getName() : monsterDlg.getMonsterName();

        qDebug() << "[Battle Dialog Manager] ... adding " << monsterCount << " monsters of type " << baseName;

        for(int i = 0; i < monsterCount; ++i)
        {
            BattleDialogModelMonsterClass* monster = new BattleDialogModelMonsterClass(monsterClass);
            if( monsterCount == 1)
            {
                monster->setMonsterName(baseName);
            }
            else
            {
                monster->setMonsterName(baseName + QString("#") + QString::number(i+1));
            }
            monster->setHitPoints(monsterClass->getHitDice().roll());
            monster->setInitiative(Dice::d20() + Combatant::getAbilityMod(monsterClass->getDexterity()));
            monster->setPosition(combatantPos);
            _dlg->addCombatant(monster);
        }

        _dlg->recreateCombatantWidgets();
    }
    else
    {
        qDebug() << "[Battle Dialog Manager] ... add monsters dialog cancelled";
    }
}

void BattleDialogManager::addWave()
{
    if((!_dlg) || (!_encounterBattle))
        return;

    qDebug() << "[Battle Dialog Manager] adding wave (waves available: " << _encounterBattle->getWaveCount() << ") ...";

    AddBattleWaveDialog waveDlg(_encounterBattle, _dlg);
    int result = waveDlg.exec();
    if((result == QDialog::Accepted) && (waveDlg.selectedWave() >= 1))
    {
        int selectedWave = waveDlg.selectedWave();
        qDebug() << "[Battle Dialog Manager] ... wave " << selectedWave << " selected with " << _encounterBattle->getCombatantCount(selectedWave) << " combatants";
        //_dlg->getModel().appendCombatants( createWaveMonsters(battle, selectedWave) );
        _dlg->addCombatants(createWaveMonsters(_encounterBattle, selectedWave));
        _dlg->recreateCombatantWidgets();
    }
    else
    {
        qDebug() << "[Battle Dialog Manager] ... add wave dialog cancelled";
    }
}

void BattleDialogManager::addCharacter()
{
    if((!_campaign) || (!_dlg))
        return;

    qDebug() << "[Battle Dialog Manager] adding character...";

    ItemSelectDialog characterSelectDlg;
    characterSelectDlg.setWindowTitle(QString("Select a Character"));
    characterSelectDlg.setLabel(QString("Select Character:"));

    QList<BattleDialogModelCombatant*> combatantList = _dlg->getModel().getCombatantList();

    for(int i = 0; i < _campaign->getCharacterCount(); ++i)
    {
        Character* character = _campaign->getCharacterByIndex(i);
        if(character)
        {
            bool found = false;
            QListIterator<BattleDialogModelCombatant*> i(combatantList);
            while((!found) && (i.hasNext()))
            {
                BattleDialogModelCombatant* combatant = i.next();
                if((combatant) && (combatant->getCombatant()) && (combatant->getCombatant()->getID() == character->getID()))
                {
                    found = true;
                }
            }

            if(!found)
            {
                characterSelectDlg.addItem(character->getName(), QVariant::fromValue(character) );
            }
        }
    }

    if(characterSelectDlg.getItemCount() > 0)
    {
        if(characterSelectDlg.exec() == QDialog::Accepted)
        {
            Character* selectedCharacter = characterSelectDlg.getSelectedData().value<Character*>();
            if(selectedCharacter)
            {
                BattleDialogModelCharacter* newCharacter = new BattleDialogModelCharacter(characterSelectDlg.getSelectedData().value<Character*>());
                newCharacter->setPosition(_dlg->viewportCenter());
                _dlg->addCombatant(newCharacter);
                _dlg->recreateCombatantWidgets();
                qDebug() << "[Battle Dialog Manager] ... character " << selectedCharacter->getName() << " added.";
            }
        }
        else
        {
            qDebug() << "[Battle Dialog Manager] ... add character dialog cancelled";
        }
    }
    else
    {
        QMessageBox::information(_dlg, QString("Add Character"), QString("No further characters could be found to add to the current battle."));
        qDebug() << "[Battle Dialog Manager] ... no characters found to add";
    }
}

void BattleDialogManager::addNPC()
{
    if((!_campaign) || (!_dlg))
        return;

    qDebug() << "[Battle Dialog Manager] adding NPCs...";

    ItemSelectDialog characterSelectDlg;
    characterSelectDlg.setWindowTitle(QString("Select an NPC"));
    characterSelectDlg.setLabel(QString("Select NPC:"));

    QList<BattleDialogModelCombatant*> combatantList = _dlg->getModel().getCombatantList();

    for(int i = 0; i < _campaign->getNPCCount(); ++i)
    {
        Character* character = _campaign->getNPCByIndex(i);
        if(character)
        {
            bool found = false;
            QListIterator<BattleDialogModelCombatant*> i(combatantList);
            while((!found) && (i.hasNext()))
            {
                BattleDialogModelCombatant* combatant = i.next();
                if((combatant) && (combatant->getCombatant()) && (combatant->getCombatant()->getID() == character->getID()))
                {
                    found = true;
                }
            }

            if(!found)
            {
                characterSelectDlg.addItem(character->getName(), QVariant::fromValue(character) );
            }
        }
    }

    if(characterSelectDlg.getItemCount() > 0)
    {
        if(characterSelectDlg.exec() == QDialog::Accepted)
        {
            Character* selectedCharacter = characterSelectDlg.getSelectedData().value<Character*>();
            if(selectedCharacter)
            {
                BattleDialogModelCharacter* newCharacter = new BattleDialogModelCharacter(characterSelectDlg.getSelectedData().value<Character*>());
                newCharacter->setPosition(_dlg->viewportCenter());
                _dlg->addCombatant(newCharacter);
                _dlg->recreateCombatantWidgets();
                qDebug() << "[Battle Dialog Manager] ... NPC " << selectedCharacter->getName() << " added.";
            }
        }
        else
        {
            qDebug() << "[Battle Dialog Manager] ... add NPC dialog cancelled";
        }
    }
    else
    {
        QMessageBox::information(_dlg, QString("Add NPC"), QString("No further NPCs could be found to add to the current battle."));
        qDebug() << "[Battle Dialog Manager] ... no NPCs found to add";
    }
}

void BattleDialogManager::targetResized(const QSize& newSize)
{
    _targetSize = newSize;

    if(_dlg)
        _dlg->setTargetSize(newSize);
}

void BattleDialogManager::setShowOnDeck(bool showOnDeck)
{
    _showOnDeck = showOnDeck;
    if(_dlg)
        _dlg->setShowOnDeck(showOnDeck);
}

void BattleDialogManager::setShowCountdown(bool showCountdown)
{
    _showCountdown = showCountdown;
    if(_dlg)
        _dlg->setShowCountdown(showCountdown);
}

void BattleDialogManager::setCountdownDuration(int countdownDuration)
{
    _countdownDuration = countdownDuration;
    if(_dlg)
        _dlg->setCountdownDuration(countdownDuration);
}

void BattleDialogManager::completeBattle()
{
    delete _dlg;
    _dlg = nullptr;
    _encounterBattle = nullptr;
    emit battleActive(false);
    emit dirty();
}

void BattleDialogManager::selectBattleMap(Map* preselectedMap, Adventure* relatedAdventure)
{
    if(!_dlg)
        return;

    Map* battleMap = preselectedMap ? preselectedMap : selectRelatedMap(relatedAdventure);
    if(!battleMap)
        return;

    battleMap->initialize();
    QImage fowImage = battleMap->getPublishImage();

    SelectZoom zoomDlg(fowImage, _dlg);
    zoomDlg.exec();
    if(zoomDlg.result() == QDialog::Accepted)
    {
        _dlg->getModel().setMap(battleMap, zoomDlg.getZoomRect());
        _dlg->setBattleMap();
    }
}

void BattleDialogManager::selectBattleMap()
{
    if(_encounterBattle)
    {
        selectBattleMap(nullptr, _encounterBattle->getAdventure());
    }
    else
    {
        selectBattleMap(nullptr, nullptr);
    }
}

void BattleDialogManager::uploadBattleModel()
{
    if((!_dlg) || (!_networkManager))
        return;

    qDebug() << "[Battle Dialog Manager] Uploading current battle model.";

    QDomDocument doc( "DMHelperXML" );
    QDomElement root = doc.createElement( "root" );
    doc.appendChild(root);

    QDir emptyDir;
    _dlg->getModel().outputXML(doc, root, emptyDir);

    _networkManager->setPayload(QString("battle"), doc.toString());
}

BattleDialog* BattleDialogManager::createBattleDialog(BattleDialogModel* dlgModel)
{
    if(!_campaign)
        return nullptr;

    BattleDialog* dlg = new BattleDialog(*dlgModel);
    connect(dlg,SIGNAL(characterSelected(int)),this,SIGNAL(characterSelected(int)));
    connect(dlg,SIGNAL(monsterSelected(QString)),this,SIGNAL(monsterSelected(QString)));
    connect(dlg,SIGNAL(publishImage(QImage)),this,SIGNAL(publishImage(QImage)));
    connect(dlg,SIGNAL(animateImage(QImage)),this,SIGNAL(animateImage(QImage)));

    connect(dlg,SIGNAL(battleComplete()),this,SLOT(completeBattle()));
    connect(dlg,SIGNAL(selectNewMap()),this,SLOT(selectBattleMap()));
    connect(dlg,SIGNAL(addMonsters()),this,SLOT(addMonsters()));
    connect(dlg,SIGNAL(addWave()),this,SLOT(addWave()));
    connect(dlg,SIGNAL(addCharacter()),this,SLOT(addCharacter()));
    connect(dlg,SIGNAL(addNPC()),this,SLOT(addNPC()));

    dlg->setShowOnDeck(_showOnDeck);
    dlg->setShowCountdown(_showCountdown);
    dlg->setCountdownDuration(_countdownDuration);

    dlg->setTargetSize(_targetSize);
    dlg->recreateCombatantWidgets();

    emit dirty();

    return dlg;
}

Map* BattleDialogManager::selectRelatedMap(Adventure* adventure)
{
    // Todo: Check what happens if separator is selected
    MapSelectDialog mapSelectDlg;
    //mapSelectDlg.setWindowTitle(QString("Select a Map"));
    //mapSelectDlg.setLabel(QString("Select Map:"));

    if(adventure)
    {
        for(int i = 0; i < adventure->getMapCount(); ++i)
        {
            mapSelectDlg.addMap(adventure->getMapByIndex(i));
            /*
            Map* map = adventure->getMapByIndex(i);
            if(map)
            {
                mapSelectDlg.addItem(map->getName(), QVariant::fromValue(map) );
            }
            */
        }

        //mapSelectDlg.addSeparator(mapSelectDlg.getItemCount());
    }

    if(_campaign)
    {
        for(int i = 0; i < _campaign->getSettingCount(); i++)
        {
            mapSelectDlg.addMap(_campaign->getSettingByIndex(i));
            /*
            Map* map = _campaign->getSettingByIndex(i);
            if(map)
            {
                mapSelectDlg.addItem(map->getName(), QVariant::fromValue(map) );
            }
            */
        }
    }

    if(mapSelectDlg.exec() != QDialog::Accepted)
        return nullptr;
    else
        return mapSelectDlg.getSelectedMap();
}

QList<BattleDialogModelCombatant*> BattleDialogManager::createWaveMonsters(EncounterBattle* battle, int wave)
{
    QList<BattleDialogModelCombatant*> result;

    QPointF combatantPos;
    if(_dlg)
        combatantPos = _dlg->viewportCenter();

    CombatantGroupList combatantsList = battle->getCombatants(wave);
    for(int i = 0; i < combatantsList.count(); ++i)
    {
        CombatantGroup combatantGroup = combatantsList.at(i);
        if((combatantGroup.second) && (combatantGroup.second->getType() == DMHelper::CombatantType_Monster))
        {
            Monster* monster = dynamic_cast<Monster*>(combatantGroup.second);
            if(monster)
            {
                QString baseName = combatantGroup.second->getName().isEmpty() ? monster->getMonsterClassName() : combatantGroup.second->getName();

                for(int n = 0; n < combatantGroup.first; ++n)
                {
                    BattleDialogModelMonsterCombatant* newMonster = new BattleDialogModelMonsterCombatant(monster,
                                                                                        (combatantGroup.first > 1) ? baseName + QString("#") + QString::number(n+1) : baseName,
                                                                                        -1,
                                                                                        (monster->getHitPoints() > 0) ? monster->getHitPoints() : monster->getHitDice().roll());
                    newMonster->setInitiative(Dice::d20() + Combatant::getAbilityMod(monster->getDexterity()));                    
                    newMonster->setPosition(combatantPos);
                    result.append(newMonster);
                }
            }
        }
    }

    return result;
}
