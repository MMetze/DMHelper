#include "addbattlewavedialog.h"
#include "encounterbattle.h"
#include "monster.h"
#include "ui_addbattlewavedialog.h"

AddBattleWaveDialog::AddBattleWaveDialog(EncounterBattle* battle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddBattleWaveDialog),
    _battle(battle)
{
    ui->setupUi(this);

    for(int i = 1; i < _battle->getWaveCount(); ++i)
    {
        ui->lstWaves->addItem(QString("Wave #") + QString::number(i));
    }

    connect(ui->lstWaves, SIGNAL(currentRowChanged(int)), this, SLOT(selectWave(int)));
    ui->lstWaves->setCurrentRow(0);
}

AddBattleWaveDialog::~AddBattleWaveDialog()
{
    delete ui;
}

int AddBattleWaveDialog::selectedWave() const
{
    return ui->lstWaves->currentRow() + 1;
}

void AddBattleWaveDialog::selectWave(int wave)
{
    if((!_battle) || (wave < 0) || (wave >= _battle->getWaveCount() - 1) || (selectedWave() == wave))
        return;

    CombatantGroupList combatants = _battle->getCombatants(wave + 1);
    ui->lstCombatants->clear();
    for(int i = 0; i < combatants.count(); ++i)
    {
        QString name = combatants.at(i).second->getName();
        if(name.isEmpty())
        {
            Monster* monster = dynamic_cast<Monster*>(combatants.at(i).second);
            if(monster)
            {
                name = monster->getMonsterClassName();
            }
        }

        ui->lstCombatants->addItem(QString::number(combatants.at(i).first) + QString("x ") + name);
    }
}
