#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>

namespace Ui {
class MainWindow;
}

class OptionsContainer;
class DMHNetworkManager;
class DMHPayload;
class DMHNetworkObserver;
class QMediaPlayer;
class QMediaPlaylist;
class QCloseEvent;
class QResizeEvent;
class BattleDialogModel;
class BattleDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data);
//    void uploadComplete(int requestID, const QString& fileMD5);
    void payloadReceived(const DMHPayload& payload, const QString& timestamp);

protected:
  void closeEvent(QCloseEvent *event);
  void resizeEvent(QResizeEvent *event);

private slots:
    void muteToggled(bool checked);
    void enableAudio(bool enabled);
    void openOptions();

private:
    void loadBattle();

    Ui::MainWindow *ui;

    OptionsContainer* _settings;

    DMHNetworkManager* _networkManager;
    DMHNetworkObserver* _networkObserver;
    QString _imageMD5client;
    QString _audioMD5client;
//    QString _imageMD5server;
//    QString _audioMD5server;
    int _currentImageRequest;
    int _currentAudioRequest;
    QMediaPlayer* _player;
    QMediaPlaylist* _playlist;
    QPixmap _pmp;

    BattleDialog* _battleDlg;
    BattleDialogModel* _battleDlgModel;
};

#endif // MAINWINDOW_H
