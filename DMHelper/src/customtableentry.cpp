#include "customtableentry.h"

CustomTableEntry::CustomTableEntry(const QString& text, int weight, const QString& subtable, QObject *parent) :
    QObject(parent),
    _text(text),
    _weight(weight),
    _subtable(subtable)
{
}

CustomTableEntry::CustomTableEntry(const QDomElement& element, QObject *parent) :
    QObject(parent),
    _text(element.text()),
    _weight(1),
    _subtable(element.attribute(QString("subtable"),QString()))
{
    bool ok = false;
    _weight = element.attribute(QString("weight"),QString("1")).toInt(&ok);
    if(!ok)
        _weight = 1;
}

CustomTableEntry::CustomTableEntry(const CustomTableEntry& other) :
    QObject(other.parent()),
    _text(other._text),
    _weight(other._weight),
    _subtable(other._subtable)
{
}

QString CustomTableEntry::getText() const
{
    return _text;
}

void CustomTableEntry::setText(const QString& text)
{
    _text = text;
}

int CustomTableEntry::getWeight() const
{
    return _weight;
}

void CustomTableEntry::setWeight(int weight)
{
    _weight = weight;
}

QString CustomTableEntry::getSubtable() const
{
    return _subtable;
}

void CustomTableEntry::setSubtable(const QString& subtable)
{
    _subtable = subtable;
}
