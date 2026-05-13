#ifndef CONDITIONS_H
#define CONDITIONS_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QStringList>
#include <QPixmap>
#include <QDir>

class QDomDocument;
class QDomElement;
class QPaintDevice;

struct ConditionDefinition
{
    QString id;
    QString title;
    QString description;
    QString iconName;
    QString customIconPath;
    QString group;

    bool operator==(const ConditionDefinition& other) const;
    bool operator!=(const ConditionDefinition& other) const;
};

class Conditions : public QObject
{
    Q_OBJECT
public:
    explicit Conditions(QObject *parent = nullptr);
    virtual ~Conditions() override;

    // Active instance singleton — set when campaign loads
    static Conditions* activeConditions();
    static void setActiveConditions(Conditions* conditions);

    // Load from XML file
    bool loadFromFile(const QString& filename);

    // Delta operations for campaign overrides
    void applyDeltas(const QDomElement& deltasElement, const QDir& baseDir);
    void outputDeltas(QDomDocument& doc, QDomElement& parent, const Conditions& rulesetDefaults, const QDir& targetDirectory) const;
    bool hasDeltasFrom(const Conditions& rulesetDefaults) const;

    // Accessors
    int getConditionCount() const;
    QList<ConditionDefinition> getConditions() const;
    ConditionDefinition getCondition(const QString& id) const;
    bool hasConditionDef(const QString& id) const;
    QString getConditionIconPath(const QString& id) const;
    QString getConditionTitle(const QString& id) const;
    QString getConditionDescription(const QString& id) const;
    QStringList getConditionIds() const;

    // Icon cache
    QPixmap getConditionPixmap(const QString& id, int size);
    void invalidateIconCache(const QString& id);
    void clearIconCache();

    // Modification
    void addCondition(const ConditionDefinition& condition);
    void modifyCondition(const ConditionDefinition& condition);
    void removeCondition(const QString& id);
    void clear();

    // Rendering
    static void drawConditions(QPaintDevice* target, const QStringList& activeConditions);

    // Utility: generate description strings for a list of active condition IDs
    static QStringList getConditionStrings(const QStringList& conditionIds);

    // Migration from old bitmask system
    static QStringList migrateFromBitmask(int bitmask);

private:
    static QString resolveIconPath(const ConditionDefinition& def);

    static Conditions* _activeConditions;
    QList<ConditionDefinition> _conditions;
    QHash<QString, int> _conditionIndex; // id -> index in _conditions
    QHash<QString, QPixmap> _iconCache;  // "id_size" -> cached pixmap
};

#endif // CONDITIONS_H
