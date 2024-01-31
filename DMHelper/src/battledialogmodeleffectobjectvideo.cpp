#include "battledialogmodeleffectobjectvideo.h"
#include "layertokens.h"

BattleDialogModelEffectObjectVideo::BattleDialogModelEffectObjectVideo(const QString& name, QObject *parent) :
    BattleDialogModelEffectObject{name, parent}
{}

BattleDialogModelEffectObjectVideo::BattleDialogModelEffectObjectVideo(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip) :
    BattleDialogModelEffectObject{size, width, position, rotation, imageFile, tip}
{}

BattleDialogModelEffectObjectVideo::~BattleDialogModelEffectObjectVideo()
{}

void BattleDialogModelEffectObjectVideo::inputXML(const QDomElement &element, bool isImport)
{
    BattleDialogModelEffectObject::inputXML(element, isImport);
}

void BattleDialogModelEffectObjectVideo::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelEffectObjectVideo* otherEffect = dynamic_cast<const BattleDialogModelEffectObjectVideo*>(other);
    if(!otherEffect)
        return;

    BattleDialogModelEffectObject::copyValues(other);
}

BattleDialogModelEffect* BattleDialogModelEffectObjectVideo::clone() const
{
    BattleDialogModelEffectObjectVideo* newEffect = new BattleDialogModelEffectObjectVideo{getName()};
    newEffect->copyValues(this);
    return newEffect;
}

void BattleDialogModelEffectObjectVideo::setLayer(LayerTokens* tokensLayer)
{
    if(tokensLayer)
        connect(this, &BattleDialogModelEffectObjectVideo::effectReady, tokensLayer, &LayerTokens::effectReady);
    else if(_tokensLayer)
        disconnect(this, &BattleDialogModelEffectObjectVideo::effectReady, _tokensLayer, &LayerTokens::effectReady);

    BattleDialogModelEffectObject::setLayer(tokensLayer);
}

int BattleDialogModelEffectObjectVideo::getEffectType() const
{
    return BattleDialogModelEffect_ObjectVideo;
}

QGraphicsItem* BattleDialogModelEffectObjectVideo::createEffectShape(qreal gridScale)
{
    return nullptr;
}

void BattleDialogModelEffectObjectVideo::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    BattleDialogModelEffectObject::internalOutputXML(doc, element, targetDirectory, isExport);
}
