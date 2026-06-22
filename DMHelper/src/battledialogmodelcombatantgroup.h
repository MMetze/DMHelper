#ifndef BATTLEDIALOGMODELCOMBATANTGROUP_H
#define BATTLEDIALOGMODELCOMBATANTGROUP_H

#include <QObject>
#include <QUuid>
#include <QString>
#include <QDomElement>
#include <QDir>

class BattleDialogModelCombatantGroup : public QObject
{
    Q_OBJECT

public:
    explicit BattleDialogModelCombatantGroup(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelCombatantGroup(const QUuid& id, const QString& name, int initiative, bool collapsed, QObject *parent = nullptr);
    virtual ~BattleDialogModelCombatantGroup() override;

    QUuid getId() const;

    QString getName() const;
    void setName(const QString& name);

    int getInitiative() const;
    void setInitiative(int initiative);

    bool isCollapsed() const;
    void setCollapsed(bool collapsed);

    QDomElement createOutputXML(QDomDocument &doc);
    void inputXML(const QDomElement &element);

signals:
    void dirty();
    void groupChanged();

private:
    QUuid _id;
    QString _name;
    int _initiative;
    bool _collapsed;
};

#endif // BATTLEDIALOGMODELCOMBATANTGROUP_H
