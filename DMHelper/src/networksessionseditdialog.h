#ifndef NETWORKSESSIONSEDITDIALOG_H
#define NETWORKSESSIONSEDITDIALOG_H

#include <QDialog>
#include "optionscontainer.h"
#include "dmhnetworkmanager.h"
#include "dmhlogon.h"

namespace Ui {
class NetworkSessionsEditDialog;
}

class NetworkSessionsEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkSessionsEditDialog(OptionsContainer& options, QWidget *parent = nullptr);
    ~NetworkSessionsEditDialog();

signals:
    void networkMessage(const QString& message);

private slots:
    void newSession();
    void addSession();
    void renameSession();
    void newInvite();
    void closeInvite();

    void createSessionComplete(int requestID, const QString& session, const QString& invite);
    void isOwnerComplete(int requestID, const QString& session, const QString& sessionName, const QString& invite, bool isOwner);
    void renameSessionComplete(int requestID, const QString& sessionName);
    void renewSessionComplete(int requestID, const QString& sessionName, const QString& invite);
    void closeSessionComplete(int requestID, const QString& sessionName);
    void sessionGeneralComplete(int requestID);

    void messageError(int requestID, const QString& errorString);
    void requestError(int requestID);

private:
    void populateSessions();

    Ui::NetworkSessionsEditDialog *ui;

    OptionsContainer& _options;
    DMHNetworkManager* _networkManager;
    DMHLogon _logon;
    int _currentRequest;
    QString _requestedSession;
    QString _requestedSessionName;
};

#endif // NETWORKSESSIONSEDITDIALOG_H
