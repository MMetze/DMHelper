#include "updatechecker.h"
#include "presentupdatedialog.h"
#include "dmversion.h"
#include <QUrlQuery>
#include <QDomDocument>
#include <QMessageBox>
#include <QDebug>

const int UPDATECHECKER_PERIOD_RELEASE = 7;
const int UPDATECHECKER_PERIOD_DEBUG = 1;

UpdateChecker::UpdateChecker(OptionsContainer& options, bool silentUpdate, bool selfDestruct, QObject *parent) :
    QObject(parent),
    _manager(nullptr),
    _selfDestruct(selfDestruct),
    _silentUpdate(silentUpdate),
    _options(options)
{
}

void UpdateChecker::checkForUpdates()
{
    if(!runUpdateCheck())
    {
        if(_selfDestruct)
            deleteLater();
    }
}

void UpdateChecker::requestFinished(QNetworkReply *reply)
{
    if(!reply)
    {
        if(!_silentUpdate)
            QMessageBox::critical(nullptr,
                                  QString("DM Helper Update"),
                                  QString("An unexpected and unknown error was encountered trying to check for updates!"));
        qDebug() << "[UpdateChecker] ERROR identified in reply, unexpected null pointer reply received!";
        return;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        if(!_silentUpdate)
        {
            if(reply->error() == QNetworkReply::HostNotFoundError)
            {
                QMessageBox::critical(nullptr,
                                      QString("DM Helper Update"),
                                      QString("A network error was encountered trying to check for updates. It was not possible to reach the server!"));
            }
            else
            {
                QMessageBox::critical(nullptr,
                                      QString("DM Helper Update"),
                                      QString("A network error was encountered trying to check for updates:") + QChar::LineFeed + QChar::LineFeed + reply->errorString());
            }
        }
        qDebug() << "[UpdateChecker] ERROR identified in network reply: " << QString::number(reply->error()) << ", Error string " << reply->errorString();
        return;
    }

    QByteArray bytes = reply->readAll();
    qDebug() << "[UpdateChecker] Request received; payload " << bytes.size() << " bytes";
    //qDebug() << "[UpdateChecker] Payload contents: " << QString(bytes.left(2000));

    _options.setLastUpdateDate(QDate::currentDate());

    if((!handleReplyPayload(bytes)) && (!_silentUpdate))
        QMessageBox::information(nullptr,
                                 QString("DM Helper Update"),
                                 QString("Your DM Helper is up to date!"));

    reply->deleteLater();

    if(_selfDestruct)
        deleteLater();
}

bool UpdateChecker::runUpdateCheck()
{
    QDate lastUpdate = _options.getLastUpdateCheck();

    if(_silentUpdate)
    {
        if(!_options.isUpdatesEnabled())
        {
            qDebug() << "[UpdateChecker] Checking for updates disabled, not checking.";
            return false;
        }

        const int UPDATECHECKER_PERIOD = (DMHelper::DMHELPER_ENGINEERING_VERSION > 0) ? UPDATECHECKER_PERIOD_DEBUG : UPDATECHECKER_PERIOD_RELEASE;

        if((lastUpdate.isValid()) && (lastUpdate.daysTo(QDate::currentDate()) < UPDATECHECKER_PERIOD))
        {
             qDebug() << "[UpdateChecker] Last check less than " << UPDATECHECKER_PERIOD << " days ago, not checking.";
             return false;
        }
    }

    _manager = new QNetworkAccessManager(this);
    connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));

    //QUrl serviceUrl = QUrl("https://www.dm-helper.net/check_version/check_version.php");
    QUrl serviceUrl = QUrl("https://update.dm-helper.com/");
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    QString versionString = QString("%1.%2.%3").arg(DMHelper::DMHELPER_MAJOR_VERSION)
                                               .arg(DMHelper::DMHELPER_MINOR_VERSION)
                                               .arg(DMHelper::DMHELPER_ENGINEERING_VERSION);

    if(DMHelper::DMHELPER_ENGINEERING_VERSION > 0)
    {
        postData.addQueryItem("debug", QString("1"));
    }

#ifdef QT_DEBUG
    QUuid debugUuid(0xffffffff, 0xffff, 0xffff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
    postData.addQueryItem("UUID", debugUuid.toString());
#else
    postData.addQueryItem("UUID", _options.getInstanceUuid().toString());
#endif
    postData.addQueryItem("version", versionString);

    qDebug() << "[UpdateChecker] Checking for updates from version " << versionString;

    qDebug() << "[UpdateChecker] Uploaded payload. Request: " << postData.toString(QUrl::FullyEncoded).toUtf8();

    _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

    return true;
}

bool UpdateChecker::handleReplyPayload(const QByteArray& data)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if(!doc.setContent(data, &errorMsg, &errorLine, &errorColumn))
    {
        qDebug() << "[UpdateChecker] ERROR identified reading data: unable to parse network reply XML at line " << errorLine << ", column " << errorColumn << ": " << errorMsg;
        qDebug() << "[UpdateChecker] Data: " << data;
        return false;
    }

    QDomElement root = doc.documentElement();
    if(root.isNull())
    {
        qDebug() << "[UpdateChecker] ERROR identified reading data: unable to find root element: " << doc.toString();
        return false;
    }

    QDomElement updateRoot;
    QString updateVersion;
    QString updateNotes;

    if(!readValue(updateRoot, root, QString("update"), doc))
        return false;

    if(!updateRoot.hasChildNodes())
    {
        qDebug() << "[UpdateChecker] Current version is the latest version, no updates found.";
        return false;
    }

    if(!parseUpdateChain(updateRoot, updateVersion, updateNotes, doc))
    {
        qDebug() << "[UpdateChecker] ERROR found parsing update payload, no updates found: " << doc.toString();
        return false;
    }

    PresentUpdateDialog updateDlg(updateVersion, updateNotes);
    updateDlg.exec();

    emit newVersionFound();

    return true;
}

bool UpdateChecker::parseUpdateChain(const QDomElement& parentElement, QString& updateVersion, QString& updateNotes, const QDomDocument& doc)
{
    if(!parentElement.hasChildNodes())
        return false;

    QDomElement updateElement;

    if(!readValue(updateElement, parentElement, QString("update"), doc))
        return false;

    if(updateElement.hasChildNodes())
    {
        return parseUpdateChain(updateElement, updateVersion, updateNotes, doc);
    }
    else
    {
        QDomElement versionElement;
        QDomElement notesElement;
        if((!readValue(versionElement, parentElement, QString("version"), doc)) ||
           (!readValue(notesElement, parentElement, QString("notes"), doc)))
            return false;

        updateVersion = versionElement.text();
        updateNotes = notesElement.text();
        return true;
    }
}


bool UpdateChecker::readValue(QDomElement& output, const QDomElement& parentElement, const QString& tagName, const QDomDocument& doc)
{
    output = parentElement.firstChildElement(tagName);
    if(output.isNull())
    {
        qDebug() << "[UpdateChecker] ERROR identified reading data: unable to find '" << tagName << "' element: " << doc.toString();
        return false;
    }
    else
    {
        return true;
    }
}
