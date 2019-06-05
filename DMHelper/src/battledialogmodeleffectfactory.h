#ifndef BATTLEDIALOGMODELEFFECTFACTORY_H
#define BATTLEDIALOGMODELEFFECTFACTORY_H

#include <QObject>

class BattleDialogModelEffect;
class QDomElement;

class BattleDialogModelEffectFactory : public QObject
{
    Q_OBJECT
public:
    explicit BattleDialogModelEffectFactory(QObject *parent = nullptr);

    static BattleDialogModelEffect* createEffect(int effectType);
    static BattleDialogModelEffect* createEffect(const QDomElement& element, bool isImport);

signals:

public slots:
};

#endif // BATTLEDIALOGMODELEFFECTFACTORY_H
