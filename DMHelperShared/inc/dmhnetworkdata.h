#ifndef DMHNETWORKDATA_H
#define DMHNETWORKDATA_H

#include "dmhglobal.h"
#include "dmhpayload.h"
#include <QDomElement>
#include <memory>

class QString;
class DMHNetworkData_Private;
class DMHNetworkData_Payload_Private;
class DMHNetworkData_Raw_Private;
class DMHNetworkData_Upload_Private;
class DMHNetworkData_Exists_Private;


//class DMHSHARED_EXPORT DMHNetworkData
class DMHNetworkData
{
public:
    DMHNetworkData();
    virtual ~DMHNetworkData();

    virtual bool isValid() = 0;

private:
    // No copy constructor allowed
    DMHNetworkData(const DMHNetworkData& other);

    std::unique_ptr<DMHNetworkData_Private> d;
};



//class DMHSHARED_EXPORT DMHNetworkData_Payload : public DMHNetworkData
class DMHNetworkData_Payload : public DMHNetworkData
{
public:
    explicit DMHNetworkData_Payload(QDomElement data);
    virtual ~DMHNetworkData_Payload() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    const DMHPayload& getPayload() const;
    QString getTimestamp() const;

private:
    // No copy constructor allowed
    DMHNetworkData_Payload(const DMHNetworkData_Payload& other);

    std::unique_ptr<DMHNetworkData_Payload_Private> d;
};



//class DMHSHARED_EXPORT DMHNetworkData_Raw : public DMHNetworkData
class DMHNetworkData_Raw : public DMHNetworkData
{
public:
    explicit DMHNetworkData_Raw(QDomElement data);
    virtual ~DMHNetworkData_Raw() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getName() const;
    QString getId() const;
    QByteArray getData() const;

private:
    // No copy constructor allowed
    DMHNetworkData_Raw(const DMHNetworkData_Raw& other);

    std::unique_ptr<DMHNetworkData_Raw_Private> d;
};





//class DMHSHARED_EXPORT DMHNetworkData_Upload : public DMHNetworkData
class DMHNetworkData_Upload : public DMHNetworkData
{
public:
    explicit DMHNetworkData_Upload(QDomElement data);
    virtual ~DMHNetworkData_Upload() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getName() const;
    QString getId() const;

private:
    // No copy constructor allowed
    DMHNetworkData_Upload(const DMHNetworkData_Upload& other);

    std::unique_ptr<DMHNetworkData_Upload_Private> d;
};






//class DMHSHARED_EXPORT DMHNetworkData_Exists : public DMHNetworkData
class DMHNetworkData_Exists : public DMHNetworkData
{
public:
    explicit DMHNetworkData_Exists(QDomElement data);
    virtual ~DMHNetworkData_Exists() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    bool exists() const;
    QString getName() const;
    QString getMD5() const;

private:
    // No copy constructor allowed
    DMHNetworkData_Exists(const DMHNetworkData_Exists& other);

    std::unique_ptr<DMHNetworkData_Exists_Private> d;
};


#endif // DMHNETWORKDATA_H
