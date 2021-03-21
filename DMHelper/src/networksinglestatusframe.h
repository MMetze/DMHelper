#ifndef NETWORKSINGLESTATUSFRAME_H
#define NETWORKSINGLESTATUSFRAME_H

#include <QFrame>

namespace Ui {
class NetworkSingleStatusFrame;
}

class QNetworkReply;

class NetworkSingleStatusFrame : public QFrame
{
    Q_OBJECT

public:
    explicit NetworkSingleStatusFrame(QNetworkReply* reply, const QString& fileName, QWidget *parent = nullptr);
    virtual ~NetworkSingleStatusFrame() override;

    QNetworkReply* getReply() const;
    QString getFilename() const;

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private slots:
    void setUploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:
    void setStatusString(const QString& statusString);
    void setStatusString();

    Ui::NetworkSingleStatusFrame *ui;
    QString _filename;
    QString _statusString;
    QNetworkReply* _reply;
    qint64 _bytesSent;
    qint64 _bytesTotal;
};

#endif // NETWORKSINGLESTATUSFRAME_H
