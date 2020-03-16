#ifndef CUSTOMTABLE_H
#define CUSTOMTABLE_H

#include "customtableentry.h"
#include <QList>

class CustomTable
{
public:
    CustomTable(const QString& tableName = QString(), const QList<CustomTableEntry>& entryList = QList<CustomTableEntry>(), int totalWeights = 0, bool visible = true);
    CustomTable(const QDomElement& element, const QString& tableFileName);
    CustomTable(const CustomTable& other);
    virtual ~CustomTable();

    QString getName() const;
    void setName(const QString& tableName);

    QList<CustomTableEntry> getEntryList() const;
    void setEntryList(const QList<CustomTableEntry>& entryList);

    int getTotalWeights() const;
    void setTotalWeights(int totalWeights);

    bool isVisible() const;
    void setVisible(bool visible);

    CustomTableEntry getRandomEntry();

protected:
    QString _tableName;
    QList<CustomTableEntry> _entryList;
    int _totalWeights;
    bool _visible;


};

#endif // CUSTOMTABLE_H
