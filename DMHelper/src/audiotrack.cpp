#include "audiotrack.h"
#include "dmconstants.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDir>

AudioTrack::AudioTrack(const QString& trackName, QObject *parent) :
    CampaignObjectBase(trackName, parent),
    _md5()
{
}

AudioTrack::~AudioTrack()
{
}

void AudioTrack::inputXML(const QDomElement &element, bool isImport)
{
    setMD5(element.attribute("md5"));

    QDomElement urlElement = element.firstChildElement(QString("url"));
    QDomCDATASection urlData = urlElement.firstChild().toCDATASection();
    setUrl(QUrl(urlData.data()));

    CampaignObjectBase::inputXML(element, isImport);
}

void AudioTrack::copyValues(const CampaignObjectBase* other)
{
    const AudioTrack* otherAudioTrack = dynamic_cast<const AudioTrack*>(other);
    if(!otherAudioTrack)
        return;

    _md5 = otherAudioTrack->_md5;

    CampaignObjectBase::copyValues(other);
}

int AudioTrack::getObjectType() const
{
    return DMHelper::CampaignType_AudioTrack;
}

QString AudioTrack::getMD5() const
{
    return _md5;
}

void AudioTrack::setMD5(const QString& md5)
{
    _md5 = md5;
}

bool AudioTrack::isPlaying() const
{
    return false;
}

bool AudioTrack::isRepeat() const
{
    return false;
}

bool AudioTrack::isMuted() const
{
    return false;
}

int AudioTrack::getVolume() const
{
    return 0;
}

QDomElement AudioTrack::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("track");
}

void AudioTrack::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("type", getAudioType());
    element.setAttribute("md5", getMD5());

    QString urlString = targetDirectory.relativeFilePath(getUrl().toString());
    if(urlString.isEmpty())
        urlString = getUrl().toString();

    QDomElement urlElement = doc.createElement("url");
    QDomCDATASection urlData = doc.createCDATASection(urlString);
    urlElement.appendChild(urlData);
    element.appendChild(urlElement);

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool AudioTrack::belongsToObject(QDomElement& element)
{
    if(element.tagName() == QString("url"))
        return true;
    else
        return CampaignObjectBase::belongsToObject(element);
}
