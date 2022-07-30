#include "spellslotradiobutton.h"

SpellSlotRadioButton::SpellSlotRadioButton(int level, int slot, bool checked) :
    QRadioButton(),
    _level(level),
    _slot(slot)
{
    setChecked(checked);
    setAutoExclusive(false);

    connect(this, &QAbstractButton::clicked, this, &SpellSlotRadioButton::cascadeClicked);
    connect(this, &QAbstractButton::toggled, this, &SpellSlotRadioButton::cascadeToggled);
}

void SpellSlotRadioButton::cascadeClicked(bool checked)
{
    emit spellSlotClicked(_level, _slot, checked);
}

void SpellSlotRadioButton::cascadeToggled(bool checked)
{
    emit spellSlotToggled(_level, _slot, checked);
}
