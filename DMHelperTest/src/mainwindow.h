#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class DMHNetworkManager;
class DMHPayload;
class DMHNetworkObserver;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data);
    void uploadComplete(int requestID, const QString& fileMD5);
    void existsComplete(int requestID, const QString& fileMD5, const QString& filename, bool exists);
    void payloadReceived(const DMHPayload& payload, const QString& timestamp);

private slots:
    void startImageUpload();
    void startAudioUpload();
    void clearImage();
    void clearAudio();
    void runFileDownload();
    void runFileExists();

    void setPayloadContents(const QByteArray& data);
    void setXMLRequest(const QByteArray& data);
    void setXMLResponse(const QByteArray& data);

private:
    void uploadPayload();
    QString xmlToString(const QByteArray& data);

    Ui::MainWindow *ui;

    DMHNetworkManager* _networkManager;
    DMHNetworkObserver* _networkObserver;
    QString _imageMD5client;
    QString _audioMD5client;
    QString _imageMD5server;
    QString _audioMD5server;
    int _currentImageRequest;
    int _currentAudioRequest;
};

#endif // MAINWINDOW_H
