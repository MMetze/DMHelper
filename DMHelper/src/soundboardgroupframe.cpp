#include "soundboardgroupframe.h"
#include "ui_soundboardgroupframe.h"
#include "soundboardtrackframe.h"
#include "soundboardgroup.h"
#include "soundboardtrack.h"
#include "campaign.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUuid>
#include <QDebug>

SoundBoardGroupFrame::SoundBoardGroupFrame(SoundboardGroup* group, Campaign* campaign, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SoundBoardGroupFrame),
    _trackWidgets(),
    _localMute(false),
    _group(group),
    _campaign(campaign)
{
    ui->setupUi(this);

    setAcceptDrops(true);

    if(!_group)
    {
        qDebug() << "[SoundBoardGroupFrame] Frame created with empty group!";
        return;
    }

    ui->edtName->setText(_group->getGroupName());
    ui->btnExpand->setIcon(QIcon(QPixmap(":/img/data/icon_downarrow.png").transformed(QTransform().rotate(180))));
    ui->sliderVolume->setValue(_group->getVolume());
    setMute(_group->getMute());

    connect(ui->btnExpand, &QPushButton::clicked, this, &SoundBoardGroupFrame::toggleContents);
    connect(ui->btnMute, &QPushButton::clicked, this, &SoundBoardGroupFrame::toggleMute);
    connect(ui->btnRemove, &QPushButton::clicked, this, &SoundBoardGroupFrame::handleRemove);
    connect(ui->sliderVolume, &QSlider::valueChanged, this, &SoundBoardGroupFrame::volumeChanged);
    connect(this, &SoundBoardGroupFrame::volumeChanged, _group, &SoundboardGroup::setVolume);

    for(SoundboardTrack* track : _group->getTracks())
    {
        if(track)
            addTrackToLayout(track);
    }

    connect(_group, &SoundboardGroup::destroyed, this, &SoundBoardGroupFrame::handleRemove);
}

SoundBoardGroupFrame::~SoundBoardGroupFrame()
{
    _trackWidgets.clear();
    delete ui;
}

bool SoundBoardGroupFrame::isMuted() const
{
    return _localMute;
}

int SoundBoardGroupFrame::getVolume() const
{
    return _localMute ? 0 : ui->sliderVolume->value();
}

SoundboardGroup* SoundBoardGroupFrame::getGroup() const
{
    return _group;
}

void SoundBoardGroupFrame::updateTrackLayout()
{
    if(_trackWidgets.count() <= 0)
        return;

    QVBoxLayout* mainLayout = dynamic_cast<QVBoxLayout*>(ui->frameWidgets->layout());
    if(!mainLayout)
        return;

    QLayoutItem* childLayoutItem;
    QLayoutItem* childLayoutChildItem;
    while((childLayoutItem = mainLayout->takeAt(0)) != nullptr)
    {
        QLayout* childLayout = childLayoutItem->layout();
        if(childLayout)
        {
            while((childLayoutChildItem = childLayout->takeAt(0)) != nullptr)
            {
                delete childLayoutChildItem;
            }
        }
        delete childLayoutItem;
    }

    if(_trackWidgets.count() == 0)
        return;

    if(_trackWidgets.at(0)->width() + mainLayout->spacing() == 0)
        return;
    int xMaximum = ui->frameWidgets->width() / (_trackWidgets.at(0)->width() + mainLayout->spacing());
    if(xMaximum == 0)
        return;
    int yCount = _trackWidgets.count() / xMaximum + (_trackWidgets.count() % xMaximum ? 1 : 0);

    for(int y = 0; y < yCount; ++y)
    {
        QHBoxLayout* newLineLayout = new QHBoxLayout();
        int xCount = y == (yCount - 1) ? (_trackWidgets.count() - (y * xMaximum)) : xMaximum;
        for(int x = 0; x < xCount; ++x)
        {
            newLineLayout->addWidget(_trackWidgets.at(y*xCount + x));
        }
        newLineLayout->addStretch();
        mainLayout->addLayout(newLineLayout);
    }

    ui->frameWidgets->adjustSize();
    update();
}

