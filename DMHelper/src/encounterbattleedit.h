#ifndef ENCOUNTERBATTLEEDIT_H
#define ENCOUNTERBATTLEEDIT_H

#include <QFrame>

class QTreeWidget;
class QTreeWidgetItem;
class QTextEdit;
class QPushButton;
class QComboBox;
class QLabel;
class EncounterBattle;
class AudioTrack;

class EncounterBattleEdit : public QFrame
{
    Q_OBJECT
public:
    explicit EncounterBattleEdit(QWidget *parent = 0);

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

protected slots:
    void startBattleClicked();
    void loadBattleClicked();
    void deleteBattleClicked();
    void trackSelected(int index);

protected:

    void updateCombatantList();
    void loadTracks();

    QTreeWidget* _tree;
    QTextEdit* _textEdit;
    QComboBox* _cmbTracks;
    QPushButton* _addWaveButton;
    QPushButton* _deleteWaveButton;
    QPushButton* _addButton;
    QPushButton* _deleteButton;
    QPushButton* _loadBattle;
    QPushButton* _deleteBattle;
    QPushButton* _startBattle;
    QLabel* _totalXP;
    QLabel* _challengeRating;

    EncounterBattle* _battle;

    enum EncounterBattleEdit_CombatantList
    {
        EncounterBattleEdit_WaveId = Qt::UserRole,
        EncounterBattleEdit_CombatantIndex

    };
};

#endif // ENCOUNTERBATTLEEDIT_H
