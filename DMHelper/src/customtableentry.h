#ifndef CUSTOMTABLEENTRY_H
#define CUSTOMTABLEENTRY_H

#include <QDomElement>

class CustomTableEntry
{
public:
    CustomTableEntry(const QString& text, int weight = 1, const QString& subtable = QString());
    CustomTableEntry(const QDomElement& element);
    CustomTableEntry(const CustomTableEntry& other);

    QString getText() const;
    void setText(const QString& text);

    int getWeight() const;
    void setWeight(int weight);

    QString getSubtable() const;
    void setSubtable(const QString& subtable);

protected:
    QString _text;
    int _weight;
    QString _subtable;
};

#endif // CUSTOMTABLEENTRY_H
