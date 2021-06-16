#include "networkplayerframe.h"
#include "ui_networkplayerframe.h"
#include "networkplayer.h"
#include <QMenu>

NetworkPlayerFrame::NetworkPlayerFrame(NetworkPlayer* player, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::NetworkPlayerFrame),
    _player(player),
    _status(0)
{
    ui->setupUi(this);
    updateFrame();
    ui->btnStatus->setIcon((QIcon(QPixmap(":/img/data/icon_contentnpc.png"))));
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
    return _player->getID();
}

QString NetworkPlayerFrame::getUserName() const
{
    return _player->getUserName();
}

QString NetworkPlayerFrame::getScreenName() const
{
    return _player->getScreenName();
}

int NetworkPlayerFrame::getStatus() const
{
    return _status;
}

void NetworkPlayerFrame::setPlayer(NetworkPlayer* player)
{
    _player = player;
    updateFrame();
}

void NetworkPlayerFrame::setConnected(bool connected)
{
    ui->lblStatus->setPixmap(connected ? QPixmap(":/img/data/icon_network_ok.png") : QPixmap(":/img/data/icon_network_off.png"));
}

void NetworkPlayerFrame::updateFrame()
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

void NetworkPlayerFrame::statusClicked()
{
    QMenu statusMenu(this);

    if(_status < 1)
    {
        QAction* acceptAction = statusMenu.addAction(QIcon(QPixmap(":/img/data/icon_contentcharacter.png")), QString("Accept"));
        connect(acceptAction, &QAction::triggered, this, &NetworkPlayerFrame::handleAccept);
        QAction* rejectAction = statusMenu.addAction(QIcon(QPixmap(":/img/data/icon_remove.png")), QString("Reject"));
        connect(rejectAction, &QAction::triggered, this, &NetworkPlayerFrame::handleReject);
    }
    else
    {
        QAction* removeAction = statusMenu.addAction(QIcon(QPixmap(":/img/data/icon_remove.png")), QString("Remove"));
        connect(removeAction, &QAction::triggered, this, &NetworkPlayerFrame::handleReject);
    }

    statusMenu.exec(ui->btnStatus->mapToGlobal(QPoint(0,ui->btnStatus->height())));
}

void NetworkPlayerFrame::handleAccept()
{
    _status = 1;
    ui->btnStatus->setIcon((QIcon(QPixmap(":/img/data/icon_contentcharacter.png"))));
}

void NetworkPlayerFrame::handleReject()
{
    _status = 0;
    ui->btnStatus->setIcon((QIcon(QPixmap(":/img/data/icon_remove.png"))));
}
