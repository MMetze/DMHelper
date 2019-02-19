#ifndef BATTLEDIALOGLOGGER_H
#define BATTLEDIALOGLOGGER_H

#include "dmhobjectbase.h"
#include <QList>

class BattleDialogEvent;

class BattleDialogLogger : public DMHObjectBase
{
    Q_OBJECT
public:
    explicit BattleDialogLogger(QObject *parent = nullptr);
    virtual ~BattleDialogLogger();
    BattleDialogLogger(const BattleDialogLogger& other) = delete;
    BattleDialogLogger& operator=(const BattleDialogLogger& other) = delete;

    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    virtual void inputXML(const QDomElement &element);
    virtual void postProcessXML(const QDomElement &element);

    virtual QList<BattleDialogEvent*> getEvents() const;

signals:

public slots:
    void damageDone(int combatantID, int targetID, int damage);
    void newRound();

protected:
    QList<BattleDialogEvent*> _battleEvents;

};

#endif // BATTLEDIALOGLOGGER_H
