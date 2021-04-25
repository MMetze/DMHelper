#include "remoterenderer.h"
#include "dmc_constants.h"
#include "dmconstants.h"
#include "campaignobjectbase.h"
#include "encountertextdownload.h"
#include "encounterbattledownload.h"
#include "campaignobjectrenderer.h"
#include "encountertextrenderer.h"
#include "battlerenderer.h"
#include <QRegularExpression>
#include <QFile>
#include <QUuid>
#include <QDomDocument>
#include <QPainter>
#include <QDebug>

RemoteRenderer::RemoteRenderer(const QString& cacheDirectory, QObject *parent) :
    QObject(parent),
    _activeObject(nullptr),
    _renderer(nullptr),
    _background(nullptr),
    _fow(nullptr),
    _payloadFiles(),
    _backgroundPmp(),
    _fowPmp(),
    _cacheDirectory(cacheDirectory),
    _targetSize(-1, -1)
{
}

RemoteRenderer::~RemoteRenderer()
{
    qDeleteAll(_payloadFiles);
}

void RemoteRenderer::parseImageData(const QString& imageData, const QString& payloadData)
{
    QDomElement payloadElement;

    if(!payloadData.isEmpty())
    {
//        QString payloadString = payloadData;
//        payloadString = QString("<root>") + payloadString.replace(QString("\\"""), QString("""")) + QString("</root>");
        QString payloadString = QString("<root>") + payloadData + QString("</root>");
        QDomDocument doc;
        QString errorMsg;
        int errorLine;
        int errorColumn;
        if(!doc.setContent(payloadString, &errorMsg, &errorLine, &errorColumn))
        {
            qDebug() << "[RemoteRenderer] ERROR identified reading image data: unable to parse XML at line " << errorLine << ", column " << errorColumn << ": " << errorMsg;
        }
        else
        {
            qDebug() << "[RemoteRenderer] Image String: " << payloadString;
            payloadElement = doc.documentElement().firstChildElement();
            parsePayloadData(payloadElement);
        }
    }

    QRegularExpression reFoW("<fow>(.*),(.*)<\\/fow>");
    QRegularExpressionMatch matchFoW = reFoW.match(imageData);
    QString fowMD5 = matchFoW.captured(1);
    QString fowUuid = matchFoW.captured(2);
    if((!_fow) || (_fow->getMD5() != fowMD5) || (_fow->isNotStarted()))
    {
        _fowPmp = QPixmap();
        if((_fow) && (_fow->getMD5() != fowMD5))
        {
            qDebug() << "[RemoteRenderer] Deleting FoW wrapper for md5: " << _fow->getMD5();
            _fow->deleteLater();
            _fow = nullptr;
        }

        if(_fow)
        {
            qDebug() << "[RemoteRenderer] Restarting download for FoW image: " << fowMD5;
            _fow->getFile();
        }
        else if((!fowMD5.isEmpty()) && (!fowUuid.isEmpty()))
        {
            qDebug() << "[RemoteRenderer] New FoW image found: " << fowMD5;
            _fow = new RemoteRenderer_FileWrapper(fowMD5, fowUuid, DMHelper::FileType_Image);
            connect(_fow, &RemoteRenderer_FileWrapper::requestFile, this, &RemoteRenderer::requestFile);
            connect(_fow, &RemoteRenderer_FileWrapper::dataAvailable, this, &RemoteRenderer::setFowData);
            _fow->getFile();
        }
    }

    QRegularExpression reBackground("<background(?: type=)?(.*)>(.*),(.*)<\\/background>");
    QRegularExpressionMatch matchBackground = reBackground.match(imageData);
    QString backgroundTypeString = matchBackground.captured(1);
    QString backgroundMD5 = matchBackground.captured(2);
    QString backgroundUuid = matchBackground.captured(3);
    if((!_background) || (_background->getMD5() != backgroundMD5) || (_background->isNotStarted()))
    {
        _backgroundPmp = QPixmap();
        if((_background) && (_background->getMD5() != backgroundMD5))
        {
            qDebug() << "[RemoteRenderer] Deleting Background wrapper for md5: " << _background->getMD5();
            _background->deleteLater();
            _background = nullptr;
        }

        if(_background)
        {
            qDebug() << "[RemoteRenderer] Restarting download for Background image: " << backgroundMD5;
            _background->getFile();
        }
        else if((!backgroundMD5.isEmpty()) && (!backgroundUuid.isEmpty()))
        {
            qDebug() << "[RemoteRenderer] New Background image found: " << backgroundMD5 << ", UUID: " << backgroundUuid;
            _background = new RemoteRenderer_FileWrapper(backgroundMD5, backgroundUuid, backgroundTypeString.toInt());
            connect(_background, &RemoteRenderer_FileWrapper::requestFile, this, &RemoteRenderer::requestFile);
            connect(_background, &RemoteRenderer_FileWrapper::dataAvailable, this, &RemoteRenderer::setBackgroundData);
            _background->getFile();
        }
    }

    requestPayloadData();
}

