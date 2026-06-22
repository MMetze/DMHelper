#include "presentupdatedialog.h"
#include "dmversion.h"
#include "ui_presentupdatedialog.h"
#include <QDesktopServices>
#include <QDebug>

PresentUpdateDialog::PresentUpdateDialog(const QString& newVersion, const QString& releaseNotes, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PresentUpdateDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    // Fix parchment background for QTextBrowser viewport in Qt6
    QPalette parchPal = ui->textBrowser->palette();
    parchPal.setBrush(QPalette::Base, QBrush(QPixmap(QString(":/img/data/parchment.jpg"))));
    ui->textBrowser->setPalette(parchPal);

    QString currentVersion = QString("%1.%2.%3").arg(DMHelper::DMHELPER_MAJOR_VERSION)
                                                .arg(DMHelper::DMHELPER_MINOR_VERSION)
                                                .arg(DMHelper::DMHELPER_ENGINEERING_VERSION);
    ui->lblCurrentVersion->setText(currentVersion);
    ui->lblNewVersion->setText(newVersion);
    ui->textBrowser->setHtml(releaseNotes);

    ui->lblWebsiteLink->setCursor(Qt::PointingHandCursor);

    qDebug() << "[PresentUpdateDialog]: Current version: " << currentVersion << ", new version: " << newVersion;
    qDebug() << "[PresentUpdateDialog]: Release notes: " << releaseNotes;
}

PresentUpdateDialog::~PresentUpdateDialog()
{
    delete ui;
}
