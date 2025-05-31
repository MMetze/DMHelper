#include "battledialoglogview.h"
#include "ui_battledialoglogview.h"
#include "battledialoglogger.h"
#include "battledialogmodel.h"
#include "battledialogevent.h"
#include "battledialogeventdamage.h"
#include "battledialogmodelmonsterbase.h"
#include "monsterclassv2.h"
#include <QList>

BattleDialogLogView::BattleDialogLogView(const BattleDialogModel& model, const BattleDialogLogger& logger, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BattleDialogLogView)
{
    ui->setupUi(this);

    int rounds = 1;
    int totalPartyDamage = 0;
    int totalEnemyDamage = 0;
    int totalExperience = 0;

    QList<BattleDialogEvent*> events = logger.getEvents();
    for(BattleDialogEvent* event : events)
    {
        if(event->getType() == DMHelper::BattleEvent_NewRound)
            ++rounds;
    }
    ui->lblRounds->setText(QString::number(rounds));

    QList<BattleDialogModelCombatant*> combatants = model.getCombatantList();
    for(BattleDialogModelCombatant* combatant : combatants)
    {
        QTableWidgetItem* itemCombatant = new QTableWidgetItem(QIcon(combatant->getIconPixmap(DMHelper::PixmapSize_Thumb)), combatant->getName());

        int taken = 0;
        int dealt = 0;
        for(BattleDialogEvent* event : events)
        {
            BattleDialogEventDamage* damageEvent = dynamic_cast<BattleDialogEventDamage*>(event);
            if(damageEvent)
            {
                if(damageEvent->getTarget() == combatant->getID())
                    taken -= damageEvent->getDamage();

                if(damageEvent->getCombatant() == combatant->getID())
                    dealt -= damageEvent->getDamage();
            }
        }

        QTableWidgetItem* itemTaken = new QTableWidgetItem();
        itemTaken->setData(Qt::EditRole, taken);
        QTableWidgetItem* itemDealt = new QTableWidgetItem();
        itemDealt->setData(Qt::EditRole, dealt);
        QTableWidgetItem* itemPerRnd = new QTableWidgetItem();
        itemPerRnd->setData(Qt::EditRole, static_cast<qreal>(dealt) / static_cast<qreal>(rounds));

        if(combatant->getCombatantType() == DMHelper::CombatantType_Character)
        {
            ui->tblParty->setRowCount(ui->tblParty->rowCount() + 1);
            ui->tblParty->setItem(ui->tblParty->rowCount() - 1, 0, itemCombatant);
            ui->tblParty->setItem(ui->tblParty->rowCount() - 1, 1, itemTaken);
            ui->tblParty->setItem(ui->tblParty->rowCount() - 1, 2, itemDealt);
            ui->tblParty->setItem(ui->tblParty->rowCount() - 1, 3, itemPerRnd);
            totalPartyDamage += dealt;
        }
        else
        {
            ui->tblEnemies->setRowCount(ui->tblEnemies->rowCount() + 1);
            ui->tblEnemies->setItem(ui->tblEnemies->rowCount() - 1, 0, itemCombatant);
            ui->tblEnemies->setItem(ui->tblEnemies->rowCount() - 1, 1, itemTaken);
            ui->tblEnemies->setItem(ui->tblEnemies->rowCount() - 1, 2, itemDealt);
            ui->tblEnemies->setItem(ui->tblEnemies->rowCount() - 1, 3, itemPerRnd);
            totalEnemyDamage += dealt;
            BattleDialogModelMonsterBase* monsterBase = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
            if((monsterBase) && (monsterBase->getMonsterClass()))
                totalExperience += monsterBase->getMonsterClass()->getIntValue("experience");
        }
    }

    ui->lblDamageParty->setText(QString::number(totalPartyDamage));
    ui->lblDamageEnemies->setText(QString::number(totalEnemyDamage));
    ui->lblExperience->setText(QString::number(totalExperience));

    ui->tblParty->setSortingEnabled(true);
    ui->tblParty->resizeColumnsToContents();
    ui->tblEnemies->setSortingEnabled(true);
    ui->tblEnemies->resizeColumnsToContents();
}

BattleDialogLogView::~BattleDialogLogView()
{
    delete ui;
}
