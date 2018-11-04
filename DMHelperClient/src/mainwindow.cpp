 #include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dmhnetworkmanager.h"
#include "dmhnetworkobserver.h"
#include "dmhpayload.h"
#include "dmhlogon.h"
#include "settingsdialog.h"
#include "optionscontainer.h"
#include "battledialog.h"
#include "dmhsrc/battledialogmodel.h"
#include <QDebug>
#include <QLibraryInfo>
#include <QFileDialog>
#include <QFile>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QDomDocument>

const int DUMMY_DOWNLOAD_ID = 1;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _settings(nullptr),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _imageMD5client(),
    _audioMD5client(),
    //_imageMD5server(),
    //_audioMD5server(),
    _currentImageRequest(0),
    _currentAudioRequest(0),
    _player(nullptr),
    _playlist(nullptr),
    _pmp(),
    _battleDlg(nullptr),
    _battleDlgModel(nullptr)
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

    connect(ui->actionOptions,SIGNAL(triggered()),this,SLOT(openOptions()));
    connect(ui->actionExit,SIGNAL(triggered()),qApp,SLOT(quit()));

    _settings = new OptionsContainer(this);
    _settings->readSettings();

    //connect(ui->btnUploadImage, SIGNAL(clicked(bool)), this, SLOT(startImageUpload()));
    //connect(ui->btnUploadAudio, SIGNAL(clicked(bool)), this, SLOT(startAudioUpload()));

    _player = new QMediaPlayer(this);
    _playlist = new QMediaPlaylist(this);
    _playlist->setPlaybackMode(QMediaPlaylist::Loop);
    connect(ui->sliderVolume, SIGNAL(valueChanged(int)), _player, SLOT(setVolume(int)));
    connect(ui->btnMute, SIGNAL(toggled(bool)), this, SLOT(muteToggled(bool)));
    ui->sliderVolume->setValue(50);
    enableAudio(false);

    DMHLogon logon(_settings->getURLString(), _settings->getUserName(), _settings->getPassword(), _settings->getSession());

    _networkManager = new DMHNetworkManager(logon, this);
    connect(_networkManager, SIGNAL(downloadComplete(int, const QString&, const QByteArray&)), this, SLOT(downloadComplete(int, const QString&, const QByteArray&)));
    //connect(_networkManager, &DMHNetworkManager::downloadComplete, this, &MainWindow::downloadComplete);
    //connect(_networkManager, SIGNAL(uploadComplete(int, const QString&)), this, SLOT(uploadComplete(int, const QString&)));
    //connect(_networkManager, &DMHNetworkManager::uploadComplete, this, &MainWindow::uploadComplete);
    //_networkObserver->start();
    //_networkManager->downloadFile(QString("1c4e4328d6874fb1eede36b941e271a3"));

    _networkObserver = new DMHNetworkObserver(logon, this);
    connect(_networkObserver, SIGNAL(payloadReceived(const DMHPayload&, const QString&)), this, SLOT(payloadReceived(const DMHPayload&, const QString&)));
    _networkObserver->start();

    loadBattle();

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
            if(_pmp.loadFromData(data))
                ui->lblImage->setPixmap(_pmp.scaled(ui->lblImage->size(), Qt::KeepAspectRatio));
            else
                qDebug() << "[Main] Download complete Pixmap loading failed";
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
            if(!outputFile.exists())
            {
                if(outputFile.open(QIODevice::WriteOnly))
                {
                    outputFile.write(data);
                    outputFile.close();
                }
            }
        }

        if(outputFile.exists())
        {
            _player->stop();
            _playlist->clear();
            QMediaContent track(QUrl::fromLocalFile(outputFile.fileName()));
            _playlist->addMedia(track);
            _player->setPlaylist(_playlist);
            _player->play();
            enableAudio(true);
        }
        else
        {
            qDebug() << "[Main] WARNING: Download complete for audio download with no data received, no playback possible";
            enableAudio(false);
        }
    }
    else
    {
        qDebug() << "[Main] ERROR: Unexpected request ID received!";
    }
}

