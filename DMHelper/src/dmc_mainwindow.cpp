#include "dmc_mainwindow.h"
#include "ui_dmc_mainwindow.h"
#include "dmc_settingsdialog.h"
#include "dmc_optionscontainer.h"
#include "dmc_serverconnection.h"
#include "dmversion.h"
#include "audiotrack.h"
#include <QDir>
#include <QLibraryInfo>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QStandardPaths>
#include <QDebug>

DMC_MainWindow::DMC_MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DMC_MainWindow),
    _settings(nullptr),
    _serverConnection(nullptr),
    _labelPixmap()
{
    qDebug() << "[Main] Initializing Main";

    qDebug() << "[MainWindow] DMHelper version information";
    qDebug() << "[MainWindow]     DMHelper Version: " << QString::number(DMHelper::DMHELPER_MAJOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_MINOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_ENGINEERING_VERSION);
    qDebug() << "[MainWindow]     Expected Bestiary Version: " << QString::number(DMHelper::BESTIARY_MAJOR_VERSION) + "." + QString::number(DMHelper::BESTIARY_MINOR_VERSION);
    qDebug() << "[MainWindow]     Expected Spellbook Version: " << QString::number(DMHelper::SPELLBOOK_MAJOR_VERSION) + "." + QString::number(DMHelper::SPELLBOOK_MINOR_VERSION);
    qDebug() << "[MainWindow]     Expected Campaign File Version: " << QString::number(DMHelper::CAMPAIGN_MAJOR_VERSION) + "." + QString::number(DMHelper::CAMPAIGN_MINOR_VERSION);
    qDebug() << "[MainWindow]     Build from: " << __DATE__ << " " << __TIME__;
#ifdef Q_OS_MAC
    qDebug() << "[MainWindow]     OS: MacOS";
#else
    qDebug() << "[MainWindow]     OS: Windows";
#endif
    qDebug() << "[MainWindow]     Working Directory: " << QDir::currentPath();
    qDebug() << "[MainWindow]     Executable Directory: " << QCoreApplication::applicationDirPath();

    qDebug() << "[MainWindow] Qt Information";
    qDebug() << "[MainWindow]     Qt Version: " << QLibraryInfo::version().toString();
    qDebug() << "[MainWindow]     Is Debug? " << QLibraryInfo::isDebugBuild();
    qDebug() << "[MainWindow]     PrefixPath: " << QLibraryInfo::location(QLibraryInfo::PrefixPath);
    qDebug() << "[MainWindow]     DocumentationPath: " << QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    qDebug() << "[MainWindow]     HeadersPath: " << QLibraryInfo::location(QLibraryInfo::HeadersPath);
    qDebug() << "[MainWindow]     LibrariesPath: " << QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qDebug() << "[MainWindow]     LibraryExecutablesPath: " << QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
    qDebug() << "[MainWindow]     BinariesPath: " << QLibraryInfo::location(QLibraryInfo::BinariesPath);
    qDebug() << "[MainWindow]     PluginsPath: " << QLibraryInfo::location(QLibraryInfo::PluginsPath);
    qDebug() << "[MainWindow]     ImportsPath: " << QLibraryInfo::location(QLibraryInfo::ImportsPath);
    qDebug() << "[MainWindow]     Qml2ImportsPath: " << QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath);
    qDebug() << "[MainWindow]     ArchDataPath: " << QLibraryInfo::location(QLibraryInfo::ArchDataPath);
    qDebug() << "[MainWindow]     DataPath: " << QLibraryInfo::location(QLibraryInfo::DataPath);
    qDebug() << "[MainWindow]     TranslationsPath: " << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    qDebug() << "[MainWindow]     ExamplesPath: " << QLibraryInfo::location(QLibraryInfo::ExamplesPath);
    qDebug() << "[MainWindow]     TestsPath: " << QLibraryInfo::location(QLibraryInfo::TestsPath);
    qDebug() << "[MainWindow]     SettingsPath: " << QLibraryInfo::location(QLibraryInfo::SettingsPath);

    qDebug() << "[MainWindow] Standard Path Information";
    qDebug() << "[MainWindow]     DocumentsLocation: " << (QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    qDebug() << "[MainWindow]     ApplicationsLocation: " << (QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).first());
    qDebug() << "[MainWindow]     RuntimeLocation: " << (QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation).first());
    qDebug() << "[MainWindow]     ConfigLocation: " << (QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first());
    qDebug() << "[MainWindow]     AppDataLocation: " << (QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first());
    qDebug() << "[MainWindow]     AppLocalDataLocation: " << (QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).isEmpty() ? QString() : QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).first());

    ui->setupUi(this);

    connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(openOptions()));
    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    _settings = new DMC_OptionsContainer(this);
    _settings->readSettings();

    _serverConnection = new DMC_ServerConnection(*_settings, this);
    connect(_serverConnection, &DMC_ServerConnection::trackActive, this, &DMC_MainWindow::enableAudio);
    connect(_serverConnection, &DMC_ServerConnection::pixmapActive, this, &DMC_MainWindow::setLabelPixmap);
    connect(_serverConnection, &DMC_ServerConnection::imageActive, this, &DMC_MainWindow::setLabelImage);

    ui->sliderVolume->setValue(50);
    enableAudio(nullptr);

    qDebug() << "[Main] Main Initialization complete";
}

