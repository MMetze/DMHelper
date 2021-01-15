#ifndef QUICKREF_H
#define QUICKREF_H

#include <QObject>
#include <QStringList>
#include <QDomElement>
#include <QMap>

class QuickRefSection;
class QuickRefSubsection;
class QuickRefData;

class QuickRef : public QObject
{
    Q_OBJECT
public:
    explicit QuickRef(QObject *parent = nullptr);
    virtual ~QuickRef();

    static QuickRef* Instance();
    static void Initialize();
    static void Shutdown();

    QString getVersion() const;
    int getMajorVersion() const;
    int getMinorVersion() const;
    bool isVersionCompatible() const;
    bool isVersionIdentical() const;
    static QString getExpectedVersion();

    bool exists(const QString& sectionName) const;
    int count() const;
    QStringList getSectionList() const;
    QuickRefSection* getSection(const QString& sectionName);
    QuickRefSubsection* getSubsection(const QString& sectionName, int subSectionIndex);
    QuickRefData* getData(const QString& sectionName, int subSectionIndex, const QString& dataTitle);

signals:
    void changed();

public slots:
    void readQuickRef(const QString& quickRefFile);

private:
    static QuickRef* _instance;

    QMap<QString, QuickRefSection*> _quickRefData;
    int _majorVersion;
    int _minorVersion;
};

class QuickRefSection
{
public:
    explicit QuickRefSection(QDomElement &element, const QString& iconDir);
    ~QuickRefSection();

    QString getName() const;
    QString getLimitation() const;

    int count() const;
    QuickRefSubsection* getSubsection(int index) const;
    QList<QuickRefSubsection*> getSubsections() const;

private:
    QString _name;
    QString _limitation;
    QList<QuickRefSubsection*> _subSections;
};

class QuickRefSubsection
{
public:
    explicit QuickRefSubsection(QDomElement &element, const QString& iconDir);
    ~QuickRefSubsection();

    QString getDescription() const;

    bool exists(const QString& dataName) const;
    int count() const;
    QStringList getDataTitles() const;
    QuickRefData* getData(const QString& dataTitle);

private:
    QString _description;
    QMap<QString, QuickRefData*> _data;
};

class QuickRefData
{
public:
    explicit QuickRefData(QDomElement &element, const QString& iconDir);
    ~QuickRefData();

    QString getTitle() const;
    QString getIcon() const;
    QString getSubtitle() const;
    QString getDescription() const;
    QString getReference() const;
    QStringList getBullets() const;
    QString getOverview() const;

private:
    QString _title;
    QString _icon;
    QString _subtitle;
    QString _description;
    QString _reference;
    QStringList _bullets;
};

#endif // QUICKREF_H
