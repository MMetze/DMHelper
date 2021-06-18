#include "networkplayerframe.h"
#include "ui_networkplayerframe.h"
#include "networkplayer.h"
#include <QMenu>

NetworkPlayerFrame::NetworkPlayerFrame(NetworkPlayer* player, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::NetworkPlayerFrame),
    _player(nullptr),
    _status(0)
{
    ui->setupUi(this);
    setPlayer(player);
    updateFrame();
    connect(ui->btnStatus, &QAbstractButton::clicked, this, &NetworkPlayerFrame::statusClicked);
}

NetworkPlayerFrame::~NetworkPlayerFrame()
{
    delete ui;
}

NetworkPlayer* NetworkPlayerFrame::getPlayer() const
{
    return _player;
}

QString NetworkPlayerFrame::getUserId() const
{
    if(_player)
        return _player->getID();
    else
        return QString();
}

QString NetworkPlayerFrame::getUserName() const
{
    if(_player)
        return _player->getUserName();
    else
        return QString();
}

QString NetworkPlayerFrame::getScreenName() const
{
    if(_player)
        return _player->getScreenName();
    else
        return QString();
}

int NetworkPlayerFrame::getStatus() const
{
    return _status;
}

void NetworkPlayerFrame::setPlayer(NetworkPlayer* player)
{
    disconnect(player, nullptr, this, nullptr);
    _player = player;

    ui->btnStatus->setEnabled(_player);

    if(_player)
    {
        connect(player, &NetworkPlayer::userNameChanged, this, &NetworkPlayerFrame::updateFrameName);
        connect(player, &NetworkPlayer::screenNameChanged, this, &NetworkPlayerFrame::updateFrameName);
        connect(player, &NetworkPlayer::lastSeenChanged, this, &NetworkPlayerFrame::updateFrameLastSeen);
        connect(player, &NetworkPlayer::statusChanged, this, &NetworkPlayerFrame::updateFrameStatus);
        updateFrame();
    }
}

void NetworkPlayerFrame::setConnected(bool connected)
{
    if((_player) && (connected))
        _player->setLastSeenNow();
}

void NetworkPlayerFrame::updateFrame()
{
    updateFrameName();
    updateFrameLastSeen();
    updateFrameStatus();
}

void NetworkPlayerFrame::updateFrameName()
{
    if(_player)
    {
        if(_player->getScreenName().isEmpty())
        {
            ui->lblName->setText(_player->getUserName());
            ui->lblName->setToolTip(_player->getUserName());
        }
        else
        {
            ui->lblName->setText(_player->getScreenName());
            ui->lblName->setToolTip(_player->getScreenName() + QString(" (") + _player->getUserName() + QString(")"));
        }
    }
    else
    {
        ui->lblName->setText(QString());
        ui->lblName->setToolTip(QString());
    }
}

void NetworkPlayerFrame::updateFrameLastSeen()
{
    ui->lblStatus->setPixmap(((_player) && (_player->isAvailable())) ? QPixmap(":/img/data/icon_network_ok.png") : QPixmap(":/img/data/icon_network_off.png"));
}

void NetworkPlayerFrame::updateFrameStatus()
{
    if(!_player)
    {
        ui->btnStatus->setIcon((QIcon(QPixmap(":/img/data/icon_contentnpc.png"))));
        return;
    }

    switch(_player->getStatus())
    {
        case NetworkPlayer::Blocked:
            ui->btnStatus->setIcon((QIcon(QPixmap(":/img/data/icon_remove.png"))));
            break;
        case NetworkPlayer::Accepted:
            ui->btnStatus->setIcon((QIcon(QPixmap(":/img/data/icon_contentcharacter.png"))));
            break;
        case NetworkPlayer::Unknown:
        default:
            ui->btnStatus->setIcon((QIcon(QPixmap(":/img/data/icon_contentnpc.png"))));
            break;
    }
}

void NetworkPlayerFrame::statusClicked()
{
    QMenu statusMenu(this);

    QAction* acceptAction = statusMenu.addAction(QIcon(QPixmap(":/img/data/icon_contentcharacter.png")), QString("Accept"));
    connect(acceptAction, &QAction::triggered, this, &NetworkPlayerFrame::handleStatusAccept);
    QAction* removeAction = statusMenu.addAction(QIcon(QPixmap(":/img/data/icon_remove.png")), QString("Remove"));
    connect(removeAction, &QAction::triggered, this, &NetworkPlayerFrame::handleStatusRemove);
    QAction* blockAction = statusMenu.addAction(QIcon(QPixmap(":/img/data/icon_shield.png")), QString("Block"));
    connect(blockAction, &QAction::triggered, this, &NetworkPlayerFrame::handleStatusBlock);

    statusMenu.exec(ui->btnStatus->mapToGlobal(QPoint(0,ui->btnStatus->height())));
}

void NetworkPlayerFrame::handleStatusAccept()
{
    if(_player)
        _player->setStatus(NetworkPlayer::Accepted);
}

void NetworkPlayerFrame::handleStatusRemove()
{
    if(_player)
        _player->setStatus(NetworkPlayer::Unknown);
}

void NetworkPlayerFrame::handleStatusBlock()
{
    if(_player)
        _player->setStatus(NetworkPlayer::Blocked);
}
