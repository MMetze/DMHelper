#ifndef PARTYFRAMECHARACTER_H
#define PARTYFRAMECHARACTER_H

#include <QFrame>

namespace Ui {
class PartyFrameCharacter;
}

class Characterv2;

class PartyFrameCharacter : public QFrame
{
    Q_OBJECT

public:
    explicit PartyFrameCharacter(Characterv2& character, QWidget *parent = nullptr);
    virtual ~PartyFrameCharacter();

private:
    Ui::PartyFrameCharacter *ui;
};

#endif // PARTYFRAMECHARACTER_H
