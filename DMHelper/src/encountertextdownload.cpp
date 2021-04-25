#include "encountertextdownload.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QDebug>

EncounterTextDownload::EncounterTextDownload(const QString& cacheDirectory, QObject *parent) :
    EncounterText(QString(), parent),
    _md5Text(),
    _md5TranslatedText(),
    _cacheDirectory(cacheDirectory)
{
}

void EncounterTextDownload::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    setID(QUuid(element.attribute(QString("_baseID"))));

    setTextWidth(element.attribute("textWidth", "90").toInt());
    int scrollSpeed = element.attribute("scrollSpeed").toInt();
    setScrollSpeed(scrollSpeed > 0 ? scrollSpeed : 25);
    setAnimated(static_cast<bool>(element.attribute("animated", QString::number(0)).toInt()));
    setTranslated(static_cast<bool>(element.attribute("translated", QString::number(0)).toInt()));

    _md5Text = element.attribute("text");
    _md5TranslatedText = element.attribute("translated-text");

    // TODO: UUID
    if(!_md5TranslatedText.isEmpty())
        emit requestFile(_md5TranslatedText, QString(), DMHelper::FileType_Text);

    if(!_md5Text.isEmpty())
        emit requestFile(_md5Text, QString(), DMHelper::FileType_Text);
}

bool EncounterTextDownload::isComplete()
{
    return(((_md5Text.isEmpty()) || (!_text.isEmpty())) &&
           ((_md5TranslatedText.isEmpty()) || (!_translatedText.isEmpty())));
}

void EncounterTextDownload::fileReceived(const QString& md5, const QString& uuid, const QByteArray& data)
{
    Q_UNUSED(uuid);

    if((!md5.isEmpty()) && (!data.isEmpty()))
    {
        if(md5 == _md5Text)
        {
            QByteArray dataFromPercent = QByteArray::fromPercentEncoding(data);
            QString dataString(dataFromPercent);
            // TODO: This is probably necessary with backend!! dataString.fromUtf8(dataFromPercent);
            setText(dataString);
        }

        if(md5 == _md5TranslatedText)
        {
            QByteArray dataFromPercent = QByteArray::fromPercentEncoding(data);
            QString dataString;
            dataString.fromUtf8(dataFromPercent);
            setTranslatedText(dataString);
        }
    }

    if(isComplete())
        emit encounterComplete();
}
