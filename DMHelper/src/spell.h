#ifndef SPELL_H
#define SPELL_H

#include "dice.h"
#include "combatant.h"
#include <QObject>
#include <QList>
#include <QDomElement>
#include <QDir>
#include <QSize>
#include <QColor>

class Spell : public QObject
{
    Q_OBJECT
public:
    explicit Spell(const QString& name, QObject *parent = nullptr);
    explicit Spell(const QDomElement &element, bool isImport, QObject *parent = nullptr);

    void inputXML(const QDomElement &element, bool isImport);
    void inputXML_CONVERT(const QDomElement &element);
    QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const;

    void beginBatchChanges();
    void endBatchChanges();

    void cloneSpell(Spell& other);

    static void outputValue(QDomDocument &doc, QDomElement &element, bool isExport, const QString& valueName, const QString& valueText);

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
    int getEffectConditions() const;
    bool hasEffectCondition(Combatant::Condition condition) const;

signals:
    void iconChanged();
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
    void setEffectType(int effectType);
    void setEffectShapeActive(bool effectShapeActive);
    void setEffectSize(QSize effectSize);
    void setEffectColor(const QColor& effectColor);
    void setEffectToken(QString effectToken);
    void setEffectTokenRotation(int effectTokenRotation);
    void setEffectConditions(int conditions);
    void applyEffectConditions(int conditions);
    void addEffectCondition(Combatant::Condition condition);
    void removeEffectCondition(Combatant::Condition condition);

    void clearRolls();

private:
    void registerChange();

    QString _name;
    int _level;
    QString _school;
    QString _time;
    QString _range;
    QString _components;
    QString _duration;
    QString _classes;
    QString _description;
    bool _ritual;
    QList<Dice> _rolls;

    int _effectType;
    bool _effectShapeActive;
    QSize _effectSize;
    QColor _effectColor;
    QString _effectToken;
    int _effectTokenRotation;
    int _effectConditions;

    bool _batchChanges;
    bool _changesMade;
};

#endif // SPELL_H
