#include "networkoptionsdialog.h"
#include "networksessionseditdialog.h"
#include "dmhlogon.h"
#include "connectionuserdialog.h"
#include "networksession.h"
#include "networkplayer.h"
#include "ui_networkoptionsdialog.h"
#include <QDomDocument>
#include <QScreen>
#include <QDebug>

const int INVALID_REQUEST_ID = -1;
const int INVALID_TIMER_ID = 0;

NetworkOptionsDialog::NetworkOptionsDialog(OptionsContainer& options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkOptionsDialog),
    _options(options),
    _networkManager(new DMHNetworkManager(DMHLogon(), this)),
    _currentRequest(INVALID_REQUEST_ID),
    _memberTimer(INVALID_TIMER_ID)
{
    ui->setupUi(this);
    connect(_networkManager, &DMHNetworkManager::messageError, this, &NetworkOptionsDialog::handleMessageError);

    ui->edtURL->setText(options.getURLString());
    ui->edtUserName->setText(options.getUserName());
    ui->edtPassword->setText(options.getPassword());
    ui->chkSavePassword->setChecked(options.getSavePassword());
    //ui->edtSessionID
    //ui->edtInviteID

    populateSessionsList();
    updatePlayersList();

    /*
    select --> check isowner text shows session name, detailed list shows name and ID
    ... to create/add existing/remove/rename

            create invite --> "renew"
            x to remove invite

    keep simple - one command at a time, each time show waitdlg until done


    add timer, check member list every 1s
    */

    connect(ui->chkSavePassword, &QAbstractButton::clicked, &options, &OptionsContainer::setSavePassword);
    connect(ui->edtURL, &QLineEdit::editingFinished, this, &NetworkOptionsDialog::checkLogon);
    connect(ui->edtUserName, &QLineEdit::editingFinished, this, &NetworkOptionsDialog::checkLogon);
    connect(ui->btnCreateUser, &QAbstractButton::clicked, this, &NetworkOptionsDialog::createUser);
    connect(ui->edtPassword, &QLineEdit::editingFinished, this, &NetworkOptionsDialog::checkLogon);
    connect(ui->cmbSession, qOverload<int>(&QComboBox::currentIndexChanged), this, &NetworkOptionsDialog::sessionSelected);

    connect(ui->btnEditSessions, &QAbstractButton::clicked, this, &NetworkOptionsDialog::editSessions);

    connect(_networkManager, &DMHNetworkManager::createUserComplete, this, &NetworkOptionsDialog::userCreated);
    connect(_networkManager, &DMHNetworkManager::userInfoComplete, this, &NetworkOptionsDialog::userInfoCompleted);
    connect(_networkManager, &DMHNetworkManager::isOwnerComplete, this, &NetworkOptionsDialog::isOwnerComplete);
    connect(_networkManager, &DMHNetworkManager::sessionMembersComplete, this, &NetworkOptionsDialog::sessionMembersComplete);
    connect(_networkManager, &DMHNetworkManager::sessionGeneralComplete, this, &NetworkOptionsDialog::sessionGeneralComplete);
}

NetworkOptionsDialog::~NetworkOptionsDialog()
{
    disconnect(_networkManager, nullptr, this, nullptr);

    if(_memberTimer != INVALID_TIMER_ID)
        killTimer(_memberTimer);

    delete ui;
}

void NetworkOptionsDialog::logMessage(const QString& message)
{
    if(message.isEmpty())
        return;

    ui->txtLog->append(message.trimmed());
}

