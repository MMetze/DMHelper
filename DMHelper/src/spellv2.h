#ifndef SPELLV2_H
#define SPELLV2_H

#include "templateobject.h"
#include "dice.h"
#include <QObject>
#include <QSize>
#include <QColor>
#include <QStringList>
#include <QList>

class QDomElement;
class QDomDocument;
class QDir;

// Value-bag spell model. Mirrors MonsterClassv2 / Characterv2 — all fields live
// in TemplateObject::valueHash() and are driven by the spell5e.xml schema via
// SpellbookFactory. Thin getters/setters preserve the original Spell API so
// existing call sites (battleframe.cpp, mainwindow.cpp, etc.) keep compiling
// after Spellv2Converter migrates the legacy XML.
class Spellv2 : public QObject, public TemplateObject
{
    Q_OBJECT
public:
    explicit Spellv2(const QString& name, QObject *parent = nullptr);
    explicit Spellv2(const QDomElement &element, bool isImport, QObject *parent = nullptr);

    void inputXML(const QDomElement &element, bool isImport);
    QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const;

    void beginBatchChanges();
    void endBatchChanges();

    void cloneSpell(Spellv2& other);

    // ---- API parity with the legacy Spell class -----------------------------
    QString getName() const;
    int getLevel() const;
    QString getSchool() const;
    QString getTime() const;
    QString getRange() const;
    QString getComponents() const;
    QString getDuration() const;
    QString getClasses() const;
    QString getDescription() const;
    bool isRitual() const;
    QList<Dice> getRolls() const;
    QString getRollsString() const;
    int getEffectType() const;
    bool getEffectShapeActive() const;
    QSize getEffectSize() const;
    QColor getEffectColor() const;
    QString getEffectToken() const;
    QString getEffectTokenPath() const;
    int getEffectTokenRotation() const;
    QStringList getEffectConditionList() const;
    bool hasEffectCondition(const QString& conditionId) const;

signals:
    void dirty();

public slots:
    void setName(const QString& name);
    void setLevel(int level);
    void setSchool(const QString& school);
    void setTime(const QString& time);
    void setRange(const QString& range);
    void setComponents(const QString& components);
    void setDuration(const QString& duration);
    void setClasses(const QString& classes);
    void setDescription(const QString& description);
    void setRitual(bool ritual);
    void setRolls(const QList<Dice>& rolls);
    void addRoll(const Dice& roll);
    void addRoll(const QString& roll);
    void clearRolls();
    void setEffectType(int effectType);
    void setEffectShapeActive(bool effectShapeActive);
    void setEffectSize(QSize effectSize);
    void setEffectColor(const QColor& effectColor);
    void setEffectToken(QString effectToken);
    void setEffectTokenRotation(int effectTokenRotation);
    void setEffectConditionList(const QStringList& conditions);
    void addEffectCondition(const QString& conditionId);
    void removeEffectCondition(const QString& conditionId);

protected:
    virtual QHash<QString, QVariant>* valueHash() override;
    virtual const QHash<QString, QVariant>* valueHash() const override;
    virtual void declareDirty() override;

    void registerChange();

private:
    QHash<QString, QVariant> _allValues;
    bool _batchChanges;
    bool _changesMade;
};

#endif // SPELLV2_H
