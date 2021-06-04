#ifndef NETWORKSTATUSFRAME_H
#define NETWORKSTATUSFRAME_H

#include <QFrame>

namespace Ui {
class NetworkStatusFrame;
}

class OptionsContainer;
class QNetworkReply;
class QVBoxLayout;

class NetworkStatusFrame : public QFrame
{
    Q_OBJECT

public:
    explicit NetworkStatusFrame(QWidget *parent = nullptr);
    virtual ~NetworkStatusFrame() override;

    void setOptionsContainer(OptionsContainer* optionsContainer);

public slots:
    void setNetworkStatus(bool success);
    void setNetworkSuccess();
    void setNetworkError(int error);
    void uploadStarted(int requestID, QNetworkReply* reply, const QString& filename);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private slots:
    void networkEnabled(bool enabled);
    void setUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void setExpanded(bool expanded);
    void uploadComplete();

private:
    void setStatusString(const QString& statusString);
    void setStatusString();
    void setReply(QNetworkReply* reply, const QString& filename);

    Ui::NetworkStatusFrame *ui;
    QString _statusString;
    QNetworkReply* _currentReply;
    QVBoxLayout* _filesLayout;
    QVBoxLayout* _playersLayout;
};

#endif // NETWORKSTATUSFRAME_H
