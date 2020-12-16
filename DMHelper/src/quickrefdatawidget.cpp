#include "quickrefdatawidget.h"
#include "ui_quickrefdatawidget.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>

QuickRefDataWidget::QuickRefDataWidget(const QuickRefData& data, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QuickRefDataWidget),
    _data(data),
    _mouseDown(Qt::NoButton)
{
    ui->setupUi(this);

    setIcon(data.getIcon());
    ui->lblTitle->setText(data.getTitle());
    ui->lblSubtitle->setText(data.getSubtitle());
}

QuickRefDataWidget::~QuickRefDataWidget()
{
    delete ui;
}
/*
QSize QuickRefDataWidget::sizeHint() const
{

}
*/

void QuickRefDataWidget::setIcon(const QString& icon)
{
    QString iconName = icon;
    //iconName = QString(":/img/data/img/") + icon + QString(".png");

    QPixmap iconPix;
    if(!iconPix.load(iconName))
        return;

    QPixmap iconScaled = iconPix.scaled(30,30,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if(!iconScaled.isNull())
    {
        QPixmap finalPix = iconScaled;
        QPainter painter(&finalPix);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(iconScaled.rect(), QColor(85, 85, 85));
        painter.end();
        ui->lblIcon->setPixmap(finalPix);
    }

    /*
     * TODO: previous version, can be deleted
    QImage iconImg;
    iconImg.load(iconName);
    QImage iconScaled = iconImg.scaled(30,30,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if(!iconScaled.isNull())
    {
        ui->lblIcon->setPixmap(QPixmap::fromImage(iconScaled));
    }
    */
}

void QuickRefDataWidget::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);
    _mouseDown = Qt::NoButton;
}

void QuickRefDataWidget::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
    _mouseDown = event->button();
}

void QuickRefDataWidget::mouseReleaseEvent(QMouseEvent * event)
{
    if(_mouseDown == event->button())
    {
        if(event->button() == Qt::LeftButton)
        {
            if(rect().contains(event->pos()))
            {
                showQuickRefDetails();
            }
        }
        _mouseDown = Qt::NoButton;
    }
}

void QuickRefDataWidget::showQuickRefDetails()
{
    QString output;
    if(!_data.getDescription().isEmpty())
    {
        output.append(QString("<i>") + _data.getDescription() + QString("</i>"));
        output.append(QString("<p>"));
    }
    output.append(QString("<ul>"));
    for(int i = 0; i < _data.getBullets().count(); ++i)
    {
        output.append(QString("<li>") + _data.getBullets().at(i) + QString("</li>"));
    }
    output.append(QString("</ul>"));
    if(!_data.getReference().isEmpty())
    {
        output.append(QString("<p>"));
        output.append(QString("<i>") + _data.getReference() + QString("</i>"));
    }

    // TODO: previous version can be deleted
    // QMessageBox::about(parentWidget(), _data.getTitle(), output);

    QMessageBox infoBox(parentWidget());
    infoBox.setWindowTitle(_data.getTitle());
    infoBox.setText(output);
    infoBox.setStandardButtons(QMessageBox::Ok);

    QString iconName = _data.getIcon();
    QPixmap iconPix;
    //iconName = QString(":/img/data/img/") + _data.getIcon() + QString(".png");
    if(iconPix.load(iconName))
    {
        iconPix = iconPix.scaled(60,60,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        /*
        QPixmap coloredPix = iconPix;
        QPainter painter(&coloredPix);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(coloredPix.rect(), QColor(85, 85, 85));
        painter.end();
        infoBox.setIconPixmap(coloredPix);
        */
        infoBox.setIconPixmap(iconPix);
    }

    infoBox.exec();
}
