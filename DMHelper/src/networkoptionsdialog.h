#ifndef NETWORKOPTIONSDIALOG_H
#define NETWORKOPTIONSDIALOG_H

#include "optionscontainer.h"
#include "dmhnetworkmanager.h"
#include <QDialog>

namespace Ui {
class NetworkOptionsDialog;
}

class NetworkPlayerFrame;
class QListWidgetItem;
class QDomElement;
class QVBoxLayout;
class QLayoutItem;

class NetworkOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkOptionsDialog(OptionsContainer& options, QWidget *parent = nullptr);
    ~NetworkOptionsDialog();

public slots:
    void logMessage(const QString& message);
    void handleMessageError(int requestID, const QString& errorString);
    void currentSessionChanged(NetworkSession* session);
    void sessionChanged(NetworkSession* session);

signals:
    //void addSessionUser(const QString& id, const QString& username, const QString& screenName);
    void updateSessionMembers(const QDomElement& rootElement);

protected:
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

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
    void updatePlayersList();
//    QListWidgetItem* playerExists(const QString& playerId);

    NetworkPlayerFrame* getPlayerFrameById(const QString& id);
    NetworkPlayerFrame* getPlayerFrameByName(const QString& username);
    bool doesSessionExcludeItem(NetworkSession* session, QLayoutItem* item);
    void removePlayerFrame(int index);

    Ui::NetworkOptionsDialog *ui;

    OptionsContainer& _options;
    DMHNetworkManager* _networkManager;
    QVBoxLayout* _playersLayout;

    int _currentRequest;
    int _memberTimer;

};

#endif // NETWORKOPTIONSDIALOG_H