void RemoteRenderer::targetResized(const QSize& newSize)
{
    if(newSize != _targetSize)
    {
        _targetSize = newSize;

        if(_renderer)
            _renderer->targetResized(newSize);
    }
}

void RemoteRenderer::loadBattle()
{
    QFile localPayload("_dmhpayload.txt");
    localPayload.open(QIODevice::ReadOnly);
    QTextStream in(&localPayload);
    in.setCodec("UTF-8");
    QDomDocument doc("DMHelperXML");
    doc.setContent( in.readAll() );
    localPayload.close();

    QDomElement root = doc.documentElement();
    QDomElement battleElement = root.firstChildElement(QString("battle"));

    /*
    _battleDlgModel = new BattleDialogModel(this);
    _battleDlgModel->inputXML(battleElement);

    _battleDlg = new BattleDialog(*_battleDlgModel, this);
    _battleDlg->updateMap();
    _battleDlg->show();
    */
}

bool RemoteRenderer::isComplete() const
{
    if((_background) && (_background->getStatus() != DMC_SERVER_CONNECTION_UPLOAD_COMPLETE))
        return false;

    if((_fow) && (_fow->getStatus() != DMC_SERVER_CONNECTION_UPLOAD_COMPLETE))
        return false;

    for(int i = 0; i < _payloadFiles.count(); ++i)
    {
        if((_payloadFiles.at(i)) && (_payloadFiles.at(i)->getStatus() != DMC_SERVER_CONNECTION_UPLOAD_COMPLETE))
            return false;
    }

    return true;
}

void RemoteRenderer::fileRequestStarted(int requestId, const QString& fileMD5, const QString& fileUuid)
{
    qDebug() << "[RemoteRenderer] File request started for " << fileMD5 << ", " << fileUuid << " with request " << requestId;

    if((_fow) && (_fow->getMD5() == fileMD5))
        _fow->setStatus(requestId);

    if((_background) && (_background->getMD5() == fileMD5))
        _background->setStatus(requestId);

    for(int i = 0; i < _payloadFiles.count(); ++i)
    {
        if((_payloadFiles.at(i)) && (_payloadFiles.at(i)->getMD5() == fileMD5))
            _payloadFiles.at(i)->setStatus(requestId);
    }
}

void RemoteRenderer::fileRequestCompleted(int requestId, const QString& fileMD5, const QString& fileUuid, const QByteArray& data)
{
    qDebug() << "[RemoteRenderer] File request completed for " << fileMD5 << ", " << fileUuid << " with request " << requestId;
    if(_fow)
        _fow->fileReceived(fileMD5, fileUuid, _cacheDirectory, data);

    if(_background)
        _background->fileReceived(fileMD5, fileUuid, _cacheDirectory, data);

    for(int i = 0; i < _payloadFiles.count(); ++i)
    {
        if(_payloadFiles.at(i))
            _payloadFiles.at(i)->fileReceived(fileMD5, fileUuid, _cacheDirectory, data);
    }
}

void RemoteRenderer::setBackgroundData(const QString& md5, const QString& uuid, const QByteArray& data)
{
    Q_UNUSED(md5);
    Q_UNUSED(uuid);

    if(!data.isEmpty())
        _backgroundPmp.loadFromData(data);

    dataComplete();
}

void RemoteRenderer::setFowData(const QString& md5, const QString& uuid, const QByteArray& data)
{
    Q_UNUSED(md5);
    Q_UNUSED(uuid);

    if(!data.isEmpty())
        _fowPmp.loadFromData(data);

    dataComplete();
}

