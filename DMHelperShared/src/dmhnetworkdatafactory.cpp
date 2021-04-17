#include "dmhnetworkdatafactory.h"
#include "dmhnetworkdata.h"
#include <QDebug>


#include <QFile>
#include <QTextStream>



DMHNetworkDataFactory::DMHNetworkDataFactory(const QByteArray& data) :
    _doc(),
    _version(),
    _mode(),
    _modeValue(DMHShared::DMH_Message_INVALID),
    _state(),
    _error(),
    _dataElement(),
    _data(nullptr)
{
    QByteArray dataFromPercent = QByteArray::fromPercentEncoding(data);
    //QString dataString = QString(dataFromPercent);
    qDebug() << "[DMHNetworkDataFactory] Data contents: " << dataFromPercent;
    while((dataFromPercent.length() > 0) && (dataFromPercent.front() == '\n'))
        dataFromPercent.remove(0, 1);

    qDebug() << "[DMHNetworkDataFactory] Data contents: " << dataFromPercent;

    QString errorMsg;
    int errorLine;
    int errorColumn;
    if(!_doc.setContent(dataFromPercent, &errorMsg, &errorLine, &errorColumn))
    {
        qDebug() << "[NetworkDataFactory] ERROR identified reading data: unable to parse network reply XML at line " << errorLine << ", column " << errorColumn << ": " << errorMsg;
        qDebug() << "[NetworkDataFactory] DataFromPercent: " << dataFromPercent;
        return;
    }

    // DEBUG: FULL PAYLOAD OUTPUT
    //qDebug() << "[NetworkDataFactory] PAYLOAD: " << _doc.toString();

    QDomElement root = _doc.documentElement();
    if(root.isNull())
    {
        qDebug() << "[NetworkDataFactory] ERROR identified reading data: unable to find root element: " << _doc.toString();
        return;
    }

    _version = root.firstChildElement(QString("version"));
    if(_version.isNull())
    {
        qDebug() << "[NetworkDataFactory] ERROR identified reading data: unable to find 'version' element: " << _doc.toString();
        return;
    }

    _mode = root.firstChildElement(QString("mode"));
    if(_mode.isNull())
    {
        qDebug() << "[NetworkDataFactory] ERROR identified reading data: unable to find 'mode' element: " << _doc.toString();
        return;
    }
    _modeValue = static_cast<DMHShared::DMH_Message>(_mode.firstChildElement(QString("int")).text().toInt());

    _state = root.firstChildElement(QString("state"));
    if(_state.isNull())
    {
        qDebug() << "[NetworkDataFactory] ERROR identified reading data: unable to find 'state' element: " << _doc.toString();
        return;
    }

    _error = root.firstChildElement(QString("error"));
    if(_error.isNull())
    {
        qDebug() << "[NetworkDataFactory] ERROR identified reading data: unable to find 'error' element: " << _doc.toString();
        return;
    }

    _dataElement = root.firstChildElement(QString("data"));
    if(_dataElement.isNull())
    {
        qDebug() << "[NetworkDataFactory] ERROR identified reading data: unable to find 'data' element: " << _doc.toString();
        return;
    }
    else
    {
        if(!readDataElement())
        {
            qDebug() << "[NetworkDataFactory] ERROR identified interpreting data element: " << _doc.toString();
            return;
        }
    }
}

QDomElement DMHNetworkDataFactory::getVersion() const
{
    return _version;
}

QDomElement DMHNetworkDataFactory::getMode() const
{
    return _mode;
}

DMHShared::DMH_Message DMHNetworkDataFactory::getModeValue() const
{
    return _modeValue;
}

QDomElement DMHNetworkDataFactory::getState() const
{
    return _state;
}

QDomElement DMHNetworkDataFactory::getError() const
{
    return _error;
}

QDomElement DMHNetworkDataFactory::getDataElement() const
{
    return _dataElement;
}

std::unique_ptr<DMHNetworkData>& DMHNetworkDataFactory::getData()
{
    return _data;
}

bool DMHNetworkDataFactory::readDataElement()
{
    switch(_modeValue)
    {
        case DMHShared::DMH_Message_pl_pull:
            _data.reset(new DMHNetworkData_Payload(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_ssn_ass:
            _data.reset(new DMHNetworkData_JoinSession(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_file_push:
            _data.reset(new DMHNetworkData_Upload(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_file_pull:
            _data.reset(new DMHNetworkData_Raw(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_file_exists:
            _data.reset(new DMHNetworkData_Exists(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_ssn_create:
            _data.reset(new DMHNetworkData_CreateSession(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_ssn_isowner:
            _data.reset(new DMHNetworkData_IsOwner(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_ssn_rename:
            _data.reset(new DMHNetworkData_RenameSession(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_ssn_renew:
            _data.reset(new DMHNetworkData_RenewSession(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_ssn_close:
            _data.reset(new DMHNetworkData_CloseSession(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_ssn_members:
            _data.reset(new DMHNetworkData_SessionMembers(_dataElement));
            return _data->isValid();
        case DMHShared::DMH_Message_usr_create:
            _data.reset(new DMHNetworkData_CreateUser(_dataElement));
            return _data->isValid();
        default:
            qDebug() << "[NetworkDataFactory] ERROR unsupported mode type: " << _modeValue;
            return false;
    }
}

/*
bool DMHNetworkDataFactory::isPayloadData() const
{
    return ((!_data.isNull()) && (!_data.firstChildElement(QString("payload")).isNull()));
}

QString DMHNetworkDataFactory::getPayloadData() const
{
    if(_data.isNull())
        return QString();

    QDomElement payloadElement = _data.firstChildElement(QString("payload"));
    if(payloadElement.isNull())
        return QString();

    return payloadElement.text();
}

QString DMHNetworkDataFactory::getPayloadTimestamp() const
{
    if(_data.isNull())
        return QString();

    QDomElement timestampElement = _data.firstChildElement(QString("last"));
    if(timestampElement.isNull())
        return QString();

    return timestampElement.text();
}

bool DMHNetworkDataFactory::isRawData() const
{
    return ((!_data.isNull()) && (!_data.firstChildElement(QString("node")).isNull()));
}

QString DMHNetworkDataFactory::getRawName() const
{
    if(_data.isNull())
        return QString();

    QDomElement nodeElement = _data.firstChildElement(QString("node"));
    if(nodeElement.isNull())
        return QString();

    QDomElement nameElement = nodeElement.firstChildElement(QString("name"));
    if(nameElement.isNull())
        return QString();

    return nameElement.text();
}

QString DMHNetworkDataFactory::getRawId() const
{
    if(_data.isNull())
        return QString();

    QDomElement nodeElement = _data.firstChildElement(QString("node"));
    if(nodeElement.isNull())
        return QString();

    QDomElement IDElement = nodeElement.firstChildElement(QString("ID"));
    if(IDElement.isNull())
        return QString();

    return IDElement.text();
}

QByteArray DMHNetworkDataFactory::getRawData() const
{
    if(_data.isNull())
        return QByteArray();

    QDomElement nodeElement = _data.firstChildElement(QString("node"));
    if(nodeElement.isNull())
        return QByteArray();

    QDomElement dataElement = nodeElement.firstChildElement(QString("data"));
    if(dataElement.isNull())
        return QByteArray();

    QString datatext = dataElement.text();
    QByteArray base64data = datatext.toUtf8();
    return QByteArray::fromBase64(base64data);
}
*/
