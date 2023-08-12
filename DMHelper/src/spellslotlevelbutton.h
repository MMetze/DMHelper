#ifndef SPELLSLOTLEVELBUTTON_H
#define SPELLSLOTLEVELBUTTON_H

#include <QPushButton>

class SpellSlotLevelButton : public QPushButton
{
    Q_OBJECT
public:
    SpellSlotLevelButton(const QString& text, int level);

signals:
    void spellSlotLevelClicked(int level);

protected slots:
    void cascadeClicked();

protected:
    int _level;
};

#endif // SPELLSLOTLEVELBUTTON_H
