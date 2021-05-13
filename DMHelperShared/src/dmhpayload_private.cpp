#include "dmhpayload_private.h"
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDebug>

DMHPayload_Private::DMHPayload_Private() :
    _imageFile(),
    _audioFile(),
    _data()
{
}

DMHPayload_Private::DMHPayload_Private(const QString& data)
{
    fromString(data);
}

QString DMHPayload_Private::getImageFile() const
{
    return _imageFile;
}

void DMHPayload_Private::setImageFile(const QString& imageFile)
{
    _imageFile = imageFile;
}

QString DMHPayload_Private::getAudioFile() const
{
    return _audioFile;
}

void DMHPayload_Private::setAudioFile(const QString& audioFile)
{
    _audioFile = audioFile;
}

QString DMHPayload_Private::getData() const
{
    return _data;
}

void DMHPayload_Private::setData(const QString& data)
{
    _data = data;
}

void DMHPayload_Private::fromString(const QString& data)
{
    QRegularExpression reImg("<image>(.*)<\\/image>");
    QRegularExpressionMatch matchImg = reImg.match(data);
    //setImageFile(matchImg.hasMatch() ? matchImg.captured(matchImg.lastCapturedIndex()).remove(QChar('-')) : QString());
    setImageFile(matchImg.hasMatch() ? matchImg.captured(matchImg.lastCapturedIndex()) : QString());

    QRegularExpression reAudio("<audio>(.*)<\\/audio>");
    QRegularExpressionMatch matchAudio = reAudio.match(data);
    setAudioFile(matchAudio.hasMatch() ? matchAudio.captured(matchAudio.lastCapturedIndex()) : QString());
    //setAudioFile(matchAudio.hasMatch() ? matchAudio.captured(matchAudio.lastCapturedIndex()).remove(QChar('-')) : QString());

    QRegularExpression reData("<data>(.*)<\\/data>");
    QRegularExpressionMatch matchData = reData.match(data);
    setData(matchData.hasMatch() ? matchData.captured(matchData.lastCapturedIndex()) : QString());

    //qDebug() << "[DMHPayload_Private] From string: " << data << " ==> data: " << _data;
}

QString DMHPayload_Private::toString() const
{
    QString result;

    result += QString("<image>") + _imageFile + QString("</image>");
    result += QString("<audio>") + _audioFile + QString("</audio>");
    result += QString("<data>") + _data + QString("</data>");

    return result;
}

void DMHPayload_Private::clear()
{
    _imageFile.clear();
    _audioFile.clear();
    _data.clear();
}

