#include "battledialogmodeleffectobjectvideo.h"
#include "layertokens.h"
#include "videoplayerscreenshot.h"
#include "battledialogeffectsettingsobjectvideo.h"

BattleDialogModelEffectObjectVideo::BattleDialogModelEffectObjectVideo(const QString& name, QObject *parent) :
    BattleDialogModelEffectObject{name, parent},
    _playAudio(true),
    _effectTransparencyType(DMHelper::TransparentType_None),
    _transparentColor(),
    _transparentTolerance(0.15),
    _colorize(false),
    _colorizeColor(),
    _screenshot{nullptr},
    _pixmap{}
{}

BattleDialogModelEffectObjectVideo::BattleDialogModelEffectObjectVideo(int size, int width, const QPointF& position, qreal rotation, const QString& videoFile, const QString& tip) :
    BattleDialogModelEffectObject{size, width, position, rotation, videoFile, tip},
    _playAudio(true),
    _effectTransparencyType(DMHelper::TransparentType_None),
    _transparentColor(),
    _transparentTolerance(0.15),
    _colorize(false),
    _colorizeColor(),
    _screenshot{nullptr},
    _pixmap{}
{}

BattleDialogModelEffectObjectVideo::~BattleDialogModelEffectObjectVideo()
{}

void BattleDialogModelEffectObjectVideo::inputXML(const QDomElement &element, bool isImport)
{
    if(element.hasAttribute("playAudio"))
        _playAudio = static_cast<bool>(element.attribute("playAudio").toInt());

    if(element.hasAttribute("effect"))
        _effectTransparencyType = static_cast<DMHelper::TransparentType>(element.attribute("effect").toInt());

    if(element.hasAttribute("transparentColor"))
        _transparentColor = QColor(element.attribute("transparentColor"));

    if(element.hasAttribute("transparentTolerance"))
        _transparentTolerance = element.attribute("transparentTolerance").toDouble();

    if(element.hasAttribute("colorize"))
        _colorize = static_cast<bool>(element.attribute("colorize").toInt());

    if(element.hasAttribute("colorizeColor"))
        _colorizeColor = QColor(element.attribute("colorizeColor"));

    BattleDialogModelEffectObject::inputXML(element, isImport);
}

void BattleDialogModelEffectObjectVideo::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelEffectObjectVideo* otherEffect = dynamic_cast<const BattleDialogModelEffectObjectVideo*>(other);
    if(!otherEffect)
        return;

    _playAudio = otherEffect->_playAudio;
    _effectTransparencyType = otherEffect->_effectTransparencyType;
    _transparentColor = otherEffect->_transparentColor;
    _transparentTolerance = otherEffect->_transparentTolerance;
    _colorize = otherEffect->_colorize;
    _colorizeColor = otherEffect->_colorizeColor;

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

BattleDialogEffectSettingsBase* BattleDialogModelEffectObjectVideo::getEffectEditor() const
{
    BattleDialogEffectSettingsObjectVideo* result = new BattleDialogEffectSettingsObjectVideo(*this);
    return result;
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

    TokenEditor* editor = new TokenEditor();

    editor->setSourceImage(_pixmap.toImage());
    editor->setTransparentValue(_effectTransparencyType);
    editor->setTransparentColor(_transparentColor);
    editor->setTransparentLevel(static_cast<int>(_transparentTolerance * 100.0));
    editor->setColorize(_colorize);
    editor->setColorizeColor(_colorizeColor);
    editor->setSquareFinalImage(false);

    QGraphicsItem* pixmapShape = createPixmapShape(gridScale, QPixmap::fromImage(editor->getFinalImage()));

    editor->deleteLater();

    return pixmapShape;
}

bool BattleDialogModelEffectObjectVideo::isPlayAudio() const
{
    return _playAudio;
}

DMHelper::TransparentType BattleDialogModelEffectObjectVideo::getEffectTransparencyType() const
{
    return _effectTransparencyType;
}

QColor BattleDialogModelEffectObjectVideo::getTransparentColor() const
{
    return _transparentColor;
}

qreal BattleDialogModelEffectObjectVideo::getTransparentTolerance() const
{
    return _transparentTolerance;
}

bool BattleDialogModelEffectObjectVideo::isColorize() const
{
    return _colorize;
}

QColor BattleDialogModelEffectObjectVideo::getColorizeColor() const
{
    return _colorizeColor;
}

QPixmap BattleDialogModelEffectObjectVideo::getPixmap() const
{
    return _pixmap;
}

void BattleDialogModelEffectObjectVideo::setPlayAudio(bool playAudio)
{
    if(_playAudio == playAudio)
        return;

    _playAudio = playAudio;
    emit dirty();
}

void BattleDialogModelEffectObjectVideo::setEffectTransparencyType(DMHelper::TransparentType effectType)
{
    if(_effectTransparencyType == effectType)
        return;

    _effectTransparencyType = effectType;
    emit dirty();
}

void BattleDialogModelEffectObjectVideo::setTransparentColor(const QColor& transparentColor)
{
    if(_transparentColor == transparentColor)
        return;

    _transparentColor = transparentColor;
    emit dirty();
}

void BattleDialogModelEffectObjectVideo::setTransparentTolerance(qreal transparentTolerance)
{
    if(_transparentTolerance == transparentTolerance)
        return;

    _transparentTolerance = transparentTolerance;
    emit dirty();
}

void BattleDialogModelEffectObjectVideo::setColorize(bool colorize)
{
    if(_colorize == colorize)
        return;

    _colorize = colorize;
    emit dirty();
}

void BattleDialogModelEffectObjectVideo::setColorizeColor(const QColor& colorizeColor)
{
    if(_colorizeColor == colorizeColor)
        return;

    _colorizeColor = colorizeColor;
    emit dirty();
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
    if(!_playAudio)
        element.setAttribute("playAudio", 0);

    if(_effectTransparencyType != DMHelper::TransparentType_None)
        element.setAttribute("effect", static_cast<int>(_effectTransparencyType));

    if((_effectTransparencyType == DMHelper::TransparentType_TransparentColor) && (_transparentColor.isValid()) && (_transparentColor != Qt::black))
        element.setAttribute("transparentColor", _transparentColor.name());

    if((_effectTransparencyType == DMHelper::TransparentType_TransparentColor) && (_transparentTolerance != 0.15))
        element.setAttribute("transparentTolerance", _transparentTolerance);

    if(_colorize)
        element.setAttribute("colorize", 1);

    if((_colorize) && (_colorizeColor.isValid()) && (_colorizeColor != Qt::black))
        element.setAttribute("colorizeColor", _colorizeColor.name());

    BattleDialogModelEffectObject::internalOutputXML(doc, element, targetDirectory, isExport);
}