DMC_MainWindow::~DMC_MainWindow()
{
    disconnect(_serverConnection, &DMC_ServerConnection::trackActive, this, &DMC_MainWindow::enableAudio);
    delete ui;
}

void DMC_MainWindow::closeEvent(QCloseEvent *event)
{
    _settings->writeSettings();
    event->accept();
}

void DMC_MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    _serverConnection->targetResized(ui->lblImage->size());
    updatePixmap();
}

void DMC_MainWindow::muteToggled(bool checked)
{
    if(checked)
        ui->btnMute->setIcon(QIcon(QPixmap(QString(":/img/data/icon_volumeoff.png"))));
    else
        ui->btnMute->setIcon(QIcon(QPixmap(QString(":/img/data/icon_volumeon.png"))));
}

void DMC_MainWindow::enableAudio(AudioTrack* track)
{
    disconnect(ui->sliderVolume, &QAbstractSlider::valueChanged, nullptr, nullptr);
    disconnect(ui->btnMute, &QAbstractButton::toggled, nullptr, nullptr);
    connect(ui->btnMute, &QAbstractButton::toggled, this, &DMC_MainWindow::muteToggled);

    if(track)
    {
        connect(ui->sliderVolume, &QAbstractSlider::valueChanged, track, &AudioTrack::setVolume);
        connect(ui->btnMute, &QAbstractButton::toggled, track, &AudioTrack::setMute);
    }

    ui->btnMute->setEnabled(track != nullptr);
    ui->lblMin->setEnabled(track != nullptr);
    ui->lblMax->setEnabled(track != nullptr);
    ui->sliderVolume->setEnabled(track != nullptr);
}

void DMC_MainWindow::openOptions()
{
    DMC_OptionsContainer tempOptions;
    tempOptions.copy(*_settings);

    DMC_SettingsDialog dlg(tempOptions);
    dlg.resize(width() * 1 / 2, height() * 3 / 4);
    if(dlg.exec() == QDialog::Accepted)
    {
        _settings->copy(tempOptions);
        if(_serverConnection)
            _serverConnection->startServer();
    }
}

void DMC_MainWindow::setLabelPixmap(QPixmap pixmap)
{
    _labelPixmap = pixmap;
    updatePixmap();
}

void DMC_MainWindow::setLabelImage(QImage image)
{
    setLabelPixmap(QPixmap::fromImage(image));
}

void DMC_MainWindow::updatePixmap()
{
/*
    QPixmap newPixmap;
    if(_labelPixmap.isNull())
        newPixmap = QPixmap(QString(":/img/data/dmc_background.png"));
    else
        newPixmap = _labelPixmap;

    ui->lblImage->setPixmap(newPixmap.scaled(ui->lblImage->size(), Qt::KeepAspectRatio));
    */

    if(!_labelPixmap.isNull())
        ui->lblImage->setPixmap(_labelPixmap);
}
