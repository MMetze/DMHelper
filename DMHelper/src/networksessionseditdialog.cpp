#include "networksessionseditdialog.h"
#include "ui_networksessionseditdialog.h"
#include "networksession.h"
#include <QInputDialog>
#include <QDebug>

const int INVALID_REQUEST_ID = -1;

NetworkSessionsEditDialog::NetworkSessionsEditDialog(OptionsContainer& options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkSessionsEditDialog),
    _options(options),
    _networkManager(new DMHNetworkManager(DMHLogon(), this)),
    _logon(),
    _currentRequest(INVALID_REQUEST_ID),
    _requestedSession(),
    _requestedSessionName()
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    populateSessions();

    connect(ui->btnNewSession, &QAbstractButton::clicked, this, &NetworkSessionsEditDialog::newSession);
    connect(ui->btnAddSession, &QAbstractButton::clicked, this, &NetworkSessionsEditDialog::addSession);
    connect(ui->btnRenameSession, &QAbstractButton::clicked, this, &NetworkSessionsEditDialog::renameSession);
    connect(ui->btnNewInvite, &QAbstractButton::clicked, this, &NetworkSessionsEditDialog::newInvite);
    connect(ui->btnCloseInvite, &QAbstractButton::clicked, this, &NetworkSessionsEditDialog::closeInvite);

    connect(_networkManager, &DMHNetworkManager::createSessionComplete, this, &NetworkSessionsEditDialog::createSessionComplete);
    connect(_networkManager, &DMHNetworkManager::isOwnerComplete, this, &NetworkSessionsEditDialog::isOwnerComplete);
    connect(_networkManager, &DMHNetworkManager::renameSessionComplete, this, &NetworkSessionsEditDialog::renameSessionComplete);
    connect(_networkManager, &DMHNetworkManager::renewSessionComplete, this, &NetworkSessionsEditDialog::renewSessionComplete);
    connect(_networkManager, &DMHNetworkManager::closeSessionComplete, this, &NetworkSessionsEditDialog::closeSessionComplete);
    connect(_networkManager, &DMHNetworkManager::sessionGeneralComplete, this, &NetworkSessionsEditDialog::sessionGeneralComplete);
    connect(_networkManager, &DMHNetworkManager::messageError, this, &NetworkSessionsEditDialog::messageError);
    connect(_networkManager, &DMHNetworkManager::requestError, this, &NetworkSessionsEditDialog::requestError);

    _logon = DMHLogon(_options.getURLString(), _options.getUserName(), _options.getUserId(), _options.getPassword(), QString());
    _networkManager->setLogon(_logon);
}

NetworkSessionsEditDialog::~NetworkSessionsEditDialog()
{
    delete ui;
}

void NetworkSessionsEditDialog::newSession()
{
    if(_currentRequest != INVALID_REQUEST_ID)
        return;

    _requestedSessionName = QInputDialog::getText(this, QString("DMHelper Edit Sessions"), QString("Enter the new session's name: "));
    if(_requestedSessionName.isEmpty())
        return;

    emit networkMessage(QString("Creating session: ") + _requestedSessionName);
    _currentRequest = _networkManager->createSession(_requestedSessionName);
    qDebug() << "[NetworkSessionsEditDialog] Creating session: " << _requestedSessionName << ", request: " << _currentRequest;
}

void NetworkSessionsEditDialog::addSession()
{
    if(_currentRequest != INVALID_REQUEST_ID)
        return;

    _requestedSession = QInputDialog::getText(this, QString("DMHelper Edit Sessions"), QString("Enter the Session ID: "));
    if(_requestedSession.isEmpty())
        return;

    if(_options.doesSessionExist(_requestedSession))
    {
        qDebug() << "[NetworkSessionsEditDialog] Session ID already exists: " << _requestedSession;
        emit networkMessage(QString("Session ID already exists: ") + _requestedSession);
    }
    else
    {
        emit networkMessage(QString("Adding session ID: ") + _requestedSession);
        _currentRequest = _networkManager->isSessionOwner(_requestedSession);
        qDebug() << "[NetworkSessionsEditDialog] Adding session ID: " << _requestedSession << ", request: " << _currentRequest;
    }
}

void NetworkSessionsEditDialog::renameSession()
{
    if(_currentRequest != INVALID_REQUEST_ID)
        return;

    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    if((items.count() < 3) || (!items.at(1)))
        return;

    _requestedSession = items.at(1)->text();
    _requestedSessionName = QInputDialog::getText(this, QString("DMHelper Edit Sessions"), QString("Enter the new name for this session: "));
    if((_requestedSession.isEmpty()) || (_requestedSessionName.isEmpty()))
        return;

    emit networkMessage(QString("Renaming session ID ") + _requestedSession + QString(" to: ") + _requestedSessionName);
    _currentRequest = _networkManager->renameSession(_requestedSessionName, _requestedSession);
    qDebug() << "[NetworkSessionsEditDialog] Renaming session ID " << _requestedSession << " to: " << _requestedSessionName << ", request: " << _currentRequest;
}

void NetworkSessionsEditDialog::newInvite()
{
    if(_currentRequest != INVALID_REQUEST_ID)
        return;

    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    if((items.count() < 3) || (!items.at(1)))
        return;

    _requestedSession = items.at(1)->text();
    if(_requestedSession.isEmpty())
        return;

    emit networkMessage(QString("Creating a new invite for session ID ") + _requestedSession + QString(" (also closes any active invites)"));
    _currentRequest = _networkManager->renewSessionInvite(_requestedSession);
    qDebug() << "[NetworkSessionsEditDialog] Creating a new invite for session ID " << _requestedSession << ", request: " << _currentRequest;
}

