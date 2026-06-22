#ifndef BATTLEDIALOGMODELCHARACTER_H
#define BATTLEDIALOGMODELCHARACTER_H

#include "battledialogmodelcombatant.h"
#include "combatant.h"
#include "scaledpixmap.h"
#include <QString>
#include <QPoint>

class Characterv2;

class BattleDialogModelCharacter : public BattleDialogModelCombatant
{
    Q_OBJECT

public:
    BattleDialogModelCharacter(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelCharacter(Characterv2* character);
    explicit BattleDialogModelCharacter(Characterv2* character, int initiative, const QPointF& position);
    virtual ~BattleDialogModelCharacter() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    // Local
    virtual int getCombatantType() const override;
    virtual BattleDialogModelCombatant* clone() const override;
    virtual qreal getSizeFactor() const override;
    virtual int getSizeCategory() const override;

    virtual int getStrength() const override;
    virtual int getDexterity() const override;
    virtual int getConstitution() const override;
    virtual int getIntelligence() const override;
    virtual int getWisdom() const override;
    virtual int getCharisma() const override;
    virtual int getSkillModifier(Combatant::Skills skill) const override;
    virtual QStringList getConditionList() const override;
    virtual bool hasConditionId(const QString& conditionId) const override;

    virtual int getSpeed() const override;
    virtual int getArmorClass() const override;
    virtual int getHitPoints() const override;
    virtual void setHitPoints(int hitPoints) override;
    virtual QString getName() const override;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const override;

    Characterv2* getCharacter() const;
    void setCharacter(Characterv2* character);

    int getIconIndex() const;
    void setIconIndex(int index);
    void setIconFile(const QString& iconFile);

signals:
    void imageChanged(BattleDialogModelCharacter* character);

public slots:
    virtual void setConditionList(const QStringList& conditions) override;
    virtual void addConditionId(const QString& conditionId) override;
    virtual void removeConditionId(const QString& conditionId) override;
    virtual void clearConditions() override;

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

private:
    int _iconIndex;
    QString _iconFile;
    ScaledPixmap* _iconPixmap;
};
#endif // BATTLEDIALOGMODELCHARACTER_H
