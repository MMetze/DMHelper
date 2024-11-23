#include "welcomeframe.h"
#include "ui_welcomeframe.h"
#include "mruhandler.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QMessageBox>
#include <QtDebug>

WelcomeFrame::WelcomeFrame(MRUHandler* mruHandler, QWidget *parent) :
    CampaignObjectFrame(parent),
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

    ui->lblDiscord->setCursor(Qt::PointingHandCursor);
    ui->lblDiscord->installEventFilter(this);
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
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    QString filePath = fileDirPath.path() + QString("/Resources/DMHelper Realm.xml");
#else
    QString filePath = QCoreApplication::applicationDirPath() + QString("/doc/DMHelper Realm.xml");
#endif

    if(!QFile::exists(filePath))
    {
        qDebug() << "[WelcomeFrame]: unable to open the sample campaign: " << filePath;
        QMessageBox::critical(this, QString("File Open Error"), QString("The sample campaign file could not be found: ") + filePath);
        return;
    }

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
        else if(watched == ui->lblDiscord)
            openLink(ui->lblDiscord->text());
        else
        {
            QLabel* mruLabel = qobject_cast<QLabel*>(watched);
            if(mruLabel)
                openCampaign(mruLabel->toolTip());
        }
    }

    return CampaignObjectFrame::eventFilter(watched, event);
}

void WelcomeFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    setMRUTexts();
}

void WelcomeFrame::openCampaign(const QString& campaignText)
{
    if(!campaignText.isEmpty())
        emit openCampaignFile(campaignText);
}

void WelcomeFrame::openDoc(const QString& docName)
{
#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    QString filePath = fileDirPath.path() + QString("/Resources/") + docName;
#else
    QString filePath = QCoreApplication::applicationDirPath() + QString("/doc/") + docName;
#endif

    if(!QFile::exists(filePath))
    {
        qDebug() << "[WelcomeFrame]: unable to open document: " << filePath;
        QMessageBox::critical(this, QString("File Open Error"), QString("The requested document could not be found: ") + filePath);
        return;
    }

    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    qDebug() << "[WelcomeFrame]: opening URL: " << fileUrl;
    QDesktopServices::openUrl(fileUrl);
}

void WelcomeFrame::openLink(const QString& linkText)
{
    QUrl linkUrl = QUrl::fromUserInput(linkText);
    qDebug() << "[WelcomeFrame]: opening link: " << linkUrl;
    QDesktopServices::openUrl(linkUrl);
}

void WelcomeFrame::setMRUTexts()
{
    if(!_mruHandler)
        return;

    // Clear the MRU list
    QLayoutItem *child;
    while((child = ui->groupMRUList->layout()->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    // Add the MRU list
    for(int i = 0; i < _mruHandler->getMRUList().count(); i++)
    {
        QLabel* newMRU = new QLabel();
        setMRUText(newMRU, i);
        ui->groupMRUList->layout()->addWidget(newMRU);
    }
}

void WelcomeFrame::setMRUText(QLabel* mruLabel, int index)
{
    if((!_mruHandler) || (!mruLabel) || (!ui->groupMRUList))
        return;

    QStringList mruItems = _mruHandler->getMRUList();
    if(mruItems.count() > index)
    {
        QFont font = mruLabel->font();
        font.setUnderline(true);
        font.setPointSize(9);
        mruLabel->setFont(font);
        QFontMetrics metrics = mruLabel->fontMetrics();
        mruLabel->setText(metrics.elidedText(mruItems.at(index), Qt::ElideMiddle, ui->groupMRUList->width() - 5));
        //mruLabel->setMinimumHeight(mruLabel->height() * 3 / 2);
        mruLabel->setToolTip(mruItems.at(index));
        mruLabel->setCursor(Qt::PointingHandCursor);
        mruLabel->installEventFilter(this);
    }
    else
    {
        mruLabel->setText(QString());
        mruLabel->setToolTip(QString());
        mruLabel->unsetCursor();
        mruLabel->removeEventFilter(this);
    }
}
