#ifndef MONSTERACTION_H
#define MONSTERACTION_H

#include <QString>
#include "dice.h"

class QDomDocument;
class QDomElement;

class MonsterAction
{
public:
    explicit MonsterAction(int attackBonus, const QString& description, const QString& name, Dice damageDice);
    explicit MonsterAction(const QDomElement &element, bool isImport);
    explicit MonsterAction(const QHash<QString, QVariant>& valueHash);
    MonsterAction(const MonsterAction& other);
    virtual ~MonsterAction();

    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, bool isExport) const;
    bool hasDiceSummary() const;
    QString summaryString() const;

    int getAttackBonus() const;
    void setAttackBonus(int attackBonus);

    QString getDescription() const;
    void setDescription(const QString& description);

    QString getName() const;
    void setName(const QString& actionName);

    Dice getDamageDice() const;
    void setDamageDice(Dice damageDice);

    MonsterAction &operator=(const MonsterAction &other);
    bool operator==(const MonsterAction &other) const;
    bool operator!=(const MonsterAction &other) const;

    static QString createSummaryString(const QHash<QString, QVariant>& valueHash);

private:
    int _attackBonus;
    QString _description;
    QString _name;
    Dice _damageDice;
};

#endif // MONSTERACTION_H
