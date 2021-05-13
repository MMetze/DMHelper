#include "dmhpayload.h"
#include "dmhpayload_private.h"
#include <QDomElement>
#include <QString>

DMHPayload::DMHPayload() :
    d(new DMHPayload_Private())
{
}

DMHPayload::DMHPayload(const QString& data) :
    d(new DMHPayload_Private(data))
{
}

DMHPayload::~DMHPayload()
{
}

QString DMHPayload::getImageFile() const
{
    return d->getImageFile();
}

void DMHPayload::setImageFile(const QString& imageFile)
{
    d->setImageFile(imageFile);
}

QString DMHPayload::getAudioFile() const
{
    return d->getAudioFile();
}

void DMHPayload::setAudioFile(const QString& audioFile)
{
    d->setAudioFile(audioFile);
}

QString DMHPayload::getData() const
{
    return d->getData();
}

void DMHPayload::setData(const QString& data)
{
    d->setData(data);
}

void DMHPayload::fromString(const QString& data)
{
    d->fromString(data);
}

QString DMHPayload::toString() const
{
    return d->toString();
}

void DMHPayload::clear()
{
    d->clear();
}

