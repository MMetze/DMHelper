#include "customtable.h"
#include "dice.h"
#include <QDebug>

CustomTable::CustomTable(const QString& tableName, const QList<CustomTableEntry>& entryList, int totalWeights, bool visible) :
    _tableName(tableName),
    _entryList(entryList),
    _totalWeights(totalWeights),
    _visible(visible)
{}

CustomTable::CustomTable(const QDomElement& element, const QString& tableFileName) :
    _tableName(element.attribute(QString("name"))),
    _entryList(),
    _totalWeights(0),
    _visible(static_cast<bool>(element.attribute("visible", QString::number(1)).toInt()))
{
    if(_tableName.isEmpty())
    {
        qDebug() << "[CustomTableFrame] WARNING: Custom table file dmhelperlist item missing name attribute";
        _tableName = QString("ERROR: No Table Name: ") + tableFileName;
    }

    QDomElement entryElement = element.firstChildElement(QString("entry"));
    while(!entryElement.isNull())
    {
        CustomTableEntry entry(entryElement);
        _entryList.append(CustomTableEntry(entryElement));
        _totalWeights += entry.getWeight();
        entryElement = entryElement.nextSiblingElement(QString("entry"));
    }
}

CustomTable::CustomTable(const CustomTable& other) :
    _tableName(other._tableName),
    _entryList(other._entryList),
    _totalWeights(other._totalWeights),
    _visible(other._visible)
{
}

CustomTable::~CustomTable()
{
    _entryList.clear();
}

QString CustomTable::getName() const
{
    return _tableName;
}

void CustomTable::setName(const QString& tableName)
{
    _tableName = tableName;
}

QList<CustomTableEntry> CustomTable::getEntryList() const
{
    return _entryList;
}

void CustomTable::setEntryList(const QList<CustomTableEntry>& entryList)
{
    _entryList = entryList;
}

int CustomTable::getTotalWeights() const
{
    return _totalWeights;
}

void CustomTable::setTotalWeights(int totalWeights)
{
    _totalWeights = totalWeights;
}

bool CustomTable::isVisible() const
{
    return _visible;
}

void CustomTable::setVisible(bool visible)
{
    _visible = visible;
}

CustomTableEntry CustomTable::getRandomEntry()
{
    if(_totalWeights > 0)
    {
        int rollValue = Dice::dX(_totalWeights) - 1;
        for(int i = 0; i < _entryList.count(); ++i)
        {
            if(rollValue < _entryList.at(i).getWeight())
                return _entryList.at(i);

            rollValue -= _entryList.at(i).getWeight();
        }
    }

    return CustomTableEntry(QString());
}
