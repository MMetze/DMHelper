#ifndef SPELLBOOKDIALOG_H
#define SPELLBOOKDIALOG_H

#include <QDialog>

namespace Ui {
class SpellbookDialog;
}

class Spell;
class Spellbook;

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

protected:
    // From QWidget
    virtual void closeEvent(QCloseEvent * event);
    virtual void showEvent(QShowEvent * event);
    virtual void hideEvent(QHideEvent * event);
    virtual void focusOutEvent(QFocusEvent * event);

private:
    void storeSpellData();

    Ui::SpellbookDialog *ui;
    Spell* _spell;
};

#endif // SPELLBOOKDIALOG_H
