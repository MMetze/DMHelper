#include "battledialogmodeleffectfactory.h"
#include "battledialogmodeleffect.h"
#include "battledialogmodeleffectcone.h"
#include "battledialogmodeleffectcube.h"
#include "battledialogmodeleffectradius.h"
#include "battledialogmodeleffectline.h"
#include "battledialogmodeleffectobject.h"
#include "battledialogmodeleffectobjectvideo.h"
#include "battledialogmodeleffectsmoke.h"
#include "battledialogmodeleffectfire.h"
#include "battledialogmodeleffectsparks.h"
#include "battledialogmodeleffectlight.h"
#include <QDomElement>
#include <QPixmap>
#include <QDebug>

//#define DEBUG_EFFECT_FACTORY

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
        case BattleDialogModelEffect::BattleDialogModelEffect_Object:
            result = new BattleDialogModelEffectObject();
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_ObjectVideo:
            result = new BattleDialogModelEffectObjectVideo();
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Smoke:
            result = new BattleDialogModelEffectSmoke();
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Fire:
            result = new BattleDialogModelEffectFire();
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Sparks:
            result = new BattleDialogModelEffectSparks();
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Light:
            result = new BattleDialogModelEffectLight();
            break;
        default:
            break;
    }

#ifdef DEBUG_EFFECT_FACTORY
    if(result)
        qDebug() << "[Battle Dialog Model Effect Factory] created model effect " << result << " of type " << result->getEffectType();
#endif

    return result;
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffect(const QDomElement& element, bool isImport)
{
    int effectType = element.attribute("type", QString::number(BattleDialogModelEffect::BattleDialogModelEffect_Base)).toInt();
    BattleDialogModelEffect* result = createEffect(effectType);

    if(result)
    {
        result->inputXML(element, isImport);
#ifdef DEBUG_EFFECT_FACTORY
        qDebug() << "[Battle Dialog Model Effect Factory] read model effect " << result << " of type " << result->getEffectType();
#endif
    }

    return result;
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectObjectVideo(const QPointF& position, const QSize& size, const QColor& color, const QString& videoFile)
{
    if(videoFile.isEmpty())
    {
        qDebug() << "[BattleDialogModelEffectFactory] ERROR: unable to create object effect, empty video file";
        return nullptr;
    }

    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_ObjectVideo);
    if(!effect)
        return nullptr;

    effect->setSize(size.height());
    effect->setWidth(size.width());
    effect->setColor(color);
    effect->setPosition(position);
    effect->setImageFile(videoFile);

    return effect;
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectObject(const QPointF& position, const QSize& size, const QColor& color, const QString& imageFile)
{
    if(imageFile.isEmpty())
    {
        qDebug() << "[BattleDialogModelEffectFactory] ERROR: unable to create object effect, empty image file";
        return nullptr;
    }

    QPixmap objectPixmap(imageFile);
    if(objectPixmap.isNull())
    {
        qDebug() << "[BattleDialogModelEffectFactory] ERROR: unable to create object effect, invalid image file selected: " << imageFile;
        return nullptr;
    }

    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(BattleDialogModelEffect::BattleDialogModelEffect_Object);
    if(!effect)
        return nullptr;

    effect->setSize(size.height());
    effect->setWidth(size.width());
    effect->setColor(color);
    effect->setPosition(position);
    effect->setImageFile(imageFile);

    return effect;
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectRadius(const QPointF& position, int radius, const QColor& color)
{
    return createEffectShape(BattleDialogModelEffect::BattleDialogModelEffect_Radius,
                             position,
                             radius,
                             color);
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectCone(const QPointF& position, int size, const QColor& color)
{
    return createEffectShape(BattleDialogModelEffect::BattleDialogModelEffect_Cone,
                             position,
                             size,
                             color);
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectCube(const QPointF& position, int size, const QColor& color)
{
    return createEffectShape(BattleDialogModelEffect::BattleDialogModelEffect_Cube,
                             position,
                             size,
                             color);
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectLine(const QPointF& position, int length, int width, const QColor& color)
{
    BattleDialogModelEffect* effect = createEffectShape(BattleDialogModelEffect::BattleDialogModelEffect_Line,
                                                        position,
                                                        length,
                                                        color);
    if(effect)
        effect->setWidth(width);

    return effect;
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectShape(int effectType, const QPointF& position, int size, const QColor& color)
{
    if((effectType != BattleDialogModelEffect::BattleDialogModelEffect_Radius) &&
       (effectType != BattleDialogModelEffect::BattleDialogModelEffect_Cone) &&
       (effectType != BattleDialogModelEffect::BattleDialogModelEffect_Cube) &&
       (effectType != BattleDialogModelEffect::BattleDialogModelEffect_Line) &&
       (effectType != BattleDialogModelEffect::BattleDialogModelEffect_Smoke) &&
       (effectType != BattleDialogModelEffect::BattleDialogModelEffect_Fire) &&
       (effectType != BattleDialogModelEffect::BattleDialogModelEffect_Sparks) &&
       (effectType != BattleDialogModelEffect::BattleDialogModelEffect_Light))
        return nullptr;

    BattleDialogModelEffect* effect = BattleDialogModelEffectFactory::createEffect(effectType);
    if(effect)
    {
        effect->setSize(size);
        effect->setPosition(position);
        effect->setColor(color);
    }

    return effect;
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectSmoke(const QPointF& position, int size, const QColor& color)
{
    return createEffectShape(BattleDialogModelEffect::BattleDialogModelEffect_Smoke, position, size, color);
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectFire(const QPointF& position, int size, const QColor& color)
{
    return createEffectShape(BattleDialogModelEffect::BattleDialogModelEffect_Fire, position, size, color);
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectSparks(const QPointF& position, int size, const QColor& color)
{
    return createEffectShape(BattleDialogModelEffect::BattleDialogModelEffect_Sparks, position, size, color);
}

BattleDialogModelEffect* BattleDialogModelEffectFactory::createEffectLight(const QPointF& position, int size, const QColor& color)
{
    return createEffectShape(BattleDialogModelEffect::BattleDialogModelEffect_Light, position, size, color);
}
