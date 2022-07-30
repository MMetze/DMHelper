#ifndef SPELLSLOTRADIOBUTTON_H
#define SPELLSLOTRADIOBUTTON_H

#include <QRadioButton>

class SpellSlotRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    SpellSlotRadioButton(int level, int slot, bool checked);

signals:
    void spellSlotClicked(int level, int slot, bool checked);
    void spellSlotToggled(int level, int slot, bool checked);

protected slots:
    void cascadeClicked(bool checked);
    void cascadeToggled(bool checked);

protected:
    int _level;
    int _slot;
};

#endif // SPELLSLOTRADIOBUTTON_H
