#ifndef COMBATANTROLLOVERFRAME_H
#define COMBATANTROLLOVERFRAME_H

#include <QFrame>
#include "monsteraction.h"

namespace Ui {
class CombatantRolloverFrame;
}

class BattleDialogModelCombatant;
class BattleDialogModelCharacter;
class BattleDialogModelMonsterBase;

class CombatantRolloverFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CombatantRolloverFrame(BattleDialogModelCombatant* combatant, QWidget *parent = nullptr);
    ~CombatantRolloverFrame();

private:
    void readCombatant(BattleDialogModelCombatant* combatant);
    void readCharacter(BattleDialogModelCharacter* character);
    void readMonster(BattleDialogModelMonsterBase* monster);
    void addActionList(const QList<MonsterAction>& actionList, const QString& listTitle);

    Ui::CombatantRolloverFrame *ui;
};

#endif // COMBATANTROLLOVERFRAME_H
