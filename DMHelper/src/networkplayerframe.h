#ifndef NETWORKPLAYERFRAME_H
#define NETWORKPLAYERFRAME_H

#include <QFrame>

namespace Ui {
class NetworkPlayerFrame;
}

class NetworkPlayerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit NetworkPlayerFrame(const QString& playerName, QWidget *parent = nullptr);
    ~NetworkPlayerFrame();

private slots:
    void statusClicked();
    void handleAccept();
    void handleReject();

private:
    Ui::NetworkPlayerFrame *ui;

    QString _playerName;
    int _status;
};

#endif // NETWORKPLAYERFRAME_H
