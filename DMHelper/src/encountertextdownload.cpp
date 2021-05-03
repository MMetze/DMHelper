#include "encountertextdownload.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QDebug>

EncounterTextDownload::EncounterTextDownload(const QString& cacheDirectory, QObject *parent) :
    EncounterText(QString(), parent),
    _textMD5(),
    _textUuid(),
    _translatedMD5(),
    _translatedUuid(),
    _cacheDirectory(cacheDirectory)
{
}

void EncounterTextDownload::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _textMD5.clear();
    _textUuid.clear();
    _translatedMD5.clear();
    _translatedUuid.clear();

    setID(QUuid(element.attribute(QString("_baseID"))));

    setTextWidth(element.attribute("textWidth", "90").toInt());
    int scrollSpeed = element.attribute("scrollSpeed").toInt();
    setScrollSpeed(scrollSpeed > 0 ? scrollSpeed : 25);
    setAnimated(static_cast<bool>(element.attribute("animated", QString::number(0)).toInt()));
    setTranslated(static_cast<bool>(element.attribute("translated", QString::number(0)).toInt()));

    QString textString = element.attribute("text");
    if(!textString.isEmpty())
    {
        QStringList textSplit = textString.split(",");
        if(textSplit.count() == 2)
        {
            _textMD5 = textSplit.at(0);
            _textUuid = textSplit.at(1);
            emit requestFile(_textMD5, _textUuid, DMHelper::FileType_Text);
        }
    }

    QString translatedString = element.attribute("translated-text");
    if(!translatedString.isEmpty())
    {
        QStringList translatedSplit = textString.split(",");
        if(translatedSplit.count() == 2)
        {
            _translatedMD5 = translatedSplit.at(0);
            _translatedUuid = translatedSplit.at(1);
            emit requestFile(_translatedMD5, _translatedUuid, DMHelper::FileType_Text);
        }
    }
}

bool EncounterTextDownload::isComplete()
{
    return(((_textMD5.isEmpty()) || (!_text.isEmpty())) &&
           ((_translatedMD5.isEmpty()) || (!_translatedText.isEmpty())));
}

void EncounterTextDownload::fileReceived(const QString& md5, const QString& uuid, const QByteArray& data)
{
    Q_UNUSED(uuid);

    if((!md5.isEmpty()) && (!data.isEmpty()))
    {
        if((md5 == _textMD5) && (uuid == _textUuid))
        {
            QByteArray dataFromPercent = QByteArray::fromPercentEncoding(data);
            QString dataString(dataFromPercent);
            // TODO: This is probably necessary with backend!! dataString.fromUtf8(dataFromPercent);
            setText(dataString);
        }

        if((md5 == _translatedMD5) && (uuid == _translatedUuid))
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
