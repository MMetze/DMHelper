#include "networksinglestatusframe.h"
#include "ui_networksinglestatusframe.h"
#include <QNetworkReply>

NetworkSingleStatusFrame::NetworkSingleStatusFrame(QNetworkReply* reply, const QString& fileName, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::NetworkSingleStatusFrame),
    _filename(fileName),
    _statusString(QString("Uploading ") + fileName),
    _reply(reply)
{
    ui->setupUi(this);
    ui->lblStatus->setToolTip(_statusString);
    setStatusString();

    if(_reply)
    {
        connect(reply, &QNetworkReply::uploadProgress, this, &NetworkSingleStatusFrame::setUploadProgress);
        connect(reply, &QNetworkReply::finished, this, &NetworkSingleStatusFrame::deleteLater);
    }
}

NetworkSingleStatusFrame::~NetworkSingleStatusFrame()
{
    delete ui;
}

QNetworkReply* NetworkSingleStatusFrame::getReply() const
{
    return _reply;
}

QString NetworkSingleStatusFrame::getFilename() const
{
    return _filename;
}

void NetworkSingleStatusFrame::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    setStatusString();
}

void NetworkSingleStatusFrame::setUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    ui->progressBar->setMaximum(bytesTotal);
    ui->progressBar->setValue(bytesSent);
}

void NetworkSingleStatusFrame::setStatusString()
{
    ui->lblStatus->setText(ui->lblStatus->fontMetrics().elidedText(_statusString, Qt::ElideRight, ui->lblStatus->width()));
}
