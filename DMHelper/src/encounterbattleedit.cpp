#include "encounterbattleedit.h"
#include "ui_encounterbattleedit.h"
#include "monster.h"
#include "combatantdialog.h"
#include "bestiary.h"
#include "monsterclass.h"
#include "campaign.h"
#include "character.h"
#include "audiotrack.h"
#include "encounterbattle.h"
#include "itemselectdialog.h"
#include "combatantreference.h"
#include <QMessageBox>
#include <QDebug>

// TODO: remove this

const int EncounterBattleEdit_CombatantIndexWave = -1;

EncounterBattleEdit::EncounterBattleEdit(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::EncounterBattleEdit),
    _battle(nullptr)
{
    ui->setupUi(this);

    connect(ui->btnAddWave,SIGNAL(clicked()),this,SLOT(addWave()));
    connect(ui->btnDeleteWave,SIGNAL(clicked()),this,SLOT(removeWave()));
    connect(ui->btnAddMonster,SIGNAL(clicked()),this,SLOT(addCombatant()));
    connect(ui->btnAddNPC,SIGNAL(clicked()),this,SLOT(addNPC()));
    connect(ui->btnDeleteMonster,SIGNAL(clicked()),this,SLOT(removeCombatant()));
    connect(ui->treeWidget,SIGNAL(itemSelectionChanged()),this,SLOT(selectionChanged()));
    connect(ui->btnEditMonster,SIGNAL(clicked()),this,SLOT(editCombatant()));
    connect(ui->treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(editCombatant(QTreeWidgetItem*,int)));
    connect(ui->textEdit,SIGNAL(textChanged()),this,SLOT(textChanged()));
    connect(ui->btnStartNewBattle,SIGNAL(clicked()),this,SLOT(startBattleClicked()));
    connect(ui->btnOpenBattle,SIGNAL(clicked()),this,SLOT(loadBattleClicked()));
    connect(ui->btnEndBattle,SIGNAL(clicked()),this,SLOT(deleteBattleClicked()));

    ui->btnDeleteMonster->setEnabled(false);

    updateStatus();
}

EncounterBattleEdit::~EncounterBattleEdit()
{
    delete ui;
}


EncounterBattle* EncounterBattleEdit::getBattle() const
{
    return _battle;
}

void EncounterBattleEdit::setBattle(EncounterBattle* battle)
{
    if(!battle)
        return;

    ui->btnEditMonster->setEnabled(false);

    ui->textEdit->setHtml(battle->getText());
    _battle = battle;
    updateCombatantList();
    updateStatus();
    calculateThresholds();
    connect(battle,SIGNAL(changed()),this,SLOT(updateStatus()));

    Campaign* campaign = dynamic_cast<Campaign*>(battle->getParentByType(DMHelper::CampaignType_Campaign));
    if(campaign)
        connect(campaign,SIGNAL(dirty()),this,SLOT(calculateThresholds()));

    ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));

    loadTracks();
}

void EncounterBattleEdit::unsetBattle(EncounterBattle* battle)
{
    if(battle)
    {
        disconnect(battle, nullptr, this, nullptr);
        Campaign* campaign = dynamic_cast<Campaign*>(battle->getParentByType(DMHelper::CampaignType_Campaign));
        if(campaign)
            disconnect(campaign, nullptr, this, nullptr);
    }

     ui->btnEditMonster->setEnabled(false);
    _battle = nullptr;
}

void EncounterBattleEdit::addWave()
{
    if(!_battle)
        return;

    int waveCount = _battle->getWaveCount();
    _battle->insertWave(waveCount);
    updateCombatantList();
    QTreeWidgetItem* item = ui->treeWidget->topLevelItem(waveCount);
    if(item)
    {
        ui->treeWidget->setCurrentItem(item);
    }
}

void EncounterBattleEdit::removeWave()
{
    if(!_battle)
        return;

    if(_battle->getWaveCount() <= 1 )
        return;

    QTreeWidgetItem* selection = ui->treeWidget->currentItem();
    if(selection)
    {
        int waveId = selection->data(1,EncounterBattleEdit_WaveId).toInt();
        QMessageBox::StandardButton result = QMessageBox::question(this,
                                                                   QString("Delete Combatant"),
                                                                   QString("Are you sure that you would like to remove Wave #" + QString::number(waveId)),
                                                                   QMessageBox::Yes | QMessageBox::No );
        if(result == QMessageBox::No)
            return;

        _battle->removeWave(waveId);
        updateCombatantList();
        if(_battle->getWaveCount() <= waveId)
        {
            ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(_battle->getWaveCount() - 1));
        }
        else
        {
            ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(waveId));
        }
    }
}

