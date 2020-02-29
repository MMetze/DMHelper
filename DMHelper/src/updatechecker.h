#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include "optionscontainer.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUuid>

class QDomDocument;
class QDomElement;

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(OptionsContainer& options, bool silentUpdate = false, bool selfDestruct = false, QObject *parent = nullptr);

public slots:
    void checkForUpdates();

signals:
    void newVersionFound();

protected slots:
    void requestFinished(QNetworkReply *reply);

protected:
    bool runUpdateCheck();
    bool handleReplyPayload(const QByteArray& data);
    bool parseUpdateChain(const QDomElement& parentElement, QString& updateVersion, QString& updateNotes, const QDomDocument& doc);
    bool readValue(QDomElement& output, const QDomElement& parentElement, const QString& tagName, const QDomDocument& doc);

    QNetworkAccessManager* _manager;
    bool _selfDestruct;
    bool _silentUpdate;

    OptionsContainer& _options;
};

#endif // UPDATECHECKER_H
