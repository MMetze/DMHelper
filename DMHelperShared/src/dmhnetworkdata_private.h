#ifndef DMHNETWORKDATA_PRIVATE_H
#define DMHNETWORKDATA_PRIVATE_H

#include <QString>
#include <QDomElement>
#include "dmhpayload.h"

class QByteArray;

class DMHNetworkData_Private_Base
{
public:
    DMHNetworkData_Private_Base();
    virtual ~DMHNetworkData_Private_Base();

    virtual bool readChildElement(QDomElement element, const QString& childName, QString& stringOut);
    virtual bool readChildElement(QDomElement element, const QString& childName, QByteArray& baOut);

private:

};




class DMHNetworkData_Private : public DMHNetworkData_Private_Base
{
public:
    DMHNetworkData_Private();
    virtual ~DMHNetworkData_Private();

private:

};



class DMHNetworkData_Payload_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_Payload_Private(QDomElement data);
    virtual ~DMHNetworkData_Payload_Private() override;

    // From DMHNetworkData
    virtual bool isValid();

    const DMHPayload& getPayload() const;
    QString getTimestamp() const;

protected:
    DMHPayload _payload;
    QString _timestamp;
    bool _valid;
};



class DMHNetworkData_Raw_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_Raw_Private(QDomElement data);
    virtual ~DMHNetworkData_Raw_Private() override;

    // From DMHNetworkData
    virtual bool isValid();

    QString getName() const;
    QString getId() const;
    QByteArray getData() const;

protected:
    QString _name;
    QString _id;
    QByteArray _data;
    bool _valid;
};





class DMHNetworkData_Upload_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_Upload_Private(QDomElement data);
    virtual ~DMHNetworkData_Upload_Private() override;

    // From DMHNetworkData
    virtual bool isValid();

    QString getName() const;
    QString getId() const;

protected:
    QString _name;
    QString _id;
    bool _valid;
};




class DMHNetworkData_Exists_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_Exists_Private(QDomElement data);
    virtual ~DMHNetworkData_Exists_Private() override;

    // From DMHNetworkData
    virtual bool isValid();

    bool exists() const;
    QString getName() const;
    QString getMD5() const;

protected:
    QString _name;
    QString _md5;
    bool _valid;
};


#endif // DMHNETWORKDATA_PRIVATE_H
