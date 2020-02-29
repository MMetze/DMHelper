#include "presentupdatedialog.h"
#include "dmconstants.h"
#include "ui_presentupdatedialog.h"
#include <QDesktopServices>
#include <QDebug>

PresentUpdateDialog::PresentUpdateDialog(const QString& newVersion, const QString& releaseNotes, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PresentUpdateDialog)
{
    ui->setupUi(this);

    QString currentVersion = QString("%1.%2.%3").arg(DMHelper::DMHELPER_MAJOR_VERSION)
                                                .arg(DMHelper::DMHELPER_MINOR_VERSION)
                                                .arg(DMHelper::DMHELPER_ENGINEERING_VERSION);
    ui->lblCurrentVersion->setText(currentVersion);
    ui->lblNewVersion->setText(newVersion);
    ui->txtReleaseNotes->setHtml(releaseNotes);

    ui->lblWebsiteLink->setCursor(Qt::PointingHandCursor);
    ui->lblWebsiteLink->installEventFilter(this);

}

PresentUpdateDialog::~PresentUpdateDialog()
{
    delete ui;
}

bool PresentUpdateDialog::eventFilter(QObject *watched, QEvent *event)
{
    if((event->type() == QEvent::MouseButtonRelease) &&
       (watched == ui->lblWebsiteLink))
    {
        QUrl linkUrl = QUrl::fromUserInput(ui->lblWebsiteLink->text());
        qDebug() << "[PresentUpdateDialog]: opening link: " << linkUrl;
        QDesktopServices::openUrl(linkUrl);
    }

    return QDialog::eventFilter(watched, event);
}
