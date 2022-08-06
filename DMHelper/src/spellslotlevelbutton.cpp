#include "spellslotlevelbutton.h"

SpellSlotLevelButton::SpellSlotLevelButton(const QString& text, int level) :
    QPushButton(text),
    _level(level)
{
    connect(this, &QPushButton::clicked, this, &SpellSlotLevelButton::cascadeClicked);
}

void SpellSlotLevelButton::cascadeClicked()
{
    emit spellSlotLevelClicked(_level);
}
