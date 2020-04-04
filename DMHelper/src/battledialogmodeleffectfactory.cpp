#include "battledialogmodeleffectfactory.h"
#include "battledialogmodeleffect.h"
#include "battledialogmodeleffectcone.h"
#include "battledialogmodeleffectcube.h"
#include "battledialogmodeleffectradius.h"
#include "battledialogmodeleffectline.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelEffectFactory::BattleDialogModelEffectFactory(QObject *parent) :
    QObject(parent)
{
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffect(int effectType)
{
    BattleDialogModelEffect* result = nullptr;

    switch(effectType)
    {
        case BattleDialogModelEffect::BattleDialogModelEffect_Radius:
            result = new BattleDialogModelEffectRadius();
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Cone:
            result = new BattleDialogModelEffectCone();
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Cube:
            result = new BattleDialogModelEffectCube();
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Line:
            result = new BattleDialogModelEffectLine();
            break;
        default:
            break;
    }

    if(result)
        qDebug() << "[Battle Dialog Model Effect Factor] created model effect " << result << " of type " << result->getEffectType();

    return result;
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffect(const QDomElement& element, bool isImport)
{
    int effectType = element.attribute("type",QString::number(BattleDialogModelEffect::BattleDialogModelEffect_Base)).toInt();
    BattleDialogModelEffect* result = createEffect(effectType);

    if(result)
    {
        result->inputXML(element, isImport);
        qDebug() << "[Battle Dialog Model Effect Factor] read model effect " << result << " of type " << result->getEffectType();
    }

    return result;
}

