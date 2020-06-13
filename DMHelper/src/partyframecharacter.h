#ifndef PARTYFRAMECHARACTER_H
#define PARTYFRAMECHARACTER_H

#include <QFrame>

namespace Ui {
class PartyFrameCharacter;
}

class Character;

class PartyFrameCharacter : public QFrame
{
    Q_OBJECT

public:
    explicit PartyFrameCharacter(Character& character, QWidget *parent = nullptr);
    virtual ~PartyFrameCharacter();

private:
    Ui::PartyFrameCharacter *ui;
};

#endif // PARTYFRAMECHARACTER_H
