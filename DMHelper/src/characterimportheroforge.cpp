#include "characterimportheroforge.h"
#include "characterimportheroforgedata.h"
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QDomDocument>
#include <QDebug>

//#define DEBUG_HEROFORGE_IMPORT

CharacterImportHeroForge::CharacterImportHeroForge(QObject *parent) :
    QObject{parent},
    _manager(nullptr),
    _importData()
{
}

void CharacterImportHeroForge::runImport(const QString& token)
{
    if(!_manager)
    {
        _manager = new QNetworkAccessManager(this);
        connect(_manager, &QNetworkAccessManager::finished, this, &CharacterImportHeroForge::urlRequestFinished);
    }

    qDeleteAll(_importData);
    _importData.clear();

#ifdef DEBUG_HEROFORGE_IMPORT
    QUrl serviceUrl = QUrl("https://api.dmhh.net/heroforge?version=3.0&debug=true");
#else
    QUrl serviceUrl = QUrl("https://api.dmhh.net/heroforge?version=3.0");
#endif
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("token", token);

#ifdef DEBUG_HEROFORGE_IMPORT
    qDebug() << "[CharacterImportHeroForge] Posting Heroforge request: " << postData.toString(QUrl::FullyEncoded).toUtf8();
#endif

    _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void CharacterImportHeroForge::urlRequestFinished(QNetworkReply *reply)
{
#ifdef DEBUG_HEROFORGE_IMPORT
    qDebug() << "[CharacterImportHeroForge] Heroforge request completed. Reply: " << reply;
#endif
    handleRequestFinished(reply);
    deleteLater();
}

void CharacterImportHeroForge::handleRequestFinished(QNetworkReply *reply)
{
    if(!reply)
        return;

    if(reply->error() != QNetworkReply::NoError)
    {
        if(reply->error() == QNetworkReply::HostNotFoundError)
        {
            QMessageBox::critical(nullptr,
                                  QString("DMHelper Hero Forge Import Error"),
                                  QString("A network error was encountered trying to find the requested Heroforge account. It was not possible to reach the server!"));
        }
        else
        {
            QMessageBox::critical(nullptr,
                                  QString("DMHelper Hero Forge Import Error"),
                                  QString("A network error was encountered trying to find the requested Heroforge account:") + QChar::LineFeed + QChar::LineFeed + reply->errorString());
        }

        qDebug() << "[CharacterImportHeroForge] ERROR identified in network reply: " << QString::number(reply->error()) << ", Error string " << reply->errorString();
    }
    else
    {
        QByteArray bytes = reply->readAll();
        qDebug() << "[CharacterImportHeroForge] Heroforge Request received; payload " << bytes.size() << " bytes";
#ifdef DEBUG_HEROFORGE_IMPORT
        qDebug() << "[CharacterImportHeroForge] Payload contents: " << QString(bytes.left(2000));
#endif
        parseReply(bytes);
    }

    emit importComplete(_importData);

    reply->deleteLater();
}

bool CharacterImportHeroForge::parseReply(const QByteArray& data)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if(!doc.setContent(data, &errorMsg, &errorLine, &errorColumn))
    {
        qDebug() << "[CharacterImportHeroForge] ERROR identified reading data: unable to parse network reply XML at line " << errorLine << ", column " << errorColumn << ": " << errorMsg;
        qDebug() << "[CharacterImportHeroForge] Data: " << data;
        return false;
    }

    QDomElement root = doc.documentElement();
    if(root.isNull())
    {
        qDebug() << "[CharacterImportHeroForge] ERROR identified reading data: unable to find root element: " << doc.toString();
        return false;
    }

    QDomElement goodsElement = root.firstChildElement(QString("goods"));
    if(goodsElement.isNull())
    {
        qDebug() << "[CharacterImportHeroForge] ERROR identified reading data: unable to find the goods element: " << doc.toString();
        return false;
    }

    QDomElement goodElement = goodsElement.firstChildElement(QString("good"));
    while(!goodElement.isNull())
    {
        CharacterImportHeroForgeData* newData = new CharacterImportHeroForgeData;
        newData->readImportData(goodElement);
        qDebug() << "[CharacterImportHeroForge] Found Hero Forge data for " << newData->getName();
        _importData.append(newData);

        goodElement = goodElement.nextSiblingElement(QString("good"));
    }

    return true;
}
