#ifndef COMBATANTDIALOG_H
#define COMBATANTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include "combatant.h"

class MonsterClass;

namespace Ui {
class CombatantDialog;
}

class CombatantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CombatantDialog(QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Close, QWidget *parent = nullptr);
    ~CombatantDialog();

    void setCombatant(int combatantCount, Combatant* combatant);
    int getCount() const;
    QString getName() const;
    QString getLocalHitPoints() const;
    MonsterClass* getMonsterClass() const;

    void writeCombatant(Combatant* combatant);

signals:
    void openMonster(const QString& monsterClass);

public slots:
    // From QDialog
    virtual void accept();

private slots:
    void monsterClassChanged(const QString &text);
    void useHitPointAverageChanged(bool useAverage);
    void openMonsterClicked();

private:

    Ui::CombatantDialog *ui;
    Combatant* _combatant;
    int _count;
};

#endif // COMBATANTDIALOG_H
