#ifndef ENCOUNTERBATTLEEDIT_H
#define ENCOUNTERBATTLEEDIT_H

#include <QFrame>

namespace Ui {
class EncounterBattleEdit;
}

class QTreeWidgetItem;
class EncounterBattle;
class AudioTrack;

class EncounterBattleEdit : public QFrame
{
    Q_OBJECT

public:
    explicit EncounterBattleEdit(QWidget *parent = nullptr);
    ~EncounterBattleEdit();

    EncounterBattle* getBattle() const;
    void setBattle(EncounterBattle* battle);
    void unsetBattle(EncounterBattle* battle);

signals:
    void widgetChanged();
    void startBattle(EncounterBattle* battle);
    void loadBattle(EncounterBattle* battle);
    void deleteBattle(EncounterBattle* battle);
    void openMonster(const QString& monsterClass);
    void startTrack(AudioTrack* track);

public slots:
    void addWave();
    void removeWave();
    void addCombatant();
    void editCombatant(QTreeWidgetItem * item, int column);
    void removeCombatant();
    void selectionChanged();
    void textChanged();
    void clear();
    void updateStatus();
    void calculateThresholds();

protected slots:
    void startBattleClicked();
    void loadBattleClicked();
    void deleteBattleClicked();
    void trackSelected(int index);

protected:
    // From QWidget
    virtual void showEvent(QShowEvent *event);

private:
    void updateCombatantList();
    void loadTracks();

    Ui::EncounterBattleEdit *ui;
    EncounterBattle* _battle;

    enum EncounterBattleEdit_CombatantList
    {
        EncounterBattleEdit_WaveId = Qt::UserRole,
        EncounterBattleEdit_CombatantIndex

    };
};

#endif // ENCOUNTERBATTLEEDIT_H
