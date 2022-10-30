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

    //void setCombatant(int combatantCount, Combatant* combatant);
    int getCount() const;
    QString getName() const;
    int getCombatantHitPoints() const;
    bool isRandomInitiative() const;
    QString getInitiative() const;
    bool isKnown() const;
    bool isShown() const;
    bool isCustomSize() const;
    QString getSizeFactor() const;
    MonsterClass* getMonsterClass() const;
    int getIconIndex() const;

    void writeCombatant(Combatant* combatant);

signals:
    void openMonster(const QString& monsterClass);

public slots:
    // From QDialog
    virtual void accept() override;

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private slots:
    void monsterClassChanged(const QString &text);
    void setIconIndex(int index);
    void updateIcon();
    void previousIcon();
    void nextIcon();
    void setHitPointAverageChanged();
    void openMonsterClicked();
    void sizeSelected(int index);

private:
    void fillSizeCombo();

    Ui::CombatantDialog *ui;
    //Combatant* _combatant;
    int _iconIndex;
    //int _count;

};

#endif // COMBATANTDIALOG_H