void RemoteRenderer::dataComplete()
{
    if(!isComplete())
        return;

    if(_activeObject)
    {
        if(_activeObject->getObjectType() == DMHelper::CampaignType_Text)
        {
            EncounterText* textEncounter = dynamic_cast<EncounterText*>(_activeObject);
            if(textEncounter)
            {
                if(_background)
                    textEncounter->setImageFile(_cacheDirectory + QString("/") + _background->getMD5());
                _renderer = new EncounterTextRenderer(*textEncounter, _backgroundPmp.toImage(), this);
            }
        }
        else if(_activeObject->getObjectType() == DMHelper::CampaignType_Battle)
        {
            EncounterBattle* battle = dynamic_cast<EncounterBattle*>(_activeObject);
            if(battle)
            {
                if(_background)
                    battle->setImageFile(_cacheDirectory + QString("/") + _background->getMD5());
                _renderer = new BattleRenderer(*battle, _backgroundPmp, this);
            }
        }
        else
        {
            qDebug() << "[RemoteRenderer] ERROR: Preparing payload data, unexpected object type found: " << _activeObject->getObjectType();
        }

        if(_renderer)
        {
            connect(_renderer, &CampaignObjectRenderer::publishImage, this, &RemoteRenderer::publishImage);
            connect(_renderer, &CampaignObjectRenderer::animateImage, this, &RemoteRenderer::publishImage);
            _renderer->targetResized(_targetSize);
            _renderer->startRendering();
        }
    }
    else
    {
        if(_fowPmp.isNull())
        {
            emit publishPixmap(_backgroundPmp);
        }
        else
        {
            QPixmap result(_backgroundPmp);
            QPainter p;
            p.begin(&result);
                p.drawPixmap(0, 0, _fowPmp);
            p.end();
            emit publishPixmap(result);
        }
    }
}

void RemoteRenderer::parsePayloadData(const QDomElement& element)
{
    if(element.isNull())
        return;

    if(_activeObject)
    {
        if(_activeObject->getID() == QUuid(element.attribute(QString("_baseID"))))
        {
            return;
        }
        else
        {
            _activeObject->deleteLater();
            _activeObject = nullptr;

            if(_renderer)
            {
                _renderer->deleteLater();
                _renderer = nullptr;
            }
        }
    }

    QString tagName = element.tagName();
    if(tagName == "entry-object")
    {
        EncounterTextDownload* encounter = new EncounterTextDownload(_cacheDirectory, this);
        connect(encounter, &EncounterTextDownload::requestFile, this, &RemoteRenderer::payloadDataRequested);
        connect(this, &RemoteRenderer::payloadDataAvailable, encounter, &EncounterTextDownload::fileReceived);
        connect(encounter, &EncounterTextDownload::encounterComplete, this, &RemoteRenderer::dataComplete);
        encounter->inputXML(element, false);
        _activeObject = encounter;
    }
    else if(tagName == "battle-object")
    {
        EncounterBattleDownload* battle = new EncounterBattleDownload(_cacheDirectory, this);
        connect(battle, &EncounterBattleDownload::requestFile, this, &RemoteRenderer::payloadDataRequested);
        connect(this, &RemoteRenderer::payloadDataAvailable, battle, &EncounterBattleDownload::fileReceived);
        connect(battle, &EncounterBattleDownload::encounterComplete, this, &RemoteRenderer::dataComplete);
        battle->inputXML(element, false);
        _activeObject = battle;
    }
}

void RemoteRenderer::requestPayloadData()
{
    for(int i = 0; i < _payloadFiles.count(); ++i)
    {
        if(_payloadFiles.at(i))
            _payloadFiles.at(i)->getFile();
    }
}

void RemoteRenderer::payloadDataRequested(const QString& md5String, const QString& uuid, int fileType)
{
    for(int i = 0; i < _payloadFiles.count(); ++i)
    {
        if((_payloadFiles.at(i)) && (_payloadFiles.at(i))->getMD5() == md5String)
            return;
    }

    // TODO: requested data needs UUIDs too
    RemoteRenderer_FileWrapper* fileWrapper = new RemoteRenderer_FileWrapper(md5String, uuid, fileType);
    connect(fileWrapper, &RemoteRenderer_FileWrapper::requestFile, this, &RemoteRenderer::requestFile);
    connect(fileWrapper, &RemoteRenderer_FileWrapper::abortRequest, this, &RemoteRenderer::abortRequest);
    connect(fileWrapper, &RemoteRenderer_FileWrapper::dataAvailable, this, &RemoteRenderer::payloadDataAvailable);
    _payloadFiles.append(fileWrapper);
}

RemoteRenderer_FileWrapper::RemoteRenderer_FileWrapper(const QString& md5, const QString& uuid, int fileType) :
    _md5(md5),
    _fileType(fileType),
    _uuid(uuid),
    _status(DMC_SERVER_CONNECTION_UPLOAD_NOT_STARTED)
{
}

