#ifndef DMHMESSAGE_H
#define DMHMESSAGE_H

#include "dmhglobal.h"
#include <memory>
#include <QString>

class DMHMessage
{
public:
    DMHMessage();
    explicit DMHMessage(const QString& id, const QString& sender, const QString& body, bool polled, const QString& timestamp);
    explicit DMHMessage(const DMHMessage& other);
    virtual ~DMHMessage();

    QString getId() const;
    void setId(const QString& id);

    QString getSender() const;
    void setSender(const QString& sender);

    QString getBody() const;
    void setBody(const QString& body);

    bool isPolled() const;
    void setPolled(bool polled);

    QString getTimestamp() const;
    void setTimestamp(const QString& timestamp);

private:
    QString _id;
    QString _sender;
    QString _body;
    bool _polled;
    QString _timestamp;

};

#endif // DMHMESSAGE_H
