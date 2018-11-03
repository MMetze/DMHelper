#include "encounterbattleedit.h"
#include "encounterbattle.h"
#include "monster.h"
#include "combatantdialog.h"
#include "bestiary.h"
#include "monsterclass.h"
#include "campaign.h"
#include "audiotrack.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QInputDialog>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

const int EncounterBattleEdit_CombatantIndexWave = -1;

EncounterBattleEdit::EncounterBattleEdit(QWidget *parent) :
    QFrame(parent),
    _tree(nullptr),
    _textEdit(nullptr),
    _addWaveButton(nullptr),
    _deleteWaveButton(nullptr),
    _addButton(nullptr),
    _deleteButton(nullptr),
    _loadBattle(nullptr),
    _deleteBattle(nullptr),
    _startBattle(nullptr),
    _totalXP(nullptr),
    _challengeRating(nullptr),
    _battle(nullptr)
{
    QVBoxLayout* vLayout = new QVBoxLayout();

    // Top List box
    _tree = new QTreeWidget();
    _tree->setSelectionMode(QAbstractItemView::SingleSelection);
    _tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    _tree->setHeaderHidden(true);
    _tree->setColumnCount(2);
    _tree->setExpandsOnDoubleClick(false);
    _tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    // Add/Delete buttons
    QHBoxLayout* hLayout = new QHBoxLayout();
    QLabel* lblTracks = new QLabel("Background Tracks:");
    _cmbTracks = new QComboBox();
    hLayout->addWidget(lblTracks);
    hLayout->addWidget(_cmbTracks);
    hLayout->addStretch();
    _addWaveButton = new QPushButton(QString("Add Wave"));
    _deleteWaveButton = new QPushButton(QString("Delete Wave"));
    _addButton = new QPushButton(QString("Add Monster"));
    _deleteButton = new QPushButton(QString("Delete Monster"));
    hLayout->addWidget(_addWaveButton);
    hLayout->addWidget(_deleteWaveButton);
    hLayout->addWidget(_addButton);
    hLayout->addWidget(_deleteButton);

    // Challenge Rating
    QHBoxLayout* hLayout2 = new QHBoxLayout();
    QLabel* totalXPLabel = new QLabel("Total XP:");
    _totalXP = new QLabel("0");
    _totalXP->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QLabel* challengeLabel = new QLabel("Challenge Rating:");
    _challengeRating = new QLabel("0");
    _challengeRating->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayout2->addWidget(totalXPLabel);
    hLayout2->addWidget(_totalXP);
    hLayout2->addWidget(challengeLabel);
    hLayout2->addWidget(_challengeRating);
    hLayout2->addStretch();
    _loadBattle = new QPushButton(QString("Open Battle"));
    hLayout2->addWidget(_loadBattle);
    _deleteBattle = new QPushButton(QString("End Battle"));
    hLayout2->addWidget(_deleteBattle);
    _startBattle = new QPushButton(QString("Start New Battle"));
    hLayout2->addWidget(_startBattle);

    // Bottom Text edit
    _textEdit = new QTextEdit();

    vLayout->addWidget(_tree);
    vLayout->addLayout(hLayout);
    vLayout->addLayout(hLayout2);
    vLayout->addWidget(_textEdit);

    setLayout(vLayout);

    connect(_addWaveButton,SIGNAL(clicked()),this,SLOT(addWave()));
    connect(_deleteWaveButton,SIGNAL(clicked()),this,SLOT(removeWave()));
    connect(_addButton,SIGNAL(clicked()),this,SLOT(addCombatant()));
    connect(_deleteButton,SIGNAL(clicked()),this,SLOT(removeCombatant()));
    connect(_tree,SIGNAL(itemSelectionChanged()),this,SLOT(selectionChanged()));
    connect(_tree,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(editCombatant(QTreeWidgetItem*,int)));
    connect(_textEdit,SIGNAL(textChanged()),this,SLOT(textChanged()));
    connect(_startBattle,SIGNAL(clicked()),this,SLOT(startBattleClicked()));
    connect(_loadBattle,SIGNAL(clicked()),this,SLOT(loadBattleClicked()));
    connect(_deleteBattle,SIGNAL(clicked()),this,SLOT(deleteBattleClicked()));

    _deleteButton->setEnabled(false);

    updateStatus();
}

EncounterBattle* EncounterBattleEdit::getBattle() const
{
    return _battle;
}

void EncounterBattleEdit::setBattle(EncounterBattle* battle)
{
    if(!battle)
        return;

    _textEdit->setHtml(battle->getText());
    _battle = battle;
    updateCombatantList();
    updateStatus();
    connect(battle,SIGNAL(changed()),this,SLOT(updateStatus()));

    _tree->setCurrentItem(_tree->topLevelItem(0));

    loadTracks();
}

void EncounterBattleEdit::unsetBattle(EncounterBattle* battle)
{
    Q_UNUSED(battle);
    _battle = nullptr;
    disconnect(battle, nullptr, this, nullptr);
}

void EncounterBattleEdit::addWave()
{
    if(!_battle)
        return;

    int waveCount = _battle->getWaveCount();
    _battle->insertWave(waveCount);
    updateCombatantList();
    QTreeWidgetItem* item = _tree->topLevelItem(waveCount);
    if(item)
    {
        _tree->setCurrentItem(item);
    }
}

