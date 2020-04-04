#ifndef BATTLEDIALOGEVENTNEWROUND_H
#define BATTLEDIALOGEVENTNEWROUND_H

#include "battledialogevent.h"

class BattleDialogEventNewRound : public BattleDialogEvent
{
    Q_OBJECT
public:
    explicit BattleDialogEventNewRound();
    explicit BattleDialogEventNewRound(const QDomElement& element);
    BattleDialogEventNewRound(const BattleDialogEventNewRound& other);
    virtual ~BattleDialogEventNewRound() override;

    virtual int getType() const override;
    virtual BattleDialogEvent* clone() override;

    virtual QDomElement outputXML(QDomElement &element, bool isExport) override;

signals:

public slots:
};

#endif // BATTLEDIALOGEVENTNEWROUND_H
