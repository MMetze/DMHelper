#include "networkplayerframe.h"
#include "ui_networkplayerframe.h"
#include <QMenu>

NetworkPlayerFrame::NetworkPlayerFrame(const QString& playerName, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::NetworkPlayerFrame),
    _playerName(playerName),
    _status(0)
{
    ui->setupUi(this);
    ui->lblName->setText(playerName);
    ui->btnStatus->setIcon((QIcon(QPixmap(":/img/data/icon_contentnpc.png"))));
    connect(ui->btnStatus, &QAbstractButton::clicked, this, &NetworkPlayerFrame::statusClicked);
}

NetworkPlayerFrame::~NetworkPlayerFrame()
{
    delete ui;
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
