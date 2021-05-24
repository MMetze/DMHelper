#ifndef DMHNETWORKDATAFACTORY_H
#define DMHNETWORKDATAFACTORY_H

#include <QString>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <memory>
#include "dmhshared.h"
#include "dmhnetworkdata.h"

class DMHNetworkDataFactory
{
public:
    explicit DMHNetworkDataFactory(const QByteArray& data);

    QDomElement getVersion() const;
    QDomElement getMode() const;
    DMHShared::DMH_Message getModeValue() const;
    QString getModeString() const;
    QDomElement getState() const;
    DMHShared::DMH_Message_State getStateValue() const;
    QDomElement getError() const;
    QString getErrorString() const;
    QDomElement getDataElement() const;
    std::unique_ptr<DMHNetworkData>& getData(); // IMPORTANT: survives only as long as the DMHNetworkDataFactory!!!

/*
    bool isPayloadData() const;
    QString getPayloadData() const;
    QString getPayloadTimestamp() const;

    bool isRawData() const;
    QString getRawName() const;
    QString getRawId() const;
    QByteArray getRawData() const;
    */

private:
    bool readDataElement();

    QDomDocument _doc;
    QDomElement _version;
    QDomElement _mode;
    DMHShared::DMH_Message _modeValue;
    QDomElement _state;
    QDomElement _error;
    QDomElement _dataElement;

    std::unique_ptr<DMHNetworkData> _data;

};

#endif // DMHNETWORKDATAFACTORY_H
