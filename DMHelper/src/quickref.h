#ifndef QUICKREF_H
#define QUICKREF_H

#include <QStringList>
#include <QDomElement>

class QuickRefData
{
public:
    explicit QuickRefData(QDomElement &element);

    QString getTitle() const;
    QString getIcon() const;
    QString getSubtitle() const;
    QString getDescription() const;
    QString getReference() const;
    QStringList getBullets() const;

private:
    QString _title;
    QString _icon;
    QString _subtitle;
    QString _description;
    QString _reference;
    QStringList _bullets;
};

class QuickRefSubsection
{
public:
    explicit QuickRefSubsection(QDomElement &element);

    QString getDescription() const;
    QList<QuickRefData> getData() const;

private:
    QString _description;
    QList<QuickRefData> _data;
};

class QuickRefSection
{
public:
    explicit QuickRefSection(QDomElement &element);

    QString getName() const;
    QString getLimitation() const;
    QList<QuickRefSubsection> getSubsections() const;

private:
    QString _name;
    QString _limitation;
    QList<QuickRefSubsection> _subSections;
};

#endif // QUICKREF_H
