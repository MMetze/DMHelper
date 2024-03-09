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
class CombatantWidget;
class QListWidgetItem;

class CombatantRolloverFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CombatantRolloverFrame(CombatantWidget* combatantWidget, QWidget *parent = nullptr);
    ~CombatantRolloverFrame();

signals:
    void hoverEnded();

public slots:
    void triggerClose();
    void cancelClose();

protected slots:
    void handleItemClicked(QListWidgetItem *item);

protected:
    virtual void leaveEvent(QEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;

private:
    void readCombatant(BattleDialogModelCombatant* combatant);
    void readCharacter(BattleDialogModelCharacter* character);
    void readMonster(BattleDialogModelMonsterBase* monster);
    void addActionList(const QList<MonsterAction>& actionList, const QString& listTitle);
    void addSeparator();
    void addSectionTitle(const QString& sectionTitle);

    Ui::CombatantRolloverFrame *ui;
    CombatantWidget* _widget;
    int _closeTimer;
};

#endif // COMBATANTROLLOVERFRAME_H
