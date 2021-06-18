#ifndef NETWORKPLAYERFRAME_H
#define NETWORKPLAYERFRAME_H

#include <QFrame>

namespace Ui {
class NetworkPlayerFrame;
}

class NetworkPlayer;

class NetworkPlayerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit NetworkPlayerFrame(NetworkPlayer* player, QWidget *parent = nullptr);
    ~NetworkPlayerFrame();

    NetworkPlayer* getPlayer() const;
    QString getUserId() const;
    QString getUserName() const;
    QString getScreenName() const;
    int getStatus() const;

public slots:
    void setPlayer(NetworkPlayer* player);
    void setConnected(bool connected);
    void updateFrame();
    void updateFrameName();
    void updateFrameLastSeen();
    void updateFrameStatus();

private slots:
    void statusClicked();
    void handleStatusAccept();
    void handleStatusRemove();
    void handleStatusBlock();

private:
    Ui::NetworkPlayerFrame *ui;

    NetworkPlayer* _player;
    int _status;
};

#endif // NETWORKPLAYERFRAME_H