/*
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
            DMHPayload payload;
            _imageMD5server = fileMD5;
            payload.setImageFile(_imageMD5server);
            payload.setAudioFile(_audioMD5server);
            _networkManager->uploadPayload(payload);
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
            DMHPayload payload;
            _audioMD5server = fileMD5;
            payload.setImageFile(_imageMD5server);
            payload.setAudioFile(_audioMD5server);
            _networkManager->uploadPayload(payload);
        }
    }
    else
    {
        qDebug() << "[Main] ERROR: Unexpected request ID received!";
    }
}
*/

void MainWindow::payloadReceived(const DMHPayload& payload, const QString& timestamp)
{
    if(_currentImageRequest <= 0)
    {
        if(payload.getImageFile() != _imageMD5client)
        {
            qDebug() << "[Main] Payload received with new Image file. Image: " << payload.getImageFile() << ", Audio: " << payload.getAudioFile() << ", Timestamp: " << timestamp;
            _imageMD5client = payload.getImageFile();
            if(_imageMD5client.isEmpty())
                ui->lblImage->setPixmap(QPixmap());
            else
                _currentImageRequest = _networkManager->downloadFile(_imageMD5client);
        }
    }

    if(_currentAudioRequest <= 0)
    {
        if(payload.getAudioFile() != _audioMD5client)
        {
            qDebug() << "[Main] Payload received with new Audio file. Image: " << payload.getImageFile() << ", Audio: " << payload.getAudioFile() << ", Timestamp: " << timestamp;
            _audioMD5client = payload.getAudioFile();
            if(_audioMD5client.isEmpty())
            {
                _player->stop();
                enableAudio(false);
            }
            else
            {
                if(QFile::exists(_audioMD5client + QString(".mp3")))
                {
                    _currentAudioRequest = DUMMY_DOWNLOAD_ID;
                     downloadComplete(DUMMY_DOWNLOAD_ID, _audioMD5client, QByteArray());
                }
                else
                {
                    _currentAudioRequest = _networkManager->downloadFile(_audioMD5client);
                }
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    _settings->writeSettings();
    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(!_pmp.isNull())
        ui->lblImage->setPixmap(_pmp.scaled(ui->lblImage->size(), Qt::KeepAspectRatio));
}

/*
void MainWindow::startImageUpload()
{
    if(_currentImageRequest > 0)
        return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Images (*.png *.xpm *.jpg)"));
    if(!fileName.isNull())
        _currentImageRequest = _networkManager->uploadFile(fileName);
}

void MainWindow::startAudioUpload()
{
    if(_currentAudioRequest > 0)
        return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Soundtracks (*.mp3)"));
    if(!fileName.isNull())
        _currentAudioRequest = _networkManager->uploadFile(fileName);
}
*/

void MainWindow::muteToggled(bool checked)
{
    _player->setMuted(checked);
    if(checked)
        ui->btnMute->setIcon(QIcon(QPixmap(QString(":/img/data/icon_mute.png"))));
    else
        ui->btnMute->setIcon(QIcon(QPixmap(QString(":/img/data/icon_audio.png"))));
}

void MainWindow::enableAudio(bool enabled)
{
    ui->btnMute->setEnabled(enabled);
    ui->lblMin->setEnabled(enabled);
    ui->lblMax->setEnabled(enabled);
    ui->sliderVolume->setEnabled(enabled);
}

void MainWindow::openOptions()
{
    OptionsContainer tempOptions;
    tempOptions.copy(*_settings);

    SettingsDialog dlg(tempOptions);
    if(dlg.exec() == QDialog::Accepted)
    {
        _settings->copy(tempOptions);
        DMHLogon logon(_settings->getURLString(), _settings->getUserName(), _settings->getPassword(), _settings->getSession());
        _networkManager->setLogon(logon);
        _networkObserver->setLogon(logon);
    }
}

void MainWindow::loadBattle()
{
    QFile localPayload("_dmhpayload.txt");
    localPayload.open(QIODevice::ReadOnly);
    QTextStream in(&localPayload);
    in.setCodec("UTF-8");
    QDomDocument doc("DMHelperXML");
    doc.setContent( in.readAll() );
    localPayload.close();

    QDomElement root = doc.documentElement();
    QDomElement battleElement = root.firstChildElement( QString("battle") );

    _battleDlgModel = new BattleDialogModel(this);
    _battleDlgModel->inputXML(battleElement);

    _battleDlg = new BattleDialog(*_battleDlgModel, this);
    _battleDlg->updateMap();
    _battleDlg->show();
}