void NetworkSessionsEditDialog::closeInvite()
{
    if(_currentRequest != INVALID_REQUEST_ID)
        return;

    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    if((items.count() < 3) || (!items.at(1)))
        return;

    _requestedSession = items.at(1)->text();
    if(_requestedSession.isEmpty())
        return;

    emit networkMessage(QString("Closing the active invite for session ID ") + _requestedSession);
    _currentRequest = _networkManager->closeSession(_requestedSession);
    qDebug() << "[NetworkSessionsEditDialog] Closing the active invite for session ID " << _requestedSession << ", request: " << _currentRequest;
}

void NetworkSessionsEditDialog::createSessionComplete(int requestID, const QString& session, const QString& invite)
{
    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkSessionsEditDialog] Session created for request " << requestID << ". Session: " << session << ", Invite: " << invite;
    emit networkMessage(QString("Session created: ") + session + QString(" with invite: ") + invite);

    NetworkSession* networkSession = _options.getSession(session);
    if(networkSession)
    {
        networkSession->setName(_requestedSessionName);
        networkSession->setInvite(invite);
    }
    else
    {
        networkSession = new NetworkSession(session, _requestedSessionName, invite);
        if(!_options.addSession(networkSession))
            delete networkSession;
    }
    populateSessions();

    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkSessionsEditDialog::isOwnerComplete(int requestID, const QString& session, const QString& sessionName, const QString& invite, bool isOwner)
{
    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkSessionsEditDialog] IsOwner query answered with request " << requestID << ". Session: " << session << ", Is Owner: " << isOwner;

    if(isOwner)
    {
        NetworkSession* networkSession = _options.getSession(session);
        if(networkSession)
        {
            networkSession->setID(session);
            networkSession->setName(sessionName);
            networkSession->setInvite(invite);
            emit networkMessage(QString("Session updated: ") + sessionName + QString(" with ID: ") + session + QString(" and invite: ") + invite);
        }
        else
        {
            networkSession = new NetworkSession(session, sessionName, invite);
            if(_options.addSession(networkSession))
                emit networkMessage(QString("Session added: ") + sessionName + QString(" with ID: ") + session + QString(" and invite: ") + invite);
            else
                delete networkSession;
        }
        populateSessions();
    }
    else
    {
        emit networkMessage(QString("Session ID: ") + session + QString(" does not belong to this user and cannot be added"));
    }

    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkSessionsEditDialog::renameSessionComplete(int requestID, const QString& sessionName)
{
    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkSessionsEditDialog] Session renamed with request " << requestID << ". Session: " << _requestedSession << ", New name: " << sessionName;

    emit networkMessage(QString("Session: ") + _requestedSession + QString(" renamed to: ") + sessionName);
    _options.setSessionName(_requestedSession, sessionName);
    populateSessions();

    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkSessionsEditDialog::renewSessionComplete(int requestID, const QString& sessionName, const QString& invite)
{
    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkSessionsEditDialog] Session invite renewed with request " << requestID << ". Session: " << sessionName << ", New invite: " << invite;
    emit networkMessage(QString("New invite created for session: ") + sessionName + QString(", Invite: ") + invite);

    _options.setInvite(_requestedSession, invite);
    populateSessions();

    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkSessionsEditDialog::closeSessionComplete(int requestID, const QString& sessionName)
{
    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkSessionsEditDialog] Session invite closed with request " << requestID << ". Session: " << sessionName;
    emit networkMessage(QString("Invite closed for session: ") + sessionName);

    _options.setInvite(_requestedSession, QString());
    populateSessions();

    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkSessionsEditDialog::sessionGeneralComplete(int requestID)
{
    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkSessionsEditDialog] General session response received for request " << requestID;
    emit networkMessage(QString("Received general session response."));
    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkSessionsEditDialog::messageError(int requestID, const QString& errorString)
{
    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkSessionsEditDialog] Message error received for request " << requestID << ": " << errorString;
    emit networkMessage(QString("Error received from network request: ") + errorString);
    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkSessionsEditDialog::requestError(int requestID)
{
    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkSessionsEditDialog] Request error received for request " << requestID;
    emit networkMessage(QString("Unexpected error received from network request."));
    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkSessionsEditDialog::populateSessions()
{
    ui->tableWidget->clear();

    QStringList sessionIds = _options.getSessionIds();
    ui->tableWidget->setRowCount(sessionIds.count());
    for(int i = 0; i < sessionIds.count(); ++i)
    {
        NetworkSession* networkSession = _options.getSession(sessionIds.at(i));
        if(networkSession)
        {
            QTableWidgetItem* nameItem = new QTableWidgetItem(networkSession->getName());
            nameItem->setToolTip(nameItem->text());
            ui->tableWidget->setItem(i, 0, nameItem);

            QTableWidgetItem* sessionItem = new QTableWidgetItem(networkSession->getID());
            nameItem->setToolTip(sessionItem->text());
            ui->tableWidget->setItem(i, 1, sessionItem);

            QTableWidgetItem* inviteItem = new QTableWidgetItem(networkSession->getInvite());
            nameItem->setToolTip(inviteItem->text());
            ui->tableWidget->setItem(i, 2, inviteItem);
        }
    }
}


