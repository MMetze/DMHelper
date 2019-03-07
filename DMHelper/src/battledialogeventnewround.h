#ifndef BATTLEDIALOGEVENTNEWROUND_H
#define BATTLEDIALOGEVENTNEWROUND_H

#include "battledialogevent.h"

class BattleDialogEventNewRound : public BattleDialogEvent
{
    Q_OBJECT
public:
    explicit BattleDialogEventNewRound();
    BattleDialogEventNewRound(const QDomElement& element);
    BattleDialogEventNewRound(const BattleDialogEventNewRound& other);
    virtual ~BattleDialogEventNewRound();

    virtual int getType() const;

    virtual void outputXML(QDomElement &element);

signals:

public slots:
};

#endif // BATTLEDIALOGEVENTNEWROUND_H
