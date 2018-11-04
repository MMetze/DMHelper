#ifndef BATTLEDIALOGMODELEFFECTFACTORY_H
#define BATTLEDIALOGMODELEFFECTFACTORY_H

#include <QObject>

class BattleDialogModelEffect;
class QDomElement;

class BattleDialogModelEffectFactory : public QObject
{
    Q_OBJECT
public:
    explicit BattleDialogModelEffectFactory(QObject *parent = 0);

    static BattleDialogModelEffect* createEffect(int effectType);
    static BattleDialogModelEffect* createEffect(const QDomElement& element);

signals:

public slots:
};

#endif // BATTLEDIALOGMODELEFFECTFACTORY_H