void EncounterBattleEdit::removeWave()
{
    if(!_battle)
        return;

    if(_battle->getWaveCount() <= 1 )
        return;

    QTreeWidgetItem* selection = _tree->currentItem();
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
            _tree->setCurrentItem(_tree->topLevelItem(_battle->getWaveCount() - 1));
        }
        else
        {
            _tree->setCurrentItem(_tree->topLevelItem(waveId));
        }
    }
}

void EncounterBattleEdit::addCombatant()
{
    if(!_battle)
        return;

    QTreeWidgetItem* selection = _tree->currentItem();
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
        QTreeWidgetItem* item = _tree->topLevelItem(waveId);
        if(item)
            _tree->setCurrentItem(item->child(item->childCount() - 1));
    }
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

    CombatantDialog dlg(QDialogButtonBox::Save);
    connect(&dlg, SIGNAL(openMonster(QString)), this, SIGNAL(openMonster(QString)));
    dlg.setCombatant(combatants.at(index).first, combatants.at(index).second);
    dlg.exec();
    _battle->editCombatant(wave, index, dlg.getCount(), combatants.at(index).second);
    updateCombatantList();
}

void EncounterBattleEdit::removeCombatant()
{
    if(!_battle)
        return;

    QTreeWidgetItem* selection = _tree->currentItem();
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
            QTreeWidgetItem* item = _tree->topLevelItem(waveId);
            if(item)
            {
                if(item->childCount() == 0)
                {
                    _tree->setCurrentItem(item);
                }
                else
                {
                    if(combatantIndex >= item->childCount())
                    {
                        combatantIndex = item->childCount() - 1;
                    }
                    _tree->setCurrentItem(item->child(combatantIndex));
                }
            }
        }
    }
}

void EncounterBattleEdit::selectionChanged()
{
    if(!_battle)
    {
        _addButton->setEnabled(false);
        _deleteButton->setEnabled(false);
        _addWaveButton->setEnabled(false);
        _deleteWaveButton->setEnabled(false);
    }
    else
    {
        QTreeWidgetItem* selection = _tree->currentItem();
        _addButton->setEnabled(selection);
        _deleteButton->setEnabled( ( selection ) && ( selection->data(1,EncounterBattleEdit_CombatantIndex).toInt() != EncounterBattleEdit_CombatantIndexWave) );
        _addWaveButton->setEnabled(true);
        _deleteWaveButton->setEnabled(_battle->getWaveCount() > 1);
    }
}

void EncounterBattleEdit::textChanged()
{
    if(!_battle)
        return;

    _battle->setText(_textEdit->toHtml());
}

void EncounterBattleEdit::clear()
{
    _battle = nullptr;
    _tree->clear();
    _textEdit->setPlainText(QString(""));
    _totalXP->setText(QString(""));
    _challengeRating->setText(QString(""));
}

void EncounterBattleEdit::updateStatus()
{
    if(_battle)
    {
        //_loadBattle->setEnabled(_battle->getBattleDialogModel() != 0);
        //_deleteBattle->setEnabled(_battle->getBattleDialogModel() != 0);
        _loadBattle->setVisible(_battle->getBattleDialogModel() != nullptr);
        _deleteBattle->setVisible(_battle->getBattleDialogModel() != nullptr);
        _startBattle->setVisible(_battle->getBattleDialogModel() == nullptr);
    }
    else
    {
        //_loadBattle->setEnabled(false);
        //_deleteBattle->setEnabled(false);
        _loadBattle->setVisible(false);
        _deleteBattle->setVisible(false);
        _startBattle->setVisible(true);
    }
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
    if(!_battle)
        return;

    AudioTrack* track = _cmbTracks->itemData(index).value<AudioTrack*>();
    _battle->setAudioTrack(track);
}

void EncounterBattleEdit::updateCombatantList()
{
    if(!_battle)
        return;

    int totalXP = 0;
    int totalMonsters = 0;

    _tree->clear();
    int waveCount = _battle->getWaveCount();
    for(int w = 0; w < waveCount; w++)
    {
        QTreeWidgetItem* waveItem = new QTreeWidgetItem(_tree);
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

    _tree->expandAll();

    _totalXP->setText(QString::number(totalXP));

    int challengeRating;
    switch(totalMonsters)
    {
        case 0: case 1: challengeRating = totalXP; break;
        case 2: challengeRating = totalXP * 1.5; break;
        case 3: case 4: case 5: case 6: challengeRating = totalXP * 2; break;
        case 7: case 8: case 9: case 10: challengeRating = totalXP * 2.5; break;
        case 11: case 12: case 13: case 14: challengeRating = totalXP * 3; break;
        default: challengeRating = totalXP * 4; break;
    }

    _challengeRating->setText(QString::number(challengeRating));
}

void EncounterBattleEdit::loadTracks()
{
    if(!_battle)
        return;

    disconnect(_cmbTracks, SIGNAL(currentIndexChanged(int)), this, SLOT(trackSelected(int)));

    _cmbTracks->clear();
    _cmbTracks->addItem(QString("---"), QVariant::fromValue(0));

    Campaign* campaign = _battle->getCampaign();
    int currentIndex = 0;
    for(int i = 0; i < campaign->getTrackCount(); ++i)
    {
        AudioTrack* track = campaign->getTrackByIndex(i);
        _cmbTracks->addItem(track->getName(), QVariant::fromValue(track));
        if(_battle->getAudioTrackId() == track->getID())
            currentIndex = _cmbTracks->count() - 1;
    }
    _cmbTracks->setCurrentIndex(currentIndex);

    connect(_cmbTracks, SIGNAL(currentIndexChanged(int)), this, SLOT(trackSelected(int)));
}
