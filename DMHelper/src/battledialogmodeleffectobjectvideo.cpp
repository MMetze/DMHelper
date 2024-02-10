#include "battledialogmodeleffectobjectvideo.h"
#include "layertokens.h"
#include "videoplayerscreenshot.h"

BattleDialogModelEffectObjectVideo::BattleDialogModelEffectObjectVideo(const QString& name, QObject *parent) :
    BattleDialogModelEffectObject{name, parent},
    _screenshot{nullptr},
    _pixmap{}
{}

BattleDialogModelEffectObjectVideo::BattleDialogModelEffectObjectVideo(int size, int width, const QPointF& position, qreal rotation, const QString& videoFile, const QString& tip) :
    BattleDialogModelEffectObject{size, width, position, rotation, videoFile, tip},
    _screenshot{nullptr},
    _pixmap{}
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
    if(!_screenshot)
    {
        _screenshot = new VideoPlayerScreenshot{getImageFile()};
        connect(_screenshot, &VideoPlayerScreenshot::screenshotReady, this, &BattleDialogModelEffectObjectVideo::onScreenshotReady);
        _screenshot->retrieveScreenshot();
        return nullptr;
    }

    if(_pixmap.isNull())
        return nullptr;
    else
        return createPixmapShape(gridScale, _pixmap);
}

DMHelper::TransparentType BattleDialogModelEffectObjectVideo::getEffectTransparencyType() const
{
    return DMHelper::TransparentType_None;
}

QColor BattleDialogModelEffectObjectVideo::getTransparentColor() const
{
    return Qt::black;
}

qreal BattleDialogModelEffectObjectVideo::getTransparentTolerance() const
{
    return 0;
}

bool BattleDialogModelEffectObjectVideo::isColorize() const
{
    return false;
}

QColor BattleDialogModelEffectObjectVideo::getColorizeColor() const
{
    return Qt::black;
}

void BattleDialogModelEffectObjectVideo::onScreenshotReady(const QImage& image)
{
    if(image.isNull())
        return;

    _pixmap = QPixmap::fromImage(image);
    emit effectReady(this);
}

void BattleDialogModelEffectObjectVideo::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    BattleDialogModelEffectObject::internalOutputXML(doc, element, targetDirectory, isExport);
}
