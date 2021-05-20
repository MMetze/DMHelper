#ifndef BATTLEDIALOGMODELCOMBATANTDOWNLOAD_H
#define BATTLEDIALOGMODELCOMBATANTDOWNLOAD_H

#include "battledialogmodelcombatant.h"
#include "scaledpixmap.h"
#include <QUuid>

class BattleDialogModelCombatantDownload : public BattleDialogModelCombatant
{
    Q_OBJECT
public:
    BattleDialogModelCombatantDownload(QObject *parent = nullptr);
    virtual ~BattleDialogModelCombatantDownload() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QString getMD5() const override;
    virtual void setMD5(const QString& md5) override;

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
    virtual int getConditions() const override;
    virtual bool hasCondition(Combatant::Condition condition) const override;
    virtual int getSpeed() const override;
    virtual int getArmorClass() const override;
    virtual int getHitPoints() const override;
    virtual void setHitPoints(int hitPoints) override;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const override;

    QString getUuid() const;
    void setUuid(const QString& uuid);

    bool isTokenValid();

public slots:
    virtual void fileReceived(const QString& md5, const QString& uuid, const QByteArray& data);

    virtual void setConditions(int conditions) override;
    virtual void applyConditions(int conditions) override;
    virtual void removeConditions(int conditions) override;

signals:
    void requestFile(const QString& md5, const QString& uuid, int fileType);

protected:
    void interpretToken();

    QUuid _combatantId;
    qreal _sizeFactor;
    int _conditions;
    int _speed;
    QString _token;
    QString _md5;
    QString _uuid;
    ScaledPixmap* _tokenPixmap;
};

#endif // BATTLEDIALOGMODELCOMBATANTDOWNLOAD_H
