#ifndef BESTIARY_H
#define BESTIARY_H

#include <QObject>
#include <QMap>
#include <QDir>

class MonsterClass;
class Monster;
class QDomDocument;
class QDomElement;

typedef QMap<QString, MonsterClass*> BestiaryMap;

class Bestiary : public QObject
{
    Q_OBJECT
public:
    explicit Bestiary(QObject *parent = 0);
    ~Bestiary();

    static Bestiary* Instance();
    static void Initialize();
    static void Shutdown();

    void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory) const;
    void inputXML(const QDomElement &element);

    QString getVersion() const;
    bool isVersionCompatible() const;
    bool isVersionIdentical() const;
    QString getExpectedVersion() const;

    bool exists(const QString& name) const;
    int count() const;
    QList<QString> getMonsterList() const;

    MonsterClass* getMonsterClass(const QString& name) const;
    MonsterClass* getFirstMonsterClass() const;
    MonsterClass* getLastMonsterClass() const;
    MonsterClass* getNextMonsterClass(MonsterClass* monsterClass) const;
    MonsterClass* getPreviousMonsterClass(MonsterClass* monsterClass) const;

    bool insertMonsterClass(MonsterClass* monsterClass);
    void removeMonsterClass(MonsterClass* monsterClass);
    void renameMonster(MonsterClass* monsterClass, const QString& newName);

    void setDirectory(const QDir& directory);
    const QDir& getDirectory() const;

    Monster* createMonster(const QString& name) const;
    Monster* createMonster(const QDomElement& element) const;

    QString findMonsterImage(const QString& monsterName, const QString& iconFile);

signals:
    void changed();

public slots:

private:
    static Bestiary* _instance;

    BestiaryMap _bestiaryMap;
    QDir _bestiaryDirectory;
    int _majorVersion;
    int _minorVersion;
};

#endif // BESTIARY_H
