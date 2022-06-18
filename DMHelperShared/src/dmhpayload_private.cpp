#include "dmhpayload_private.h"
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

DMHPayload_Private::DMHPayload_Private() :
    _imageFile(),
    _audioFile(),
    _command(),
    _payload()
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

QString DMHPayload_Private::getCommand() const
{
    return _command;
}

void DMHPayload_Private::setCommand(const QString& command)
{
    _command = command;
}

QString DMHPayload_Private::getPayload() const
{
    return _payload;
}

void DMHPayload_Private::setPayload(const QString& payload)
{
    _payload = payload;
}

void DMHPayload_Private::fromString(const QString& data)
{
    QRegularExpression reImg("<image>(\\S*)<\\/image>");
    QRegularExpressionMatch matchImg = reImg.match(data);
    setImageFile(matchImg.hasMatch() ? matchImg.captured(matchImg.lastCapturedIndex()).remove(QChar('-')) : QString());

    QRegularExpression reAudio("<audio>(\\S*)<\\/audio>");
    QRegularExpressionMatch matchAudio = reAudio.match(data);
    setAudioFile(matchAudio.hasMatch() ? matchAudio.captured(matchAudio.lastCapturedIndex()).remove(QChar('-')) : QString());

    QRegularExpression reCommand("<command>(\\S*)<\\/command>");
    QRegularExpressionMatch matchCommand = reCommand.match(data);
    setCommand(matchCommand.hasMatch() ? matchCommand.captured(matchCommand.lastCapturedIndex()) : QString());

    QRegularExpression rePayload("<payload>(\\S*)<\\/payload>");
    QRegularExpressionMatch matchPayload = rePayload.match(data);
    setCommand(matchPayload.hasMatch() ? matchPayload.captured(matchPayload.lastCapturedIndex()) : QString());
}

QString DMHPayload_Private::toString() const
{
    QString result;

    result += QString("<image>") + _imageFile + QString("</image>");
    result += QString("<audio>") + _audioFile + QString("</audio>");
    result += QString("<command>") + _command + QString("</command>");
    result += QString("<payload>") + _payload + QString("</payload>");

    return result;
}
