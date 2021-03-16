#include "networkstatusframe.h"
#include "ui_networkstatusframe.h"
#include "networksinglestatusframe.h"
#include "optionscontainer.h"
#include <QNetworkReply>
#include <QVBoxLayout>

NetworkStatusFrame::NetworkStatusFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::NetworkStatusFrame),
    _statusString(),
    _currentReply(nullptr),
    _statusLayout(nullptr)
{
    ui->setupUi(this);
    setStatusString();

    connect(ui->btnExpand, &QAbstractButton::clicked, this, &NetworkStatusFrame::setExpanded);
    setExpanded(false);

    _statusLayout = new QVBoxLayout;
    _statusLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->scrollAreaWidgetContents->setLayout(_statusLayout);
}

NetworkStatusFrame::~NetworkStatusFrame()
{
    delete ui;
}

void NetworkStatusFrame::setOptionsContainer(OptionsContainer* optionsContainer)
{
    if(!optionsContainer)
        return;

    networkEnabled(optionsContainer->getNetworkEnabled());
    connect(optionsContainer, &OptionsContainer::networkEnabledChanged, this, &NetworkStatusFrame::networkEnabled);
}

void NetworkStatusFrame::setNetworkSuccess()
{
    if(!_currentReply)
    {
        ui->lblConnection->setPixmap(QPixmap(":/img/data/icon_network_ok.png"));
        setStatusString(QString("Connected"));
    }
}

void NetworkStatusFrame::setNetworkError(int error)
{
    Q_UNUSED(error);

    ui->lblConnection->setPixmap(QPixmap(":/img/data/icon_network_error.png"));
    setStatusString(QString("Connection Error"));
}

void NetworkStatusFrame::uploadStarted(int requestID, QNetworkReply* reply, const QString& filename)
{
    Q_UNUSED(requestID);

    if(!reply)
        return;

    NetworkSingleStatusFrame* newFrame = new NetworkSingleStatusFrame(reply, filename);
    _statusLayout->addWidget(newFrame);

    if(!_currentReply)
        setReply(reply, filename);
}

void NetworkStatusFrame::uploadComplete()
{
    setUploadProgress(0, 100);

    NetworkSingleStatusFrame* nextStatus = nullptr;
    int i = 0;
    while((!nextStatus) && (i < _statusLayout->count()))
    {
        if((_statusLayout->itemAt(i)) && (_statusLayout->itemAt(i)->widget()))
        {
            nextStatus = dynamic_cast<NetworkSingleStatusFrame*>(_statusLayout->itemAt(i)->widget());
            if(nextStatus->getReply() == _currentReply)
                nextStatus = nullptr;
        }
        ++i;
    }

    if(nextStatus)
        setReply(nextStatus->getReply(), nextStatus->getFilename());
    else
        setReply(nullptr, QString());
}

void NetworkStatusFrame::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    setStatusString();
}

void NetworkStatusFrame::networkEnabled(bool enabled)
{
    ui->lblConnection->setPixmap(QPixmap(":/img/data/icon_network_off.png"));
    setStatusString(QString("Not Connected"));
    setVisible(enabled);
}

void NetworkStatusFrame::setUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    ui->progressBar->setMaximum(bytesTotal);
    ui->progressBar->setValue(bytesSent);
}

void NetworkStatusFrame::setExpanded(bool expanded)
{
    QPixmap expandPmp(":/img/data/icon_downarrow.png");
    if(!expanded)
        expandPmp = expandPmp.transformed(QTransform().rotate(180));
    ui->btnExpand->setIcon(QIcon(expandPmp));

    ui->progressBar->setVisible(!expanded);
    ui->scrollArea->setVisible(expanded);
    setMinimumHeight(expanded ? 300 : 100);
    setMaximumHeight(expanded ? 300 : 100);
}

void NetworkStatusFrame::setStatusString(const QString& statusString)
{
    _statusString = statusString;
    ui->lblStatus->setToolTip(_statusString);
    setStatusString();
}

void NetworkStatusFrame::setStatusString()
{
    ui->lblStatus->setText(ui->lblStatus->fontMetrics().elidedText(_statusString, Qt::ElideRight, ui->lblStatus->width()));
}

void NetworkStatusFrame::setReply(QNetworkReply* reply, const QString& filename)
{
    if(reply)
    {
        setStatusString(QString("Uploading ") + filename + QString("..."));
        connect(reply, &QNetworkReply::uploadProgress, this, &NetworkStatusFrame::setUploadProgress);
        connect(reply, &QNetworkReply::destroyed, this, &NetworkStatusFrame::uploadComplete);
    }
    else
    {
        setNetworkSuccess();
    }

    _currentReply = reply;
}
