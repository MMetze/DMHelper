#include "audiofactory.h"
#include "audiotrackfile.h"
#include "audiotracksyrinscape.h"
#include "audiotrackyoutube.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QDebug>

AudioFactory::AudioFactory(QObject *parent) :
    ObjectFactory(parent)
{
}

CampaignObjectBase* AudioFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    Q_UNUSED(isImport);

    if(objectType != DMHelper::CampaignType_AudioTrack)
        return nullptr;

    switch(subType)
    {
        case DMHelper::AudioType_File:
            return new AudioTrackFile(objectName);
        case DMHelper::AudioType_Syrinscape:
            return new AudioTrackSyrinscape(objectName);
        case DMHelper::AudioType_Youtube:
            return new AudioTrackYoutube(objectName);
        default:
            return nullptr;
    }
}

CampaignObjectBase* AudioFactory::createObject(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.tagName() != QString("track"))
        return nullptr;

    bool ok = false;
    int audioType = element.attribute("type").toInt(&ok);

    if(!ok)
    {
        QDomElement urlElement = element.firstChildElement(QString("url"));
        QDomCDATASection urlData = urlElement.firstChild().toCDATASection();
        QUrl url(urlData.data());
        if(url.isEmpty())
        {
            qDebug() << "[AudioFactory] ERROR: unexpected audio type found reading element: " << audioType << ", read success: " << ok;
            return nullptr;
        }
        audioType = identifyAudioSubtype(url);
    }

    switch(audioType)
    {
        case DMHelper::AudioType_File:
            return new AudioTrackFile();
        case DMHelper::AudioType_Syrinscape:
            return new AudioTrackSyrinscape();
        case DMHelper::AudioType_Youtube:
            return new AudioTrackYoutube();
        default:
            return nullptr;
    }
}

AudioTrack* AudioFactory::createTrackFromUrl(const QUrl& url, const QString& objectName)
{
    CampaignObjectBase* newObject = createObject(DMHelper::CampaignType_AudioTrack,
                                                 identifyAudioSubtype(url),
                                                 objectName,
                                                 false);

    AudioTrackUrl* urlTrack = dynamic_cast<AudioTrackUrl*>(newObject);
    if(urlTrack)
        urlTrack->setUrl(url);

    return dynamic_cast<AudioTrack*>(newObject);
}

int AudioFactory::identifyAudioSubtype(const QUrl& url)
{
    QString urlScheme = url.scheme();
    if((!urlScheme.isEmpty()) && (urlScheme.left(10) == QString("syrinscape")))
        return DMHelper::AudioType_Syrinscape;
    else if(url.host().contains(QString("youtube")))
        return DMHelper::AudioType_Youtube;
    else
        return DMHelper::AudioType_File;
}
