#ifndef BESTIARY_H
#define BESTIARY_H

#include <QObject>
#include <QMap>
#include <QDir>
#include <QStringList>

class MonsterClass;
class Monster;
class QDomDocument;
class QDomElement;

typedef QMap<QString, MonsterClass*> BestiaryMap;

class Bestiary : public QObject
{
    Q_OBJECT
public:
    explicit Bestiary(QObject *parent = nullptr);
    ~Bestiary();

    static Bestiary* Instance();
    static void Initialize();
    static void Shutdown();

    int outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) const;
    void inputXML(const QDomElement &element, bool isImport);

    QString getVersion() const;
    int getMajorVersion() const;
    int getMinorVersion() const;
    bool isVersionCompatible() const;
    bool isVersionIdentical() const;
    static QString getExpectedVersion();

    bool exists(const QString& name) const;
    int count() const;
    QList<QString> getMonsterList() const;
    QStringList getLicenseText() const;

    MonsterClass* getMonsterClass(const QString& name);
    MonsterClass* getFirstMonsterClass() const;
    MonsterClass* getLastMonsterClass() const;
    MonsterClass* getNextMonsterClass(MonsterClass* monsterClass) const;
    MonsterClass* getPreviousMonsterClass(MonsterClass* monsterClass) const;

    bool insertMonsterClass(MonsterClass* monsterClass);
    void removeMonsterClass(MonsterClass* monsterClass);
    void renameMonster(MonsterClass* monsterClass, const QString& newName);

    void setDirectory(const QDir& directory);
    const QDir& getDirectory() const;

    Monster* createMonster(const QString& name);
    Monster* createMonster(const QDomElement& element, bool isImport);

    QString findMonsterImage(const QString& monsterName, const QString& iconFile);

signals:
    void changed();

public slots:
    void startBatchProcessing();
    void finishBatchProcessing();

private:
    void showMonsterClassWarning(const QString& monsterClass);

    static Bestiary* _instance;

    BestiaryMap _bestiaryMap;
    QDir _bestiaryDirectory;
    int _majorVersion;
    int _minorVersion;
    QStringList _licenseText;

    bool _batchProcessing;
    bool _batchAcknowledge;
};

#endif // BESTIARY_H
