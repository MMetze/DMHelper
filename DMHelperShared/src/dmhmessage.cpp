#include "dmhmessage.h"
#include <QString>

DMHMessage::DMHMessage() :
    _id(),
    _sender(),
    _body(),
    _polled(false),
    _timestamp()
{
}

DMHMessage::DMHMessage(const QString& id, const QString& sender, const QString& body, bool polled, const QString& timestamp) :
    _id(id),
    _sender(sender),
    _body(body),
    _polled(polled),
    _timestamp(timestamp)
{
}

DMHMessage::DMHMessage(const DMHMessage& other) :
    _id(other._id),
    _sender(other._sender),
    _body(other._body),
    _polled(other._polled),
    _timestamp(other._timestamp)
{
}

DMHMessage::~DMHMessage()
{
}

QString DMHMessage::getId() const
{
    return _id;
}

void DMHMessage::setId(const QString& id)
{
    _id = id;
}

QString DMHMessage::getSender() const
{
    return _sender;
}

void DMHMessage::setSender(const QString& sender)
{
    _sender = sender;
}

QString DMHMessage::getBody() const
{
    return _body;
}

void DMHMessage::setBody(const QString& body)
{
    _body = body;
}

bool DMHMessage::isPolled() const
{
    return _polled;
}

void DMHMessage::setPolled(bool polled)
{
    _polled = polled;
}

QString DMHMessage::getTimestamp() const
{
    return _timestamp;
}

void DMHMessage::setTimestamp(const QString& timestamp)
{
    _timestamp = timestamp;
}
