#ifndef SPELLBOOK_H
#define SPELLBOOK_H

#include <QObject>
#include <QMap>
#include <QDir>
#include <QStringList>

class Spell;
class Monster;
class QDomDocument;
class QDomElement;

typedef QMap<QString, Spell*> SpellbookMap;

class Spellbook : public QObject
{
    Q_OBJECT
public:
    explicit Spellbook(QObject *parent = nullptr);
    ~Spellbook();

    static Spellbook* Instance();
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
    QList<QString> getSpellList() const;
    QStringList getLicenseText() const;

    Spell* getSpell(const QString& name);
    Spell* getFirstSpell() const;
    Spell* getLastSpell() const;
    Spell* getNextSpell(Spell* spell) const;
    Spell* getPreviousSpell(Spell* spell) const;

    bool insertSpell(Spell* spell);
    void removeSpell(Spell* spell);
    void renameSpell(Spell* spell, const QString& newName);

    void setDirectory(const QDir& directory);
    const QDir& getDirectory() const;

    QString findSpellImage(const QString& spellName, const QString& iconFile);

signals:
    void changed();

public slots:
    void startBatchProcessing();
    void finishBatchProcessing();

private:
    void showSpellWarning(const QString& spell);

    static Spellbook* _instance;

    SpellbookMap _spellbookMap;
    QDir _spellbookDirectory;
    int _majorVersion;
    int _minorVersion;
    QStringList _licenseText;

    bool _batchProcessing;
    bool _batchAcknowledge;
};

#endif // SPELLBOOK_H
