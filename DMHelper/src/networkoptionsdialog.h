#ifndef NETWORKOPTIONSDIALOG_H
#define NETWORKOPTIONSDIALOG_H

#include "optionscontainer.h"
#include "dmhnetworkmanager.h"
#include <QDialog>

namespace Ui {
class NetworkOptionsDialog;
}

class NetworkOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkOptionsDialog(OptionsContainer& options, QWidget *parent = nullptr);
    ~NetworkOptionsDialog();

public slots:
    void logMessage(const QString& message);
    void handleMessageError(int requestID, const QString& errorString);

signals:

protected:
    virtual void timerEvent(QTimerEvent *event);

private slots:
    void createUser();
    void sessionSelected(int selection);
    void editSessions();
    void checkLogon();

    void userCreated(int requestID, const QString& username, const QString& userId, const QString& email);
    void userInfoCompleted(int requestID, const QString& username, const QString& userId, const QString& email, const QString& surname, const QString& forename, bool disabled);
    void isOwnerComplete(int requestID, const QString& session, const QString& sessionName, const QString& invite, bool isOwner);
    void sessionMembersComplete(int requestID, const QString& sessionName, const QString& members);
    void sessionGeneralComplete(int requestID);

    void requestError(int requestID);

private:
    void populateSessionsList();

    Ui::NetworkOptionsDialog *ui;

    OptionsContainer& _options;
    DMHNetworkManager* _networkManager;

    int _currentRequest;
    int _memberTimer;

};

#endif // NETWORKOPTIONSDIALOG_H
