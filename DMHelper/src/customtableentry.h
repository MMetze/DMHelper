#ifndef CUSTOMTABLEENTRY_H
#define CUSTOMTABLEENTRY_H

#include <QObject>
#include <QDomElement>

class CustomTableEntry : public QObject
{
    Q_OBJECT
public:
    CustomTableEntry(const QString& text, int weight = 1, const QString& subtable = QString(), QObject *parent = nullptr);
    CustomTableEntry(const QDomElement& element, QObject *parent = nullptr);
    CustomTableEntry(const CustomTableEntry& other);

    QString getText() const;
    void setText(const QString& text);

    int getWeight() const;
    void setWeight(int weight);

    QString getSubtable() const;
    void setSubtable(const QString& subtable);

signals:

public slots:

protected:
    QString _text;
    int _weight;
    QString _subtable;
};

#endif // CUSTOMTABLEENTRY_H
