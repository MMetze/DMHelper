#ifndef SPELLBOOKDIALOG_H
#define SPELLBOOKDIALOG_H

#include "combatant.h"
#include <QDialog>

namespace Ui {
class SpellbookDialog;
}

class Spell;
class Spellbook;
class QHBoxLayout;

class SpellbookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpellbookDialog(QWidget *parent = nullptr);
    ~SpellbookDialog();

    Spell* getSpell() const;

signals:
    void spellChanged(); // todo connect this
    void spellDataEdit(); // used internally
    void dialogClosed();

public slots:
    void setSpell(Spell* spell);
    void setSpell(const QString& spellName);

    void previousSpell();
    void nextSpell();
    void createNewSpell();
    void deleteCurrentSpell();

    void dataChanged(); // todo call this

protected slots:
    void handleEditedData();
    void handleEffectChanged(int index);
    void handleWidthChanged();
    void handleHeightChanged();
    void handleTokenRotateCW();
    void handleTokenRotateCCW();
    void editConditions();
    void selectToken();
    void updateLayout();
    void clearGrid();
    void addCondition(Combatant::Condition condition);

protected:
    // From QWidget
    virtual void closeEvent(QCloseEvent * event);
    virtual void showEvent(QShowEvent * event);
    virtual void hideEvent(QHideEvent * event);
    virtual void focusOutEvent(QFocusEvent * event);

private:
    void storeSpellData();
    void updateImage();

    Ui::SpellbookDialog *ui;
    Spell* _spell;
    int _tokenRotation;
    QHBoxLayout* _conditionLayout;
};

#endif // SPELLBOOKDIALOG_H
