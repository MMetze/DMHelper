#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dmhnetworkmanager.h"
#include "dmhnetworkobserver.h"
#include "dmhpayload.h"
#include "dmhlogon.h"
#include <QDebug>
#include <QLibraryInfo>
#include <QFileDialog>
#include <QFile>
#include <QDomDocument>

const int DUMMY_DOWNLOAD_ID = 1;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _imageMD5client(),
    _audioMD5client(),
    _imageMD5server(),
    _audioMD5server(),
    _currentImageRequest(0),
    _currentAudioRequest(0)
{
    qDebug() << "[Main] Initializing Main";

    qDebug() << "[Main] Qt Information";
    qDebug() << "[Main]     Qt Version: " << QLibraryInfo::version().toString();
    qDebug() << "[Main]     Is Debug? " << QLibraryInfo::isDebugBuild();
    qDebug() << "[Main]     PrefixPath: " << QLibraryInfo::location(QLibraryInfo::PrefixPath);
    qDebug() << "[Main]     DocumentationPath: " << QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    qDebug() << "[Main]     HeadersPath: " << QLibraryInfo::location(QLibraryInfo::HeadersPath);
    qDebug() << "[Main]     LibrariesPath: " << QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qDebug() << "[Main]     LibraryExecutablesPath: " << QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
    qDebug() << "[Main]     BinariesPath: " << QLibraryInfo::location(QLibraryInfo::BinariesPath);
    qDebug() << "[Main]     PluginsPath: " << QLibraryInfo::location(QLibraryInfo::PluginsPath);
    qDebug() << "[Main]     ImportsPath: " << QLibraryInfo::location(QLibraryInfo::ImportsPath);
    qDebug() << "[Main]     Qml2ImportsPath: " << QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath);
    qDebug() << "[Main]     ArchDataPath: " << QLibraryInfo::location(QLibraryInfo::ArchDataPath);
    qDebug() << "[Main]     DataPath: " << QLibraryInfo::location(QLibraryInfo::DataPath);
    qDebug() << "[Main]     TranslationsPath: " << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    qDebug() << "[Main]     ExamplesPath: " << QLibraryInfo::location(QLibraryInfo::ExamplesPath);
    qDebug() << "[Main]     TestsPath: " << QLibraryInfo::location(QLibraryInfo::TestsPath);
    qDebug() << "[Main]     SettingsPath: " << QLibraryInfo::location(QLibraryInfo::SettingsPath);

    ui->setupUi(this);

    connect(ui->btnUploadImage, SIGNAL(clicked(bool)), this, SLOT(startImageUpload()));
    connect(ui->btnUploadAudio, SIGNAL(clicked(bool)), this, SLOT(startAudioUpload()));
    connect(ui->btnClearImage, SIGNAL(clicked(bool)), this, SLOT(clearImage()));
    connect(ui->btnClearAudio, SIGNAL(clicked(bool)), this, SLOT(clearAudio()));
    connect(ui->btnDownload, SIGNAL(clicked(bool)), this, SLOT(runFileDownload()));
    connect(ui->btnExists, SIGNAL(clicked(bool)), this, SLOT(runFileExists()));

    DMHLogon logon("https://dmh.wwpd.de", "c.turner", "Ente12345", "7B3AA550-649A-4D51-920E-CAB465616995");

    _networkManager = new DMHNetworkManager(logon, this);
    connect(_networkManager, SIGNAL(downloadComplete(int, const QString&, const QByteArray&)), this, SLOT(downloadComplete(int, const QString&, const QByteArray&)));
    connect(_networkManager, SIGNAL(existsComplete(int, const QString&, const QString&, bool)), this, SLOT(existsComplete(int, const QString&, const QString&, bool)));
    connect(_networkManager, SIGNAL(uploadComplete(int, const QString&)), this, SLOT(uploadComplete(int, const QString&)));
    connect(_networkManager, SIGNAL(DEBUG_message_contents(const QByteArray&)), this, SLOT(setXMLRequest(const QByteArray&)));
    connect(_networkManager, SIGNAL(DEBUG_response_contents(const QByteArray&)), this, SLOT(setXMLResponse(const QByteArray&)));

    _networkObserver = new DMHNetworkObserver(logon, this);
    connect(_networkObserver, SIGNAL(payloadReceived(const DMHPayload&, const QString&)), this, SLOT(payloadReceived(const DMHPayload&, const QString&)));
    connect(_networkObserver, SIGNAL(DEBUG_response_contents(const QByteArray&)), this, SLOT(setPayloadContents(const QByteArray&)));
    _networkObserver->start();

    qDebug() << "[Main] Main Initialization complete";
}

