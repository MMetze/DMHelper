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
    explicit CharacterWidget(BattleDialogModelCharacter* character, QWidget *parent = nullptr);

    virtual BattleDialogModelCombatant* getCombatant() override;
    BattleDialogModelCharacter* getCharacter();

signals:

    void clicked(QUuid characterID);

public slots:
    // From CombatantWidget
    virtual void updateData() override;

protected slots:
    virtual void setHighlighted(bool highlighted) override;
    virtual void hitPointsEdited();

protected:

    // From CombatantWidget
    virtual void executeDoubleClick() override;

    // Data
    BattleDialogModelCharacter* _character;

    // UI elements
    QHBoxLayout* pairName;
    QHBoxLayout* pairArmorClass;
    QLineEdit* _edtHP;

};

#endif // CHARACTERWIDGET_H
