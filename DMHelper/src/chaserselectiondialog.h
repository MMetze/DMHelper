#ifndef CHASERSELECTIONDIALOG_H
#define CHASERSELECTIONDIALOG_H

#include <QDialog>
#include "combatant.h"

namespace Ui {
class ChaserSelectionDialog;
}

class Character;
class QListWidget;
class QListWidgetItem;

class ChaserSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChaserSelectionDialog(QWidget *parent = 0);
    ~ChaserSelectionDialog();

    void addCharacters(QList<Character*> characters);
    void clearCharacters();

    void addEncounterCombatants(CombatantGroupList combatants);
    void clearEncounterCombatants();

    void addBattleCombatants(QList<Combatant*> combatants);
    void clearBattleCombatants();

    QList<Combatant*> getPursuers() const;
    CombatantGroupList getPursuerGroups() const;
    QList<Combatant*> getQuarry() const;
    CombatantGroupList getQuarryGroups() const;

public slots:
    void addPursuer();
    void addQuarry();
    void removePursuer();
    void removeQuarry();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:

    void moveSelectedToList(QListWidget* list);
    void moveListToList(QListWidget* from, QListWidget* to);
    void removeSelectedFromList(QListWidget* list);

    QList<Combatant*> getListCombatants(QListWidget* list) const;
    CombatantGroupList getListGroups(QListWidget* list) const;

    Ui::ChaserSelectionDialog *ui;

    enum WidgetType
    {
        WidgetType_Character = 0,
        WidgetType_Encounter,
        WidgetType_Battle,
        WidgetType_Local
    };

    enum WidgetData_CombatantCount
    {
        WidgetData_Pointer = Qt::UserRole,
        WidgetData_Type,
        WidgetData_CombatantCount

    };

};

#endif // CHASERSELECTIONDIALOG_H
