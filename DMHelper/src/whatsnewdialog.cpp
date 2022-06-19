#include "whatsnewdialog.h"
#include "ui_whatsnewdialog.h"
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QScreen>

WhatsNewDialog::WhatsNewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WhatsNewDialog),
    _backgroundImage(),
    _backgroundImageScaled()
{
    ui->setupUi(this);

    ui->textBrowser->installEventFilter(this);

    QFile data(":/img/data/whatsnew.txt");
    if(data.open(QFile::ReadOnly))
    {
        QTextStream in(&data);
        ui->textBrowser->setHtml(in.readAll());
    }

    _backgroundImage.load(":/img/data/dmc_background.png");
}

WhatsNewDialog::~WhatsNewDialog()
{
    delete ui;
}

bool WhatsNewDialog::eventFilter(QObject *watched, QEvent *event)
{
    if((watched == ui->textBrowser) && (event->type() == QEvent::Paint) && (!_backgroundImageScaled.isNull()))
    {
        QPainter paint(ui->textBrowser);
        paint.drawImage((ui->textBrowser->width() - _backgroundImageScaled.width()) / 2,
                        (ui->textBrowser->height() - _backgroundImageScaled.height()) / 2,
                        _backgroundImageScaled);
    }

    return QDialog::eventFilter(watched, event);
}

void WhatsNewDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    deleteLater();
}

void WhatsNewDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    QSize dlgSize = size();
    QSize browserSize = ui->textBrowser->size();
    QScreen* primary = QGuiApplication::primaryScreen();
    if(primary)
    {
        int dlgWidth = primary->availableSize().width() / 2;
        int dlgHeight = dlgWidth * _backgroundImage.height() / _backgroundImage.width();
        QSize newDlgSize(dlgWidth + (dlgSize.width() - browserSize.width()), dlgHeight + (dlgSize.height() - browserSize.height()));
        resize(newDlgSize);
    }
}

void WhatsNewDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    scaleBackgroundImage();
}

void WhatsNewDialog::scaleBackgroundImage()
{
    if(!_backgroundImage.isNull())
        _backgroundImageScaled = _backgroundImage.scaled(ui->textBrowser->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

