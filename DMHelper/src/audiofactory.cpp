#include "audiofactory.h"
#include "audiotrackfile.h"
#include "audiotracksyrinscape.h"
#include "audiotracksyrinscapeonline.h"
#include "audiotrackyoutube.h"
#include "ruleset.h"
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

    AudioTrack* track = nullptr;

    switch(subType)
    {
        case DMHelper::AudioType_File:
            track = new AudioTrackFile(objectName); break;
        case DMHelper::AudioType_Syrinscape:
            track = new AudioTrackSyrinscape(objectName); break;
        case DMHelper::AudioType_SyrinscapeOnline:
            track = new AudioTrackSyrinscapeOnline(objectName); break;
        case DMHelper::AudioType_Youtube:
            track = new AudioTrackYoutube(objectName); break;
        default:
            return nullptr;
    }

    emit trackCreated(track);
    return track;
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

    AudioTrack* track = nullptr;

    switch(audioType)
    {
        case DMHelper::AudioType_File:
            track = new AudioTrackFile(); break;
        case DMHelper::AudioType_Syrinscape:
            track = new AudioTrackSyrinscape(); break;
        case DMHelper::AudioType_SyrinscapeOnline:
            track = new AudioTrackSyrinscapeOnline(); break;
        case DMHelper::AudioType_Youtube:
            track = new AudioTrackYoutube(); break;
        default:
            return nullptr;
    }

    emit trackCreated(track);
    return track;
}

void AudioFactory::configureFactory(const Ruleset& ruleset, int inputMajorVersion, int inputMinorVersion)
{
    Q_UNUSED(ruleset);
    Q_UNUSED(inputMajorVersion);
    Q_UNUSED(inputMinorVersion);
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

    AudioTrack* track = dynamic_cast<AudioTrack*>(newObject);

    if(track)
        emit trackCreated(track);

    return track;
}

int AudioFactory::identifyAudioSubtype(const QUrl& url)
{
    QString urlScheme = url.scheme();
    if((!urlScheme.isEmpty()) && (urlScheme.left(10) == QString("syrinscape")))
        return DMHelper::AudioType_Syrinscape;
    else if (url.path().contains(QString("online/frontend-api")))
        return DMHelper::AudioType_SyrinscapeOnline;
    else if(url.host().contains(QString("youtube")))
        return DMHelper::AudioType_Youtube;
    else
        return DMHelper::AudioType_File;
}