MainWindow::~MainWindow()
{
    disconnect(_networkManager, nullptr, this, nullptr);
    disconnect(_networkObserver, nullptr, this, nullptr);

    delete ui;
}

void MainWindow::downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data)
{
    qDebug() << "[Main] Download complete " << requestID << ": " << fileMD5 << ", " << data.size() << " bytes";
    if(requestID == _currentImageRequest)
    {
        _currentImageRequest = 0;
        if(data.size() > 0)
        {
            qDebug() << "[Main] Image data received.";
            //ui->edtImage->setPlainText(QString::fromUtf8(data).toHtmlEscaped());
        }
        else
        {
            qDebug() << "[Main] WARNING: Download complete for image download with no data received, no pixmap set";
        }
    }
    else if(requestID == _currentAudioRequest)
    {
        _currentAudioRequest = 0;
        QFile outputFile(fileMD5 + QString(".mp3"));
        if(data.size() > 0)
        {
            qDebug() << "[Main] Audio data received.";
            //ui->edtAudio->setPlainText(QString::fromUtf8(data));
        }
        else
        {
            qDebug() << "[Main] WARNING: Download complete for audio download with no data received, no playback possible";
        }
    }
    else
    {
        qDebug() << "[Main] ERROR: Unexpected request ID received!";
    }
}

void MainWindow::uploadComplete(int requestID, const QString& fileMD5)
{
    qDebug() << "[Main] Upload complete " << requestID << ": " << fileMD5;

    if(requestID == _currentImageRequest)
    {
        _currentImageRequest = 0;
        if(fileMD5.isEmpty())
        {
            qDebug() << "[Main] Upload complete for Image with invalid MD5 value, no payload uploaded.";
        }
        else
        {
            qDebug() << "[Main] Upload complete for Image: " << _imageMD5server << " Payload being uploaded, Audio: " << _audioMD5server;
            _imageMD5server = fileMD5;
            uploadPayload();
        }
    }
    else if(requestID == _currentAudioRequest)
    {
        _currentAudioRequest = 0;
        if(fileMD5.isEmpty())
        {
            qDebug() << "[Main] Upload complete for Audio with invalid MD5 value, no payload uploaded.";
        }
        else
        {
            qDebug() << "[Main] Upload complete for Audio: " << _audioMD5server << " Payload being uploaded, Image: " << _imageMD5server;
            _audioMD5server = fileMD5;
            uploadPayload();
        }
    }
    else
    {
        qDebug() << "[Main] ERROR: Unexpected request ID received!";
    }
}

void MainWindow::existsComplete(int requestID, const QString& fileMD5, const QString& filename, bool exists)
{
    qDebug() << "[Main] Exists complete " << requestID << ": " << fileMD5 << " = " << filename << ", exists = " << exists;
    ui->edtExistsMD5received->setText(fileMD5);
    ui->edtExistsFilename->setText(filename);
    ui->edtExists->setText(exists ? QString("true") : QString("false"));
}


