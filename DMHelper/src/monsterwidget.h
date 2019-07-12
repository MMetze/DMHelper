#ifndef MONSTERWIDGET_H
#define MONSTERWIDGET_H

#include "combatantwidget.h"
#include "monster.h"

class BattleDialogModelMonsterBase;
class QLayout;
class QHBoxLayout;
class QCheckBox;
class QLabel;
class QLineEdit;

class MonsterWidget : public CombatantWidget
{
    Q_OBJECT
public:
    explicit MonsterWidget(BattleDialogModelMonsterBase* monster, QWidget *parent = nullptr);

    virtual BattleDialogModelCombatant* getCombatant() override;
    BattleDialogModelMonsterBase* getMonster();

    virtual bool isShown() override;
    virtual bool isKnown() override;

signals:

    void clicked(const QString& monsterClass);
    void hitPointsChanged(int hp);


public slots:
    // From CombatantWidget
    virtual void updateData() override;

protected slots:
    // From CombatantWidget
    virtual void setHighlighted(bool highlighted) override;

    // local
    void localHitPointsChanged();

protected:

    // From CombatantWidget
    virtual void executeDoubleClick() override;

    // Data
    BattleDialogModelMonsterBase* _monster;

    // UI elements
    QHBoxLayout* _pairName;
    QHBoxLayout* _pairArmorClass;
    QLabel* _lblHP;
    QLineEdit* _edtHP;
    QCheckBox* _chkVisible;
    QCheckBox* _chkKnown;
};

#endif // MONSTERWIDGET_H
