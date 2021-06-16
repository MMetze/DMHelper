#ifndef NETWORKSTATUSFRAME_H
#define NETWORKSTATUSFRAME_H

#include <QFrame>

namespace Ui {
class NetworkStatusFrame;
}

class NetworkPlayerFrame;
class OptionsContainer;
class NetworkSession;
class QNetworkReply;
class QVBoxLayout;
class QLayoutItem;

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

    void currentSessionChanged(NetworkSession* session);
    void sessionChanged(NetworkSession* session);

//    void addUser(const QString& username, const QString& screenName);
    void userJoined(const QString& username);

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
    NetworkPlayerFrame* getPlayerFrameById(const QString& id);
    NetworkPlayerFrame* getPlayerFrameByName(const QString& username);
    bool doesSessionExcludeItem(NetworkSession* session, QLayoutItem* item);

    Ui::NetworkStatusFrame *ui;
    QString _statusString;
    QNetworkReply* _currentReply;
    QVBoxLayout* _filesLayout;
    QVBoxLayout* _playersLayout;
};

#endif // NETWORKSTATUSFRAME_H
