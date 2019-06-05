#include "audiotrack.h"
#include <QDomDocument>
#include <QDomElement>

AudioTrack::AudioTrack(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    CampaignObjectBase(parent),
    _name(trackName),
    _url(trackUrl),
    _md5()
{
}

AudioTrack::AudioTrack(QDomElement &element, bool isImport, QObject *parent) :
    CampaignObjectBase(parent),
    _name(),
    _url(),
    _md5()
{
    inputXML(element, isImport);
}

AudioTrack::AudioTrack(const AudioTrack &obj) :
    CampaignObjectBase(obj),
    _name(obj._name),
    _url(obj._url),
    _md5(obj._md5)
{
}

AudioTrack::~AudioTrack()
{
}

void AudioTrack::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    QDomElement element = doc.createElement( "track" );

    CampaignObjectBase::outputXML(doc, element, targetDirectory, isExport);

    element.setAttribute( "name", getName() );
    element.setAttribute( "md5", getMD5() );

    QDomElement urlElement = doc.createElement( "url" );
    QDomCDATASection urlData = doc.createCDATASection(getUrl().toString());
    urlElement.appendChild(urlData);
    element.appendChild(urlElement);

    parent.appendChild(element);
}

void AudioTrack::inputXML(const QDomElement &element, bool isImport)
{
    CampaignObjectBase::inputXML(element, isImport);

    setName(element.attribute("name"));
    setMD5(element.attribute("md5"));

    QDomElement urlElement = element.firstChildElement( QString("url") );
    QDomCDATASection urlData = urlElement.firstChild().toCDATASection();
    setUrl(QUrl(urlData.data()));
}

void AudioTrack::postProcessXML(const QDomElement &element, bool isImport)
{
    CampaignObjectBase::postProcessXML(element, isImport);
}

QString AudioTrack::getName() const
{
    return _name;
}

void AudioTrack::setName(const QString& trackName)
{
    if(_name != trackName)
    {
        _name = trackName;
        emit changed();
        emit dirty();
    }
}

QUrl AudioTrack::getUrl() const
{
    return _url;
}

void AudioTrack::setUrl(const QUrl& trackUrl)
{
    if(_url != trackUrl)
    {
        _url = trackUrl;
        emit changed();
        emit dirty();
    }
}

QString AudioTrack::getMD5() const
{
    return _md5;
}

void AudioTrack::setMD5(const QString& md5)
{
    _md5 = md5;
}