void MainWindow::payloadReceived(const DMHPayload& payload, const QString& timestamp)
{
    ui->edtTimestamp->setText(timestamp);

    if(payload.getImageFile() != _imageMD5client)
    {
        qDebug() << "[Main] Payload received with new Image file. Image: " << payload.getImageFile() << ", Audio: " << payload.getAudioFile() << ", Timestamp: " << timestamp;
        _imageMD5client = payload.getImageFile();
        ui->edtPayloadImage->setText(_imageMD5client);
        /*
        if(!_imageMD5client.isEmpty())
            _currentImageRequest = _networkManager->downloadFile(_imageMD5client);
        */
    }

    if(payload.getAudioFile() != _audioMD5client)
    {
        qDebug() << "[Main] Payload received with new Audio file. Image: " << payload.getImageFile() << ", Audio: " << payload.getAudioFile() << ", Timestamp: " << timestamp;
        _audioMD5client = payload.getAudioFile();
        ui->edtPayloadAudio->setText(_audioMD5client);
        /*
        if(!_audioMD5client.isEmpty())
        {
            if(QFile::exists(_audioMD5client + QString(".mp3")))
            {
                qDebug() << "[Main] Payload received with existing Audio file";
                _currentAudioRequest = DUMMY_DOWNLOAD_ID;
                 downloadComplete(DUMMY_DOWNLOAD_ID, _audioMD5client, QByteArray());
            }
            else
            {
                qDebug() << "[Main] Payload received with a new Audio file";
                _currentAudioRequest = _networkManager->downloadFile(_audioMD5client);
            }
        }
        */
    }
}

void MainWindow::startImageUpload()
{
//    if(_currentImageRequest > 0)
//        return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Images (*.png *.xpm *.jpg)"));
    if(!fileName.isNull())
        _currentImageRequest = _networkManager->uploadFile(fileName);
}

void MainWindow::startAudioUpload()
{
//    if(_currentAudioRequest > 0)
//        return;
/*
    if(_audioMD5server.isEmpty())
    {*/
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Soundtracks (*.mp3)"));
        if(!fileName.isNull())
            _currentAudioRequest = _networkManager->uploadFile(fileName);
    /*}
    else
    {
        //_currentAudioRequest = _networkManager->fileExists(_audioMD5server); //1b21d77beda1f4743c11d618e38d1113
        _currentAudioRequest = _networkManager->fileExists(QString("1b21d77beda1f4743c11d618e38d1113"));
    }
    */
}

void MainWindow::clearImage()
{
//    if(_currentImageRequest > 0)
//        return;

    _imageMD5server = QString("");
    uploadPayload();
}

void MainWindow::clearAudio()
{
//    if(_currentAudioRequest > 0)
//        return;

    _audioMD5server = QString("");
    uploadPayload();
}

void MainWindow::runFileDownload()
{
    _networkManager->downloadFile(ui->edtDownloadMD5->text());
}

void MainWindow::runFileExists()
{
    _networkManager->fileExists(ui->edtExistsMD5->text());
}

void MainWindow::setPayloadContents(const QByteArray& data)
{
    ui->edtPayloadXML->setPlainText(xmlToString(data));
}

void MainWindow::setXMLRequest(const QByteArray& data)
{
    ui->edtXMLRequest->setPlainText(xmlToString(data));
}

void MainWindow::setXMLResponse(const QByteArray& data)
{
    ui->edtXMLResponse->setPlainText(xmlToString(data));
}

void MainWindow::uploadPayload()
{
    if(!_networkManager)
        return;

    DMHPayload payload;
    payload.setImageFile(_imageMD5server);
    payload.setAudioFile(_audioMD5server);
    _networkManager->uploadPayload(payload);
}

QString MainWindow::xmlToString(const QByteArray& data)
{
    QString msg(data);
    QString errorMsg;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if(doc.setContent(data, &errorMsg, &errorLine, &errorColumn))
    {
        return msg + "\n" + doc.toString(4);
    }
    else
    {
        return QString::number(errorLine) + " " + QString::number(errorColumn) + " " + errorMsg + "\n" + msg;
    }
}
