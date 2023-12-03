#include "textpublishdialog.h"
#include "ui_textpublishdialog.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QHideEvent>

TextPublishDialog::TextPublishDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextPublishDialog),
    mouseDown(false),
    mouseDownPos()
{
    ui->setupUi(this);

    connect(ui->btnPublish, SIGNAL(clicked()), this, SLOT(publishTextImage()));
    connect(this, SIGNAL(publishImage(QImage, const QColor&)), this, SLOT(setPreviewImage(QImage, const QColor&)));
    connect(ui->btnClear, SIGNAL(clicked()), ui->textEdit, SLOT(clear()));
}

TextPublishDialog::~TextPublishDialog()
{
    delete ui;
}

void TextPublishDialog::publishTextImage()
{
    if(!ui->textEdit->document())
        return;

    QImage pub(ui->textEdit->document()->size().toSize(), QImage::Format_ARGB32);
    pub.fill(Qt::white);
    QPainter painter(&pub);
    ui->textEdit->document()->drawContents(&painter);

    emit publishImage(pub, Qt::white);
}

void TextPublishDialog::setPreviewImage(QImage img, const QColor& color)
{
    Q_UNUSED(color);

    QImage scaledImg = img.scaled(ui->lblPreview->size(), Qt::KeepAspectRatio);
    ui->lblPreview->setPixmap(QPixmap::fromImage(scaledImg));
}

void TextPublishDialog::mousePressEvent(QMouseEvent * event)
{
    mouseDownPos = event->pos();
    mouseDown = true;

    QDialog::mousePressEvent(event);
}

void TextPublishDialog::mouseReleaseEvent(QMouseEvent * event)
{
    if((mouseDown)&&(mouseDownPos == event->pos())&&(ui->lblPreview->geometry().contains(event->pos())))
        emit openPreview();

    mouseDown = false;
    QDialog::mouseReleaseEvent(event);
}

void TextPublishDialog::keyPressEvent(QKeyEvent * event)
{
    if(event->key() == Qt::Key_Escape)
        hide();
    else
        QDialog::keyPressEvent(event);
}

void TextPublishDialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);
    deleteLater();
}
