#ifndef CHARACTERWIDGET_H
#define CHARACTERWIDGET_H

#include "combatantwidget.h"
#include "character.h"

class BattleDialogModelCharacter;
class QLayout;
class QHBoxLayout;
class QCheckBox;

class CharacterWidget : public CombatantWidget
{
    Q_OBJECT
public:
    explicit CharacterWidget(BattleDialogModelCharacter* character, QWidget *parent = 0);

    virtual BattleDialogModelCombatant* getCombatant();
    BattleDialogModelCharacter* getCharacter();

signals:

    void clicked(int characterID);

public slots:
    // From CombatantWidget
    virtual void updateData();

protected slots:
    virtual void setHighlighted(bool highlighted);
    virtual void hitPointsEdited();

protected:

    // From CombatantWidget
    virtual void executeDoubleClick();

    // Data
    BattleDialogModelCharacter* _character;

    // UI elements
    QHBoxLayout* pairName;
    QHBoxLayout* pairArmorClass;
    QLineEdit* _edtHP;

};

#endif // CHARACTERWIDGET_H
