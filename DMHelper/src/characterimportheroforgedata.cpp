#include "characterimportheroforgedata.h"
#include <QDomElement>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>



#include <QFile>



CharacterImportHeroForgeData::CharacterImportHeroForgeData(QObject *parent) :
    QObject{parent},
    _imgFrontPath(),
    _imgFront(),
    _imgPerspectivePath(),
    _imgPerspective(),
    _imgTopPath(),
    _imgTop(),
    _imgThumbPath(),
    _imgThumb(),
    _meshPath(),
    _mesh(),
    _metaPath(),
    _meta(),
    _name(),
    _manager(nullptr)
{
}

void CharacterImportHeroForgeData::clear()
{
    _imgFrontPath = QString();
    _imgFront = QImage();
    _imgPerspectivePath = QString();
    _imgPerspective = QImage();
    _imgTopPath = QString();
    _imgTop = QImage();
    _imgThumbPath = QString();
    _imgThumb = QImage();
    _meshPath = QString();
    _mesh = QString();
    _metaPath = QString();
    _meta = QString();
    _name = QString();
}

bool CharacterImportHeroForgeData::readImportData(const QDomElement& element)
{
    clear();

    _imgFrontPath = element.firstChildElement("imgFront").text();
    _imgPerspectivePath = element.firstChildElement("imgPerspective").text();
    _imgTopPath = element.firstChildElement("imgTop").text();
    _imgThumbPath = element.firstChildElement("imgThumb").text();
    _meshPath = element.firstChildElement("mesh").text();
    _metaPath = element.firstChildElement("meta").text();
    _name = element.firstChildElement("name").text();

     return true;
}

void CharacterImportHeroForgeData::getData()
{
    if(!_manager)
    {
        _manager = new QNetworkAccessManager(this);
        connect(_manager, &QNetworkAccessManager::finished, this, &CharacterImportHeroForgeData::urlRequestFinished);
    }

    _manager->get(QNetworkRequest(QUrl(_imgFrontPath)));
    _manager->get(QNetworkRequest(QUrl(_imgPerspectivePath)));
    _manager->get(QNetworkRequest(QUrl(_imgTopPath)));
    _manager->get(QNetworkRequest(QUrl(_imgThumbPath)));
    _manager->get(QNetworkRequest(QUrl(_meshPath)));
    _manager->get(QNetworkRequest(QUrl(_metaPath)));
}

QImage CharacterImportHeroForgeData::getFrontImage() const
{
    return _imgFront;
}

QImage CharacterImportHeroForgeData::getPerspectiveImage() const
{
    return _imgPerspective;
}

QImage CharacterImportHeroForgeData::getTopImage() const
{
    return _imgTop;
}

QImage CharacterImportHeroForgeData::getThumbImage() const
{
    return _imgThumb;
}

QString CharacterImportHeroForgeData::getMesh() const
{
    return _mesh;
}

QString CharacterImportHeroForgeData::getMeta() const
{
    return _meta;
}

QString CharacterImportHeroForgeData::getName() const
{
    return _name;
}

void CharacterImportHeroForgeData::urlRequestFinished(QNetworkReply *reply)
{
    readReply(reply);
    if(isComplete())
        emit dataReady(this);
}

void CharacterImportHeroForgeData::readReply(QNetworkReply* reply)
{
    if(!reply)
        return;

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "[CharacterImportHeroForgeData] ERROR: network image reply not ok: " << reply->error();
        qDebug() << "[CharacterImportHeroForgeData] ERROR: " << reply->errorString();
        return;
    }

    QByteArray bytes = reply->readAll();
    if(bytes.size() <= 0)
    {
        qDebug() << "[CharacterImportHeroForgeData] ERROR: empty response received for request: " << reply;
        return;
    }

    if(reply->url() == _imgFrontPath)
    {
        if(!_imgFront.loadFromData(bytes))
        {
            qDebug() << "[CharacterImportHeroForgeData] Not able to read image from data: " << _imgFrontPath;
            _imgFrontPath = QString();
        }
    }
    else if(reply->url() == _imgPerspectivePath)
    {
        if(!_imgPerspective.loadFromData(bytes))
        {
            qDebug() << "[CharacterImportHeroForgeData] Not able to read image from data: " << _imgPerspectivePath;
            _imgPerspectivePath = QString();
        }
    }
    else if(reply->url() == _imgTopPath)
    {
        if(!_imgTop.loadFromData(bytes))
        {
            qDebug() << "[CharacterImportHeroForgeData] Not able to read image from data: " << _imgTopPath;
            _imgTopPath = QString();
        }
    }
    else if(reply->url() == _imgThumbPath)
    {
        if(!_imgThumb.loadFromData(bytes))
        {
            qDebug() << "[CharacterImportHeroForgeData] Not able to read image from data: " << _imgThumbPath;
            _imgThumbPath = QString();
        }
    }
    else if(reply->url() == _meshPath)
    {
        _mesh = QString(bytes);
        QFile meshFile(getName() + QString("_mesh.txt"));
        if(meshFile.open(QIODevice::WriteOnly))
        {
            meshFile.write(bytes);
            meshFile.close();
        }

    }
    else if(reply->url() == _metaPath)
        _meta = QString(bytes);
}

bool CharacterImportHeroForgeData::isComplete()
{
    return(((_imgFrontPath.isEmpty()) || (!_imgFront.isNull())) &&
           ((_imgPerspectivePath.isEmpty()) || (!_imgPerspective.isNull())) &&
           ((_imgTopPath.isEmpty()) || (!_imgTop.isNull())) &&
           ((_imgThumbPath.isEmpty()) || (!_imgThumb.isNull())) &&
           ((_meshPath.isEmpty()) || (!_mesh.isEmpty())) &&
           ((_metaPath.isEmpty()) || (!_meta.isEmpty())));
}
