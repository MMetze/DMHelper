#ifndef SPELLBOOK_H
#define SPELLBOOK_H

#include "globalsearch.h"
#include <QObject>
#include <QMap>
#include <QDir>
#include <QStringList>

class Spellv2;
class Monster;
class QDomDocument;
class QDomElement;

typedef QMap<QString, Spellv2*> SpellbookMap;

class Spellbook : public QObject, public GlobalSearch_Interface
{
    Q_OBJECT
public:
    explicit Spellbook(QObject *parent = nullptr);
    ~Spellbook();

    // Static Interface
    static Spellbook* Instance();
    static void Initialize();
    static void Shutdown();    

    // GlobalSearch_Interface
    QStringList search(const QString& searchString) override;

    // Local Interface
    bool writeSpellbook(const QString& targetFilename);
    bool readSpellbook(const QString& targetFilename);
    int outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) const;
    void inputXML(const QDomElement &element, bool isImport);
    void input_START_CONVERSION(const QDomElement &element);

    QString getVersion() const;
    int getMajorVersion() const;
    int getMinorVersion() const;
    bool isVersionCompatible() const;
    bool isVersionIdentical() const;
    static QString getExpectedVersion();

    bool exists(const QString& name) const;
    int count() const;
    QList<QString> getSpellList() const;
    QStringList getLicenseText() const;

    bool isDirty();

    Spellv2* getSpell(const QString& name);
    Spellv2* getFirstSpell() const;
    Spellv2* getLastSpell() const;
    Spellv2* getNextSpell(Spellv2* spell) const;
    Spellv2* getPreviousSpell(Spellv2* spell) const;

    bool insertSpell(Spellv2* spell);
    void removeSpell(Spellv2* spell);
    void renameSpell(Spellv2* spell, const QString& newName);

    void setDirectory(const QDir& directory);
    const QDir& getDirectory() const;

    QString findSpellImage(const QString& spellName, const QString& iconFile);

signals:
    void changed();

public slots:
    void startBatchProcessing();
    void finishBatchProcessing();

    void registerDirty();
    void setDirty(bool dirty = true);

private:
    void showSpellWarning(const QString& spell);
    QString searchSpell(const Spellv2* spell, const QString& searchString) const;

    static Spellbook* _instance;

    SpellbookMap _spellbookMap;
    QDir _spellbookDirectory;
    int _majorVersion;
    int _minorVersion;
    QStringList _licenseText;
    bool _dirty;

    bool _batchProcessing;
    bool _batchAcknowledge;
};

#endif // SPELLBOOK_H