void SoundBoardGroupFrame::addTrack(SoundboardTrack* track)
{
    if((!_group) || (!track))
        return;

    _group->addTrack(track);
    addTrackToLayout(track);
    emit dirty();
}

void SoundBoardGroupFrame::removeTrack(SoundboardTrack* track)
{
    if((!_group) || (!track))
        return;

    for(int i = 0; i < _trackWidgets.count(); ++i)
    {
        SoundboardTrackFrame* trackWidget = _trackWidgets.at(i);
        if(trackWidget->getTrack() == track)
        {
            if(_trackWidgets.removeOne(trackWidget))
                trackWidget->deleteLater();
        }
    }

    _group->removeTrack(track);
    updateTrackLayout();
    emit dirty();
}

void SoundBoardGroupFrame::setMute(bool mute)
{
    ui->btnMute->setIcon(mute ? QIcon(QPixmap(":/img/data/icon_volumeoff.png")) : QIcon(QPixmap(":/img/data/icon_volumeon.png")));
    ui->sliderVolume->setEnabled(!mute);
    ui->btnMute->setChecked(mute);
}

void SoundBoardGroupFrame::trackMuteChanged(bool mute)
{
    if((_localMute)&&(!mute))
    {
        setMute(false);
        _localMute = false;
        emit overrideChildMute(true);
    }
}

void SoundBoardGroupFrame::handleRemove()
{
    if(_group)
        emit removeGroup(_group);
}

void SoundBoardGroupFrame::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    toggleContents();
}

void SoundBoardGroupFrame::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    ui->frameControls->move(width() - ui->frameControls->width(), 0);
}

void SoundBoardGroupFrame::dragEnterEvent(QDragEnterEvent *event)
{
    if((!_campaign) || (!event) || (!event->mimeData()))
        return;

    if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void SoundBoardGroupFrame::dropEvent(QDropEvent *event)
{
    if((!_campaign) || (!event) || (!event->mimeData()))
        return;

    if(!event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
        return;

    QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    while(!stream.atEnd())
    {
        int row, col;
        QMap<int,  QVariant> roleDataMap;
        stream >> row >> col >> roleDataMap;
        if(roleDataMap.contains(Qt::UserRole))
        {
            QUuid trackId(roleDataMap.value(Qt::UserRole).toString());
            AudioTrack* track = _campaign->getTrackById(trackId);
            if(track)
                addTrack(new SoundboardTrack(track, 100, false, _group));
        }
    }

    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void SoundBoardGroupFrame::toggleContents()
{
    bool newVisible = !ui->frameWidgets->isVisible();
    ui->frameWidgets->setVisible(newVisible);

    QPixmap expandIcon(":/img/data/icon_downarrow.png");
    if(newVisible)
        expandIcon = expandIcon.transformed(QTransform().rotate(180));
    ui->btnExpand->setIcon(QIcon(expandIcon));
}

void SoundBoardGroupFrame::toggleMute()
{
    bool newMute = ui->btnMute->isChecked();
    setMute(newMute);
    _localMute = newMute;
    if(_group)
        _group->setMute(newMute);
    emit muteChanged(newMute);
    emit dirty();
}

void SoundBoardGroupFrame::addTrackToLayout(SoundboardTrack* track)
{
    SoundboardTrackFrame* trackFrame = new SoundboardTrackFrame(track);
    connect(this, &SoundBoardGroupFrame::muteChanged, trackFrame, &SoundboardTrackFrame::parentMuteChanged);
    connect(this, &SoundBoardGroupFrame::overrideChildMute, trackFrame, &SoundboardTrackFrame::setMute);
    connect(trackFrame, &SoundboardTrackFrame::muteChanged, this, &SoundBoardGroupFrame::trackMuteChanged);
    connect(trackFrame, &SoundboardTrackFrame::removeTrack, this, &SoundBoardGroupFrame::removeTrack);
    _trackWidgets.append(trackFrame);
    updateTrackLayout();
}
