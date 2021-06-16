#ifndef DMHMESSAGE_H
#define DMHMESSAGE_H

#include "dmhglobal.h"
#include <memory>
#include <QString>
#include <QDateTime>

class DMHMessage
{
public:
    DMHMessage();
    explicit DMHMessage(const QString& id, const QString& sender, const QString& body, bool polled, const QString& timestamp);
    explicit DMHMessage(const QString& command, const QString& message);
    explicit DMHMessage(const DMHMessage& other);
    virtual ~DMHMessage();

    bool isEmpty() const;

    QString getId() const;
    void setId(const QString& id);

    QString getSender() const;
    void setSender(const QString& sender);

    QString getBody() const;
    void setBody(const QString& body);

    QString getCommand() const;
    void setCommand(const QString& command);

    QString getMessage() const;
    void setMessage(const QString& message);

    void setParts(const QString& command, const QString& message);

    bool isPolled() const;
    void setPolled(bool polled);

    QString getTimestamp() const;
    QDateTime getTimestampConverted() const;
    void setTimestamp(const QString& timestamp);

    friend QDebug operator<<(QDebug d, const DMHMessage &message);

protected:
    void convertBodyToParts();
    void convertPartsToBody();

private:
    QString _id;
    QString _sender;
    QString _body;
    QString _command;
    QString _message;
    bool _polled;
    QString _timestamp;

};

#endif // DMHMESSAGE_H
