#include "dmhmessage.h"
#include <QString>
#include <QRegularExpression>
#include <QDebug>

DMHMessage::DMHMessage() :
    _id(),
    _sender(),
    _body(),
    _command(),
    _message(),
    _polled(false),
    _timestamp()
{
}

DMHMessage::DMHMessage(const QString& id, const QString& sender, const QString& body, bool polled, const QString& timestamp) :
    _id(id),
    _sender(sender),
    _body(body),
    _command(),
    _message(),
    _polled(polled),
    _timestamp(timestamp)
{
    convertBodyToParts();
}

DMHMessage::DMHMessage(const QString& command, const QString& message) :
    _id(),
    _sender(),
    _body(),
    _command(command),
    _message(message),
    _polled(false),
    _timestamp()
{
    convertPartsToBody();
}

DMHMessage::DMHMessage(const DMHMessage& other) :
    _id(other._id),
    _sender(other._sender),
    _body(other._body),
    _command(other._command),
    _message(other._message),
    _polled(other._polled),
    _timestamp(other._timestamp)
{
}

DMHMessage::~DMHMessage()
{
}

bool DMHMessage::isEmpty() const
{
    return _body.isEmpty();
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
    if(_body != body)
    {
        _body = body;
        convertBodyToParts();
    }
}

QString DMHMessage::getCommand() const
{
    return _command;
}

void DMHMessage::setCommand(const QString& command)
{
    if(_command != command)
    {
        _command = command;
        convertPartsToBody();
    }
}

QString DMHMessage::getMessage() const
{
    return _message;
}

void DMHMessage::setMessage(const QString& message)
{
    if(_message != message)
    {
        _message = message;
        convertPartsToBody();
    }
}

void DMHMessage::setParts(const QString& command, const QString& message)
{
    if((_command != command) || (_message != message))
    {
        _command = command;
        _message = message;
        convertPartsToBody();
    }
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

QDateTime DMHMessage::getTimestampConverted() const
{
    return QDateTime::fromString(_timestamp);
}

void DMHMessage::setTimestamp(const QString& timestamp)
{
    _timestamp = timestamp;
}

QDebug operator<<(QDebug d, const DMHMessage &message)
{
    d << QString("(DMHMessage: ID: ") << message.getId() << QString(", Sender: ") << message.getSender() << QString(", Body: ") << message.getBody() << QString(", Command: ") << message.getCommand() << QString(", Message: ") << message.getMessage() << QString(", Polled: ") << message.isPolled() << QString(", Timestamp: ") << message.getTimestamp() << QString(")");
    return d;
}

void DMHMessage::convertBodyToParts()
{
    if(_body.isEmpty())
        return;

    QRegularExpression reBody("<body(?: cmd=)?(.*)>(.*)<\\/body>");
    QRegularExpressionMatch matchBody = reBody.match(_body);
    _command = matchBody.captured(1);
    _message = matchBody.captured(2);
}

void DMHMessage::convertPartsToBody()
{
    _body = _command.isEmpty() ? QString("<body>") : QString("<body cmd=") + _command + QString(">");
    _body += _message + QString("</body>");
}