void EncounterBattleEdit::addCombatant()
{
    if(!_battle)
        return;

    QTreeWidgetItem* selection = ui->treeWidget->currentItem();
    if(!selection)
        return;

    CombatantDialog dlg(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&dlg, SIGNAL(openMonster(QString)), this, SIGNAL(openMonster(QString)));

    int result = dlg.exec();
    if(result == QDialog::Accepted)
    {
        MonsterClass* monsterClass = dlg.getMonsterClass();
        if(monsterClass == nullptr)
            return;

        Monster* newCombatant = new Monster(monsterClass);
        dlg.writeCombatant(newCombatant);

        int waveId = selection->data(1,EncounterBattleEdit_WaveId).toInt();
        _battle->addCombatant(waveId, dlg.getCount(), newCombatant);
        updateCombatantList();
        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(waveId);
        if(item)
            ui->treeWidget->setCurrentItem(item->child(item->childCount() - 1));
    }
}

void EncounterBattleEdit::addNPC()
{
    if(!_battle)
        return;

    QTreeWidgetItem* selection = ui->treeWidget->currentItem();
    if(!selection)
        return;

    Campaign* campaign = dynamic_cast<Campaign*>(_battle->getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return;

    ItemSelectDialog characterSelectDlg;
    characterSelectDlg.setWindowTitle(QString("Select an NPC"));
    characterSelectDlg.setLabel(QString("Select NPC:"));

    QList<Character*> characterList = campaign->findChildren<Character *>();
    for(Character* character : characterList)
    {
        if((character) &&
           (!character->isInParty()) &&
           (_battle->getCombatantById(character->getID(), character->getIntID()) == nullptr))
        {
            characterSelectDlg.addItem(character->getName(), QVariant::fromValue(character));
        }
    }
    /*
    for(int i = 0; i < campaign->getNPCCount(); ++i)
    {
        Character* character = campaign->getNPCByIndex(i);
        if((character) && (_battle->getCombatantById(character->getID(), character->getIntID()) == nullptr))
        {
            characterSelectDlg.addItem(character->getName(), QVariant::fromValue(character));
        }
    }
    */

    if(characterSelectDlg.getItemCount() > 0)
    {
        if(characterSelectDlg.exec() == QDialog::Accepted)
        {
            Character* selectedCharacter = characterSelectDlg.getSelectedData().value<Character*>();
            if(selectedCharacter)
            {
                CombatantReference* newReference = new CombatantReference(*selectedCharacter, _battle);
                int waveId = selection->data(1,EncounterBattleEdit_WaveId).toInt();
                _battle->addCombatant(waveId, 1, newReference);
                updateCombatantList();
                QTreeWidgetItem* item = ui->treeWidget->topLevelItem(waveId);
                if(item)
                    ui->treeWidget->setCurrentItem(item->child(item->childCount() - 1));
            }
        }
        else
        {
            qDebug() << "[EncounterBattleEdit] ... add NPC dialog cancelled";
        }
    }
    else
    {
        QMessageBox::information(this, QString("Add NPC"), QString("No further NPCs could be found to add to the current battle."));
        qDebug() << "[EncounterBattleEdit] ... no NPCs found to add";
    }
}

void EncounterBattleEdit::editCombatant()
{
    editCombatant(ui->treeWidget->currentItem(), 0);
}

void EncounterBattleEdit::editCombatant(QTreeWidgetItem * item, int column)
{
    Q_UNUSED(column);

    if((!item)||(!_battle))
        return;

    int wave = item->data(1,EncounterBattleEdit_WaveId).toInt();
    int index = item->data(1,EncounterBattleEdit_CombatantIndex).toInt();
    CombatantGroupList combatants = _battle->getCombatants(wave);
    if((index < 0) || (index >= combatants.count()))
        return;

    if(combatants.at(index).second->getCombatantType() == DMHelper::CombatantType_Monster)
    {
        CombatantDialog dlg(QDialogButtonBox::Save);
        connect(&dlg, SIGNAL(openMonster(QString)), this, SIGNAL(openMonster(QString)));
        //dlg.setCombatant(combatants.at(index).first, combatants.at(index).second);
        dlg.exec();
        _battle->editCombatant(wave, index, dlg.getCount(), combatants.at(index).second);
        updateCombatantList();
    }
}

void EncounterBattleEdit::removeCombatant()
{
    if(!_battle)
        return;

    QTreeWidgetItem* selection = ui->treeWidget->currentItem();
    if(selection)
    {
        int waveId = selection->data(1,EncounterBattleEdit_WaveId).toInt();
        int combatantIndex = selection->data(1,EncounterBattleEdit_CombatantIndex).toInt();

        if(combatantIndex != EncounterBattleEdit_CombatantIndexWave)
        {
            QMessageBox::StandardButton result = QMessageBox::question(this,
                                                                       QString("Delete Combatant"),
                                                                       QString("Are you sure that you would like to remove this combatant?"),
                                                                       QMessageBox::Yes | QMessageBox::No );
            if(result == QMessageBox::No)
                return;

            _battle->removeCombatant(waveId, combatantIndex);
            updateCombatantList();
            QTreeWidgetItem* item = ui->treeWidget->topLevelItem(waveId);
            if(item)
            {
                if(item->childCount() == 0)
                {
                    ui->treeWidget->setCurrentItem(item);
                }
                else
                {
                    if(combatantIndex >= item->childCount())
                    {
                        combatantIndex = item->childCount() - 1;
                    }
                    ui->treeWidget->setCurrentItem(item->child(combatantIndex));
                }
            }
        }
    }
}

void EncounterBattleEdit::selectionChanged()
{
    if(!_battle)
    {
        ui->btnAddMonster->setEnabled(false);
        ui->btnAddNPC->setEnabled(false);
        ui->btnEditMonster->setEnabled(false);
        ui->btnDeleteMonster->setEnabled(false);
        ui->btnAddWave->setEnabled(false);
        ui->btnDeleteWave->setEnabled(false);
    }
    else
    {
        QTreeWidgetItem* selection = ui->treeWidget->currentItem();
        int wave = EncounterBattleEdit_CombatantIndexWave;
        int index = EncounterBattleEdit_CombatantIndexWave;
        if(selection)
        {
            wave = selection->data(1,EncounterBattleEdit_WaveId).toInt();
            index = selection->data(1,EncounterBattleEdit_CombatantIndex).toInt();
        }

        CombatantGroupList combatants = _battle->getCombatants(wave);

        ui->btnAddMonster->setEnabled(selection);
        ui->btnAddNPC->setEnabled(selection);
        ui->btnEditMonster->setEnabled((index >= 0) && (index < combatants.count()) && (combatants.at(index).second->getCombatantType() == DMHelper::CombatantType_Monster));
        ui->btnDeleteMonster->setEnabled(index != EncounterBattleEdit_CombatantIndexWave);
        ui->btnAddWave->setEnabled(true);
        ui->btnDeleteWave->setEnabled(_battle->getWaveCount() > 1);
    }
}

void EncounterBattleEdit::textChanged()
{
    if(!_battle)
        return;

    _battle->setText(ui->textEdit->toHtml());
}

void EncounterBattleEdit::clear()
{
    _battle = nullptr;
    ui->treeWidget->clear();
    ui->textEdit->setPlainText(QString(""));
    ui->lblTotalXP->setText(QString(""));
    ui->lblTotalCR->setText(QString(""));
}

void EncounterBattleEdit::updateStatus()
{
    if(_battle)
    {
        ui->btnOpenBattle->setVisible(_battle->getBattleDialogModel() != nullptr);
        ui->btnEndBattle->setVisible(_battle->getBattleDialogModel() != nullptr);
        ui->btnStartNewBattle->setVisible(_battle->getBattleDialogModel() == nullptr);
    }
    else
    {
        ui->btnOpenBattle->setVisible(false);
        ui->btnEndBattle->setVisible(false);
        ui->btnStartNewBattle->setVisible(true);
    }
}

void EncounterBattleEdit::calculateThresholds()
{
    //Campaign* campaign = _battle ? _battle->getCampaign() : nullptr;
    Campaign* campaign = _battle ? dynamic_cast<Campaign*>(_battle->getParentByType(DMHelper::CampaignType_Campaign)) : nullptr;
    if(!campaign)
    {
        ui->lblEasy->setText(QString("---"));
        ui->lblMedium->setText(QString("---"));
        ui->lblHard->setText(QString("---"));
        ui->lblDeadly->setText(QString("---"));
        return;
    }

    int thresholds[4] = {0,0,0,0};

    QList<Character*> activeCharacters = campaign->getActiveCharacters();
    for(int i = 0; i < activeCharacters.count(); ++i)
    {
        Character* character = activeCharacters.at(i);
        for(int j = 0; j < 4; ++j)
        {
            thresholds[j] += character->getXPThreshold(j);
        }
    }

    ui->lblEasy->setText(QString::number(thresholds[DMHelper::XPThreshold_Easy]));
    ui->lblMedium->setText(QString::number(thresholds[DMHelper::XPThreshold_Medium]));
    ui->lblHard->setText(QString::number(thresholds[DMHelper::XPThreshold_Hard]));
    ui->lblDeadly->setText(QString::number(thresholds[DMHelper::XPThreshold_Deadly]));
}

void EncounterBattleEdit::startBattleClicked()
{
    if(_battle)
    {
        emit startBattle(_battle);
        emit startTrack(_battle->getAudioTrack());
    }
}

void EncounterBattleEdit::loadBattleClicked()
{
    if(_battle)
    {
        emit loadBattle(_battle);
        emit startTrack(_battle->getAudioTrack());
    }
}

void EncounterBattleEdit::deleteBattleClicked()
{
    if(_battle)
    {
        emit deleteBattle(_battle);
    }
}

void EncounterBattleEdit::trackSelected(int index)
{
    Q_UNUSED(index);

    if(!_battle)
        return;

    AudioTrack* track = ui->cmbTracks->itemData(index).value<AudioTrack*>();
    _battle->setAudioTrack(track);
}

void EncounterBattleEdit::showEvent(QShowEvent *event)
{
    calculateThresholds();
    QFrame::showEvent(event);
}

void EncounterBattleEdit::updateCombatantList()
{
    if(!_battle)
        return;

    int totalXP = 0;
    int totalMonsters = 0;

    ui->treeWidget->clear();
    int waveCount = _battle->getWaveCount();
    for(int w = 0; w < waveCount; w++)
    {
        QTreeWidgetItem* waveItem = new QTreeWidgetItem(ui->treeWidget);
        waveItem->setText(0, QString("Wave ") + QString::number(w));
        waveItem->setData(1, EncounterBattleEdit_WaveId, w);
        waveItem->setData(1, EncounterBattleEdit_CombatantIndex, EncounterBattleEdit_CombatantIndexWave);
        CombatantGroupList combatants = _battle->getCombatants(w);
        for(int i = 0; i < combatants.count(); ++i)
        {
            CombatantGroup combatantPair = combatants.at(i);
            if(combatantPair.second)
            {
                QTreeWidgetItem* combatantItem = new QTreeWidgetItem(waveItem);
                Monster* monster = dynamic_cast<Monster*>(combatantPair.second);

                combatantItem->setText(0, QString::number(combatantPair.first) + QString("x"));
                QString name = combatantPair.second->getName();
                if(monster)
                {
                    if(name.isEmpty())
                    {
                        name = monster->getMonsterClassName();
                    }
                    else
                    {
                        name += " (" + monster->getMonsterClassName() + ")";
                    }
                }
                combatantItem->setText(1, name);
                combatantItem->setData(1, EncounterBattleEdit_WaveId, w);
                combatantItem->setData(1, EncounterBattleEdit_CombatantIndex, i);

                if(monster)
                {
                    totalXP += monster->getMonsterClass()->getXP() * combatantPair.first;
                    totalMonsters += combatantPair.first;
                }
            }
        }
    }

    ui->treeWidget->expandAll();

    ui->lblTotalXP->setText(QString::number(totalXP));

    int challengeRating;
    switch(totalMonsters)
    {
        case 0: case 1: challengeRating = totalXP; break;
        case 2: challengeRating = static_cast<int>(totalXP * 1.5); break;
        case 3: case 4: case 5: case 6: challengeRating = static_cast<int>(totalXP * 2); break;
        case 7: case 8: case 9: case 10: challengeRating = static_cast<int>(totalXP * 2.5); break;
        case 11: case 12: case 13: case 14: challengeRating = static_cast<int>(totalXP * 3); break;
        default: challengeRating = static_cast<int>(totalXP * 4); break;
    }

    ui->lblTotalCR->setText(QString::number(challengeRating));
}

void EncounterBattleEdit::loadTracks()
{
    if(!_battle)
        return;

    disconnect(ui->cmbTracks, SIGNAL(currentIndexChanged(int)), this, SLOT(trackSelected(int)));

    ui->cmbTracks->clear();
    ui->cmbTracks->addItem(QString("---"), QVariant::fromValue(0));

    //Campaign* campaign = _battle->getCampaign();
    Campaign* campaign = dynamic_cast<Campaign*>(_battle->getParentByType(DMHelper::CampaignType_Campaign));
    QList<AudioTrack*> trackList = campaign->findChildren<AudioTrack*>();
    int currentIndex = 0;
    //for(int i = 0; i < campaign->getTrackCount(); ++i)
    for(AudioTrack* track : trackList)
    {
        //AudioTrack* track = campaign->getTrackByIndex(i);
        ui->cmbTracks->addItem(track->getName(), QVariant::fromValue(track));
        if(_battle->getAudioTrackId() == track->getID())
            currentIndex = ui->cmbTracks->count() - 1;
    }
    ui->cmbTracks->setCurrentIndex(currentIndex);

    connect(ui->cmbTracks, SIGNAL(currentIndexChanged(int)), this, SLOT(trackSelected(int)));
}