void NetworkOptionsDialog::handleMessageError(int requestID, const QString& errorString)
{
    if(_currentRequest != requestID)
        return;

    logMessage(QString("Network error encountered for: ") + errorString);
    qDebug() << "[NetworkController] Network error discovered for request " << requestID << ": " << errorString;
    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkOptionsDialog::currentSessionChanged(NetworkSession* session)
{
    ui->edtInviteID->setText(session ? session->getInvite() : QString());
    if(session)
    {
        if(ui->cmbSession->currentData().toString() != session->getID())
            ui->cmbSession->setCurrentText(session->getName());

        updatePlayersList();
        checkLogon();
    }
}

void NetworkOptionsDialog::sessionChanged(NetworkSession* session)
{
    if((!session) || (_options.getCurrentSession() != session))
        return;

    ui->cmbSession->setItemText(ui->cmbSession->currentIndex(), session->getName());
    ui->edtInviteID->setText(session->getInvite());

    updatePlayersList();

    if(ui->cmbSession->currentData().toString() != session->getID())
    {
        ui->cmbSession->setItemData(ui->cmbSession->currentIndex(), session->getID());
        checkLogon();
    }
}

void NetworkOptionsDialog::timerEvent(QTimerEvent *event)
{
    if((!event) || (event->timerId() != _memberTimer))
        return;

    if(_currentRequest != INVALID_REQUEST_ID)
        return;

    _currentRequest = _networkManager->getSessionMembers(ui->cmbSession->currentData().toString());
}

void NetworkOptionsDialog::closeEvent(QCloseEvent *event)
{
    if(_memberTimer != INVALID_TIMER_ID)
    {
        killTimer(_memberTimer);
        _memberTimer = INVALID_TIMER_ID;
    }

    QDialog::closeEvent(event);
}

void NetworkOptionsDialog::showEvent(QShowEvent *event)
{
    checkLogon();
    QDialog::showEvent(event);
}

void NetworkOptionsDialog::createUser()
{
    ConnectionUserDialog dlg;
    QScreen* primary = QGuiApplication::primaryScreen();
    if(primary)
        dlg.resize(primary->availableSize().width() / 4, primary->availableSize().height() / 3);

    if((dlg.exec() != QDialog::Accepted) || (!dlg.doesPasswordMatch()))
        return;

    qDebug() << "[NetworkOptionsDialog] Trying to create user with name: " << dlg.getUsername() << ", Email: " << dlg.getEmail() << ", screen name: " << dlg.getScreenName();
    logMessage(QString("Creating user with username: ") + dlg.getUsername() + QString(" and screen name: ") + dlg.getScreenName());
    ui->edtUserName->setText(QString());
    ui->edtPassword->setText(QString());
    _options.setUserName(QString());
    _options.setPassword(QString());

    _networkManager->setLogon(DMHLogon(_options.getURLString(), QString(), QString(), QString(), QString()));
    _networkManager->createUser(dlg.getUsername(), dlg.getPassword(), dlg.getEmail(), dlg.getScreenName());
}

void NetworkOptionsDialog::sessionSelected(int selection)
{
    if((selection < 0) || (selection >= ui->cmbSession->count()))
        return;

    QString newSession = ui->cmbSession->itemData(selection).toString();
//    if((newSession.isEmpty()) || (!_options.doesSessionExist(newSession)))
//        return;

    _options.setCurrentSessionId(newSession);
}

void NetworkOptionsDialog::editSessions()
{
    NetworkSessionsEditDialog editDlg(_options, this);
    connect(&editDlg, &NetworkSessionsEditDialog::networkMessage, this, &NetworkOptionsDialog::logMessage);
    editDlg.resize(width() * 9 / 10, height() * 9 / 10);
    editDlg.exec();

    populateSessionsList();
    checkLogon();
}

void NetworkOptionsDialog::checkLogon()
{
    if(_currentRequest != INVALID_REQUEST_ID)
        return;

    if(_memberTimer != INVALID_TIMER_ID)
    {
        killTimer(_memberTimer);
        _memberTimer = INVALID_TIMER_ID;
    }

    _options.setURLString(ui->edtURL->text());
    _options.setUserName(ui->edtUserName->text());
    _options.setPassword(ui->edtPassword->text());

    DMHLogon logon(_options.getURLString(), _options.getUserName(), _options.getUserId(), _options.getPassword(), ui->cmbSession->currentData().toString());
    if(logon.isValid())
    {
        _networkManager->setLogon(logon);
        logMessage(QString("Connecting to server: ") + logon.getURLString() + QString(" with user ") + logon.getUserName());
        if(_options.getUserId().isEmpty())
        {
            _currentRequest = _networkManager->getUserInfo();
        }
        else
        {
            userInfoCompleted(INVALID_REQUEST_ID, _options.getUserName(), _options.getUserId(), QString(), QString(), QString(), false);
        }
    }
}

void NetworkOptionsDialog::userCreated(int requestID, const QString& username, const QString& userId, const QString& email)
{
    Q_UNUSED(requestID);
    Q_UNUSED(email);

    qDebug() << "[NetworkOptionsDialog] User creation complete. Request: " << requestID << ". User: " << username << ", User ID: " << userId << ", email: " << email;
    logMessage(QString("User created: ") + username);

    ui->edtUserName->setText(username);
    ui->edtPassword->setText(QString());
    _options.setUserId(userId);
}

void NetworkOptionsDialog::userInfoCompleted(int requestID, const QString& username, const QString& userId, const QString& email, const QString& surname, const QString& forename, bool disabled)
{
    if(_currentRequest != requestID)
        return;

    if(username != _options.getUserName())
        return;

    qDebug() << "[NetworkOptionsDialog] User Info query answered with request " << requestID << ". User: " << username << ", User ID: " << userId << ", email: " << email << ", surname: " << surname << ", forename: " << forename << ", disabled: " << disabled;

    _options.setUserId(userId);

    logMessage(QString("Connected to server. Starting session: ") + ui->cmbSession->currentData().toString());
    _networkManager->setLogon(DMHLogon(_options.getURLString(), _options.getUserName(), _options.getUserId(), _options.getPassword(), QString()));
    _currentRequest = _networkManager->isSessionOwner(ui->cmbSession->currentData().toString());
}

void NetworkOptionsDialog::isOwnerComplete(int requestID, const QString& session, const QString& sessionName, const QString& invite, bool isOwner)
{
    Q_UNUSED(sessionName);
    Q_UNUSED(invite);

    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkOptionsDialog] IsOwner query answered with request " << requestID << ". Session: " << session << ", Is Owner: " << isOwner;

    ui->lblSessionStatus->setPixmap(isOwner ? QPixmap(":/img/data/icon_network_ok.png") : QPixmap(":/img/data/icon_network_error.png"));
    if(isOwner)
    {
        logMessage(QString("Session ") + sessionName + QString(" with ID ") + session + QString(" started."));
        if(_memberTimer == INVALID_TIMER_ID)
            _memberTimer = startTimer(500);
    }
    else
    {
        logMessage(QString("Session ") + sessionName + QString(" with ID ") + session + QString(" does not belong to user ") + _options.getUserName());
    }

    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkOptionsDialog::sessionMembersComplete(int requestID, const QString& sessionName, const QString& members)
{
    Q_UNUSED(sessionName);

    if(_currentRequest != requestID)
        return;

    if(_options.getCurrentSession()->getName() == sessionName)
    {
        QDomDocument doc;
        QString errorMsg;
        int errorLine;
        int errorColumn;
        if(!doc.setContent(members, &errorMsg, &errorLine, &errorColumn))
        {
            qDebug() << "[NetworkOptionsDialog] ERROR identified reading audio data: unable to parse XML at line " << errorLine << ", column " << errorColumn << ": " << errorMsg;
            qDebug() << "[NetworkOptionsDialog] Members String: " << members;
            return;
        }

        /*
        QDomElement rootElement = doc.documentElement();
        QDomElement memberElement = rootElement.firstChildElement("node");
        while(!memberElement.isNull())
        {
            QDomElement idElement = memberElement.firstChildElement("ID");
            QDomElement usernameElement = memberElement.firstChildElement("username");
            QDomElement surnameElement = memberElement.firstChildElement("surname");
            QDomElement forenameElement = memberElement.firstChildElement("forename");

            emit addSessionUser(idElement.text(), usernameElement.text(), surnameElement.text());

            memberElement = memberElement.nextSiblingElement("node");
        }
        */

        qDebug() << "[NetworkOptionsDialog] Members: " << doc.toString();

        emit updateSessionMembers(doc.documentElement());
    }

    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkOptionsDialog::sessionGeneralComplete(int requestID)
{
    if(_currentRequest != requestID)
        return;

    logMessage(QString("Received general session response."));
    qDebug() << "[NetworkOptionsDialog] General session response received for request " << requestID;
    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkOptionsDialog::requestError(int requestID)
{
    if(_currentRequest != requestID)
        return;

    logMessage(QString("Unexpected error received from network request."));
    qDebug() << "[NetworkOptionsDialog] Error received for request " << requestID;
    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkOptionsDialog::populateSessionsList()
{
    ui->cmbSession->clear();

    QStringList sessionIds = _options.getSessionIds();
    for(QString sessionId : sessionIds)
    {
        NetworkSession* session = _options.getSession(sessionId);
        if(session)
            ui->cmbSession->addItem(session->getName(), sessionId);
    }

    if(_options.getCurrentSession())
    {
        int index = ui->cmbSession->findData(_options.getCurrentSession()->getID());
        if((index >= 0) && (index < ui->cmbSession->count()))
        {
            ui->cmbSession->setCurrentIndex(index);
            ui->edtInviteID->setText(_options.getCurrentSession()->getInvite());
            return;
        }
    }

    if(ui->cmbSession->count() > 0)
    {
        ui->cmbSession->setCurrentIndex(0);
        _options.setCurrentSessionId(ui->cmbSession->currentData().toString());
        if(_options.getCurrentSession())
        {
            ui->edtInviteID->setText(_options.getCurrentSession()->getInvite());
            return;
        }
    }

    ui->edtInviteID->setText(QString());
}

void NetworkOptionsDialog::updatePlayersList()
{
    NetworkSession* session = _options.getCurrentSession();
    if(!session)
    {
        ui->listPlayers->clear();
        return;
    }

    QList<NetworkPlayer*> players = session->getPlayers();
    /*if(players.isEmpty())
    {
        ui->listPlayers->clear();
        return;
    }*/

    for(NetworkPlayer* player : players)
    {
        if(player)
        {
            QString playerText = player->getScreenName().isEmpty() ? player->getUserName() : player->getScreenName() + QString(" (") + player->getUserName() + QString(")");
            QListWidgetItem* item = playerExists(player->getID());
            if(item)
            {
                item->setText(playerText);
            }
            else
            {
                item = new QListWidgetItem(playerText);
                item->setData(Qt::UserRole, player->getID());
                ui->listPlayers->addItem(item);
            }
        }
    }

    int i = ui->listPlayers->count() - 1;
    while(i >= 0)
    {
        QListWidgetItem* item = ui->listPlayers->item(i);
        if(item)
        {
            if(!session->playerExistsById(item->data(Qt::UserRole).toString()))
                delete ui->listPlayers->takeItem(i);
        }

        --i;
    }
}

QListWidgetItem* NetworkOptionsDialog::playerExists(const QString& playerId)
{
    for(int i = 0; i < ui->listPlayers->count(); ++i)
    {
        QListWidgetItem* item = ui->listPlayers->item(i);
        if((item) && (item->data(Qt::UserRole).toString() == playerId))
            return item;
    }

    return nullptr;
}
