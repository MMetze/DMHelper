#ifndef BESTIARY_H
#define BESTIARY_H

#include "globalsearch.h"
#include <QObject>
#include <QMap>
#include <QDir>
#include <QStringList>

class MonsterClass;
class MonsterClassv2;
class Monster;
class QDomDocument;
class QDomElement;

typedef QMap<QString, MonsterClassv2*> BestiaryMap;

class Bestiary : public QObject, public GlobalSearch_Interface
{
    Q_OBJECT
public:
    explicit Bestiary(QObject *parent = nullptr);
    virtual ~Bestiary();

    // Static interface
    static Bestiary* Instance();
    static void Initialize();
    static void Shutdown();

    // GlobalSearch_Interface
    QStringList search(const QString& searchString) override;

    // Local Interface
    bool writeBestiary(const QString& targetFilename);
    int outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) const;
    void inputXML(const QDomElement &element, const QString& importFile = QString());

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

    bool isDirty();

    MonsterClassv2* getMonsterClass(const QString& name);
    MonsterClassv2* getFirstMonsterClass() const;
    MonsterClassv2* getLastMonsterClass() const;
    MonsterClassv2* getNextMonsterClass(MonsterClassv2* monsterClass) const;
    MonsterClassv2* getPreviousMonsterClass(MonsterClassv2* monsterClass) const;

    bool insertMonsterClass(MonsterClassv2* monsterClass);
    void removeMonsterClass(MonsterClassv2* monsterClass);
    void renameMonster(MonsterClassv2* monsterClass, const QString& newName);

    void setDirectory(const QDir& directory);
    const QDir& getDirectory() const;

    Monster* createMonster(const QString& name);
    Monster* createMonster(const QDomElement& element, bool isImport);

    QString findMonsterImage(const QString& monsterName, const QString& iconFile);
    QString findMonsterImage(const QDir& sourceDir, const QString& monsterName);
    QStringList findMonsterImages(const QString& monsterName);
    QStringList findSpecificImages(const QDir& sourceDir, const QStringList& filterList, const QString& filterName = QString());

signals:
    void changed();
    void bestiaryLoaded(const QString& bestiaryFile);

public slots:
    bool readBestiary(const QString& targetFilename);

    void startBatchProcessing();
    void finishBatchProcessing();

    void registerDirty();
    void setDirty(bool dirty = true);

    void startBatchChanges();
    void finishBatchChanges();
    void registerChange();

private:
    void showMonsterClassWarning(const QString& monsterClass);
    void loadAndConvertBestiary(const QDomElement& bestiaryElement);
    void loadBestiary(const QDomElement& bestiaryElement);
    void importBestiary(const QDomElement& bestiaryElement, const QString& importFile);
    void importMonsterImage(const QDomElement& monsterElement, const QString& importFile);

    static Bestiary* _instance;

    BestiaryMap _bestiaryMap;
    QDir _bestiaryDirectory;
    QString _bestiaryFile;
    int _majorVersion;
    int _minorVersion;
    QStringList _licenseText;
    bool _dirty;

    bool _batchProcessing;
    bool _batchAcknowledge;

    bool _batchChanges;
    bool _changesMade;
};

#endif // BESTIARY_H
