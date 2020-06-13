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
    explicit BattleDialogLogger(const BattleDialogLogger& other);
    virtual ~BattleDialogLogger();

    BattleDialogLogger& operator=(const BattleDialogLogger& other) = delete;

    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport);
    virtual void inputXML(const QDomElement &element, bool isImport);
    //virtual void postProcessXML(const QDomElement &element, bool isImport);

    virtual QList<BattleDialogEvent*> getEvents() const;

    int getRounds() const;

signals:
    void roundsChanged();

public slots:
    void damageDone(QUuid combatantID, QUuid targetID, int damage);
    void newRound();

protected:
    QList<BattleDialogEvent*> _battleEvents;

};

#endif // BATTLEDIALOGLOGGER_H
