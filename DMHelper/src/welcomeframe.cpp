#include "welcomeframe.h"
#include "ui_welcomeframe.h"
#include "mruhandler.h"
#include <QDesktopServices>
#include <QUrl>
#include <QtDebug>

WelcomeFrame::WelcomeFrame(MRUHandler* mruHandler, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::WelcomeFrame),
    _mruHandler(mruHandler)
{
    ui->setupUi(this);

    ui->lblUsersGuide->setCursor(Qt::PointingHandCursor);
    ui->lblUsersGuide->installEventFilter(this);
    ui->lblGettingStartedGuide->setCursor(Qt::PointingHandCursor);
    ui->lblGettingStartedGuide->installEventFilter(this);
    ui->lblOpenSampleCampaign->setCursor(Qt::PointingHandCursor);
    ui->lblOpenSampleCampaign->installEventFilter(this);

    ui->lblMRU1->setCursor(Qt::PointingHandCursor);
    ui->lblMRU1->installEventFilter(this);
    ui->lblMRU2->setCursor(Qt::PointingHandCursor);
    ui->lblMRU2->installEventFilter(this);
    ui->lblMRU3->setCursor(Qt::PointingHandCursor);
    ui->lblMRU3->installEventFilter(this);
    ui->lblMRU4->setCursor(Qt::PointingHandCursor);
    ui->lblMRU4->installEventFilter(this);
}

WelcomeFrame::~WelcomeFrame()
{
    delete ui;
}

void WelcomeFrame::setMRUHandler(MRUHandler* mruHandler)
{
    _mruHandler = mruHandler;
}

void WelcomeFrame::openUsersGuide()
{
    openDoc(QString("DM Helper Users Guide.pdf"));
}

void WelcomeFrame::openGettingStarted()
{
    openDoc(QString("DM Helper Getting Started.pdf"));
}

void WelcomeFrame::openSampleCampaign()
{
#ifdef Q_OS_MAC
    QDir filePath(QCoreApplication::applicationDirPath());
    filePath.cdUp();
    filePath.cdUp();
    filePath.cdUp();
    QString filePath = filePath.path() + QString("/doc/DMHelper Realm.xml");
#else
    QString filePath = QCoreApplication::applicationDirPath() + QString("/doc/DMHelper Realm.xml");
#endif
    emit openCampaignFile(filePath);
}

bool WelcomeFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        if(watched == ui->lblUsersGuide)
            openUsersGuide();
        else if(watched == ui->lblGettingStartedGuide)
            openGettingStarted();
        else if(watched == ui->lblOpenSampleCampaign)
            openSampleCampaign();
        else if(watched == ui->lblMRU1)
            openCampaign(ui->lblMRU1->text());
        else if(watched == ui->lblMRU2)
            openCampaign(ui->lblMRU2->text());
        else if(watched == ui->lblMRU3)
            openCampaign(ui->lblMRU3->text());
        else if(watched == ui->lblMRU4)
            openCampaign(ui->lblMRU4->text());
    }

    return QFrame::eventFilter(watched, event);
}

void WelcomeFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    if(_mruHandler)
    {
        QStringList mruItems = _mruHandler->getMRUList();
        ui->lblMRU1->setText(mruItems.count() > 0 ? mruItems.at(0) : QString(""));
        ui->lblMRU2->setText(mruItems.count() > 1 ? mruItems.at(1) : QString(""));
        ui->lblMRU3->setText(mruItems.count() > 2 ? mruItems.at(2) : QString(""));
        ui->lblMRU4->setText(mruItems.count() > 3 ? mruItems.at(3) : QString(""));
    }
}

void WelcomeFrame::openCampaign(const QString& campaignText)
{
    if(!campaignText.isEmpty())
        emit openCampaignFile(campaignText);
}

void WelcomeFrame::openDoc(const QString& docName)
{
#ifdef Q_OS_MAC
    QDir filePath(QCoreApplication::applicationDirPath());
    filePath.cdUp();
    filePath.cdUp();
    filePath.cdUp();
    QString filePath = filePath.path() + QString("/doc/") + docName;
#else
    QString filePath = QCoreApplication::applicationDirPath() + QString("/doc/") + docName;
#endif
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    qDebug() << "[WelcomeFrame]: opening URL: " << fileUrl;
    QDesktopServices::openUrl(fileUrl);
}