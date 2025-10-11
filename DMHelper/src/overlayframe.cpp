#include "overlayframe.h"
#include "ui_overlayframe.h"
#include "overlay.h"
#include "dmconstants.h"
#include <QDebug>

OverlayFrame::OverlayFrame(Overlay& overlay, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::OverlayFrame),
    _overlay(overlay)
{
    ui->setupUi(this);

    switch(_overlay.getOverlayType())
    {
        case DMHelper::OverlayType_Fear:
            ui->lblIcon->setPixmap(QPixmap(":/img/data/hoodeyeless.png"));
            break;
        case DMHelper::OverlayType_Counter:
            ui->lblIcon->setPixmap(QPixmap(":/img/data/icon_overlaycounter.png"));
            break;
        case DMHelper::OverlayType_Timer:
            ui->lblIcon->setPixmap(QPixmap(":/img/data/icon_overlaytimer.png"));
            break;
        default:
            qDebug() << "[OverlayFrame] ERROR: unknown overlay type: " << _overlay.getOverlayType();
            break;
    }
}

OverlayFrame::~OverlayFrame()
{
    delete ui;
}
