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

void QuickRefDataWidget::setIcon(const QString& icon)
{
    QString iconName = icon;

    QPixmap iconPix;
    if(!iconPix.load(iconName))
        return;

    QPixmap iconScaled = iconPix.scaled(30, 30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if(!iconScaled.isNull())
    {
        QPixmap finalPix = iconScaled;
        QPainter painter(&finalPix);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(iconScaled.rect(), QColor(85, 85, 85));
        painter.end();
        ui->lblIcon->setPixmap(finalPix);
    }
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
    QString output = _data.getOverview();

    QMessageBox infoBox(parentWidget());
    infoBox.setWindowTitle(_data.getTitle());
    infoBox.setText(output);
    infoBox.setStandardButtons(QMessageBox::Ok);

    QString iconName = _data.getIcon();
    QPixmap iconPix;
    if(iconPix.load(iconName))
    {
        iconPix = iconPix.scaled(60, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        infoBox.setIconPixmap(iconPix);
    }

    infoBox.exec();
}
