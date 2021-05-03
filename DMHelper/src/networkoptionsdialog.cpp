#include "networkoptionsdialog.h"
#include "networksessionseditdialog.h"
#include "dmhlogon.h"
#include "connectionuserdialog.h"
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

    ui->edtURL->setText(options.getURLString());
    ui->edtUserName->setText(options.getUserName());
    ui->edtPassword->setText(options.getPassword());
    ui->chkSavePassword->setChecked(options.getSavePassword());
    //ui->edtSessionID
    //ui->edtInviteID

    populateSessionsList();

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

    checkLogon();
}

NetworkOptionsDialog::~NetworkOptionsDialog()
{
    if(_memberTimer != INVALID_TIMER_ID)
        killTimer(_memberTimer);

    delete ui;
}

void NetworkOptionsDialog::timerEvent(QTimerEvent *event)
{
    if((!event) || (event->timerId() != _memberTimer))
        return;

    if(_currentRequest != INVALID_REQUEST_ID)
        return;

    _currentRequest = _networkManager->getSessionMembers(ui->cmbSession->currentData().toString());
}

void NetworkOptionsDialog::createUser()
{
    ConnectionUserDialog dlg;
    QScreen* primary = QGuiApplication::primaryScreen();
    if(primary)
        dlg.resize(primary->availableSize().width() / 4, primary->availableSize().height() / 3);

    if(dlg.exec() == QDialog::Accepted)
    {
        if(dlg.doesPasswordMatch())
        {
            _networkManager->setLogon(DMHLogon(_options.getURLString(), QString(), QString(), QString(), QString()));
            _networkManager->createUser(dlg.getUsername(), dlg.getPassword(), dlg.getEmail(), dlg.getScreenName());
        }
    }
}

void NetworkOptionsDialog::sessionSelected(int selection)
{
    if((selection < 0) || (selection >= ui->cmbSession->count()))
        return;

    QString newSession = ui->cmbSession->itemData(selection).toString();
    if((newSession.isEmpty()) || (!_options.doesSessionExist(newSession)))
        return;

    ui->edtInviteID->setText(_options.getSessionInvite(newSession));
   _options.setCurrentSession(newSession);

   checkLogon();
}

void NetworkOptionsDialog::editSessions()
{
    NetworkSessionsEditDialog editDlg(_options, this);
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
        if(_options.getUserId().isEmpty())
            _currentRequest = _networkManager->getUserInfo();
        else
            userInfoCompleted(INVALID_REQUEST_ID, _options.getUserName(), _options.getUserId(), QString(), QString(), QString(), false);
    }
}

void NetworkOptionsDialog::userCreated(int requestID, const QString& username, const QString& userId, const QString& email)
{
    Q_UNUSED(requestID);
    Q_UNUSED(email);

    ui->edtUserName->setText(username);
    ui->edtPassword->setText(QString());

    _options.setUserId(userId);
}

void NetworkOptionsDialog::userInfoCompleted(int requestID, const QString& username, const QString& userId, const QString& email, const QString& surname, const QString& forename, bool disabled)
{
    if(_currentRequest != requestID)
        return;

    DMHLogon logon = _networkManager->getLogon();
    if(username != logon.getUserName())
        return;

    qDebug() << "[NetworkOptionsDialog] User Info query answered with request " << requestID << ". User: " << username << ", User ID: " << userId << ", email: " << email << ", surname: " << surname << ", forename: " << forename << ", disabled: " << disabled;

    logon.setUserId(userId);
    _networkManager->setLogon(logon);
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
    if((isOwner) && (_memberTimer == INVALID_TIMER_ID))
        _memberTimer = startTimer(500);

    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkOptionsDialog::sessionMembersComplete(int requestID, const QString& sessionName, const QString& members)
{
    Q_UNUSED(sessionName);

    if(_currentRequest != requestID)
        return;

    ui->listPlayers->clear();

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

    //<node><username>m.metze</username><surname>Matthias</surname><forename>Metze</forename></node>
    QDomElement rootElement = doc.documentElement();
    QDomElement memberElement = rootElement.firstChildElement("node");
    while(!memberElement.isNull())
    {
        QDomElement usernameElement = memberElement.firstChildElement("username");
        if(!usernameElement.isNull())
            ui->listPlayers->addItem(usernameElement.text());

        QDomElement surnameElement = memberElement.firstChildElement("surname");
        QDomElement forenameElement = memberElement.firstChildElement("forename");

        memberElement = memberElement.nextSiblingElement("node");
    }

    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkOptionsDialog::requestError(int requestID)
{
    if(_currentRequest != requestID)
        return;

    qDebug() << "[NetworkOptionsDialog] Error received for request " << requestID;
    _currentRequest = INVALID_REQUEST_ID;
}

void NetworkOptionsDialog::populateSessionsList()
{
    ui->cmbSession->clear();

    QString currentSession = _options.getCurrentSession();

    QStringList sessions = _options.getSessions();
    for(QString session : sessions)
        ui->cmbSession->addItem(_options.getSessionName(session), session);

    if((!currentSession.isEmpty()) && (sessions.contains(currentSession)))
    {
        int index = sessions.indexOf(currentSession);
        if((index < 0) || (index >= ui->cmbSession->count()))
        {
            ui->cmbSession->setCurrentIndex(index);
            ui->edtInviteID->setText(_options.getSessionInvite(currentSession));
        }
    }
    else if(ui->cmbSession->count() > 0)
    {
        ui->cmbSession->setCurrentIndex(0);
        _options.setCurrentSession(ui->cmbSession->currentData().toString());
        ui->edtInviteID->setText(_options.getSessionInvite(_options.getCurrentSession()));
    }
}