/*
RemoteRenderer_FileWrapper::RemoteRenderer_FileWrapper(const QDomElement& element) :
    _md5(),
    _id(element.attribute(QString("id"))),
    _status(DMC_SERVER_CONNECTION_UPLOAD_NOT_STARTED)
{
    bool ok = false;
    int type = element.attribute("type").toInt(&ok);
    if(!ok)
        return;

    //QString trackName = element.attribute("trackname");

    switch(type)
    {
        default:
            return;
    }
}
*/

RemoteRenderer_FileWrapper::~RemoteRenderer_FileWrapper()
{
    if(_status > 0)
        emit abortRequest(_status);
}

QString RemoteRenderer_FileWrapper::getMD5() const
{
    return _md5;
}

void RemoteRenderer_FileWrapper::setMD5(const QString& md5)
{
    _md5 = md5;
}

int RemoteRenderer_FileWrapper::getFileType() const
{
    return _fileType;
}

void RemoteRenderer_FileWrapper::setFileType(int fileType)
{
    _fileType = fileType;
}

QString RemoteRenderer_FileWrapper::getUuid() const
{
    return _uuid;
}

void RemoteRenderer_FileWrapper::setUuid(const QString& uuid)
{
    _uuid = uuid;
}

int RemoteRenderer_FileWrapper::getStatus() const
{
    return _status;
}

void RemoteRenderer_FileWrapper::setStatus(int status)
{
    _status = status;
}

bool RemoteRenderer_FileWrapper::isNotStarted() const
{
    return((_status == DMC_SERVER_CONNECTION_UPLOAD_NOT_STARTED) ||
           (_status == DMC_SERVER_CONNECTION_CONTROLLER_UPLOAD_ERROR));
}

void RemoteRenderer_FileWrapper::getFile()
{
    if(_status > 0)
        return;

    emit requestFile(_md5, _uuid);
}

void RemoteRenderer_FileWrapper::fileReceived(const QString& md5, const QString& uuid, const QString& cacheDirectory, const QByteArray& data)
{
    if(_md5 != md5)
        return;

    QString fullFileName = cacheDirectory + QString("/") + _md5;
    if(QFile::exists(fullFileName))
    {
        if(_fileType == DMHelper::FileType_Video)
        {
            qDebug() << "[RemoteRenderer_FileWrapper] File available: " << fullFileName;
            _status = DMC_SERVER_CONNECTION_UPLOAD_COMPLETE;
            emit dataAvailable(md5, uuid, QByteArray());
        }
        else
        {
            QFile readFile(fullFileName);
            if(!readFile.open(QIODevice::ReadOnly))
            {
                qDebug() << "[RemoteRenderer_FileWrapper] ERROR: not able to open input file for reading: " << readFile.error() << ", " << readFile.errorString();
                _status = DMC_SERVER_CONNECTION_CONTROLLER_UPLOAD_ERROR;
                return;
            }

            QByteArray readArray = readFile.readAll();
            if(readArray.isEmpty())
            {
                qDebug() << "[RemoteRenderer_FileWrapper] ERROR: not able to read input file: " << readFile.error() << ", " << readFile.errorString();
                _status = DMC_SERVER_CONNECTION_CONTROLLER_UPLOAD_ERROR;
                return;
            }

            qDebug() << "[RemoteRenderer_FileWrapper] File read: " << fullFileName;
            _status = DMC_SERVER_CONNECTION_UPLOAD_COMPLETE;
            emit dataAvailable(md5, uuid, readArray);
        }
    }
    else
    {
        if(data.size() <= 0)
        {
           qDebug() << "[RemoteRenderer_FileWrapper] ERROR: empty data string received!";
           _status = DMC_SERVER_CONNECTION_CONTROLLER_UPLOAD_ERROR;
           return;
        }

        QFile outputFile(fullFileName);
        if(!outputFile.open(QIODevice::WriteOnly))
        {
            qDebug() << "[RemoteRenderer_FileWrapper] ERROR: not able to open output file for writing: " << outputFile.error() << ", " << outputFile.errorString();
            _status = DMC_SERVER_CONNECTION_CONTROLLER_UPLOAD_ERROR;
            return;
        }

        outputFile.write(data);
        outputFile.close();

        qDebug() << "[RemoteRenderer_FileWrapper] File available: " << md5;
        _status = DMC_SERVER_CONNECTION_UPLOAD_COMPLETE;
        emit dataAvailable(md5, uuid, ((_fileType == DMHelper::FileType_Video) ? QByteArray() : data));
    }
}
