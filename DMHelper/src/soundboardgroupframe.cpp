#include "soundboardgroupframe.h"
#include "ui_soundboardgroupframe.h"
#include "soundboardtrackframe.h"
#include "soundboardgroup.h"
#include "campaign.h"
#include <QGridLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUuid>
#include <QDebug>

SoundBoardGroupFrame::SoundBoardGroupFrame(SoundboardGroup* group, Campaign* campaign, QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::SoundBoardGroupFrame),
    _groupLayout(nullptr),
    _trackWidgets(),
    _localMute(false),
    _group(group),
    _campaign(campaign)
{
    ui->setupUi(this);

    setAcceptDrops(true);

    setTitle(group->getGroupName());
    _groupLayout = new QGridLayout();
    setLayout(_groupLayout);
    _groupLayout->setSpacing(20);
    _groupLayout->setHorizontalSpacing(20);
    _groupLayout->setVerticalSpacing(20);

    ui->btnExpand->setIcon(QIcon(QPixmap(":/img/data/icon_downarrow.png").transformed(QTransform().rotate(180))));

    connect(ui->btnExpand, &QPushButton::clicked, this, &SoundBoardGroupFrame::toggleContents);
    connect(ui->btnVolume, &QPushButton::clicked, this, &SoundBoardGroupFrame::toggleMute);

    if(_group)
    {
        for(AudioTrack* track : _group->getTracks())
        {
            if(track)
                addTrackToLayout(track);
        }

        connect(_group, &SoundboardGroup::destroyed, this, &SoundBoardGroupFrame::handleRemove);
    }
}

SoundBoardGroupFrame::~SoundBoardGroupFrame()
{
    qDeleteAll(_trackWidgets);

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

    if((_trackWidgets.at(0)->width() + _groupLayout->horizontalSpacing()) <= 0)
        return;

    while(_groupLayout->count() > 0)
    {
        _groupLayout->takeAt(0);
    }

    int xCount = (width() - _groupLayout->contentsMargins().left() - _groupLayout->contentsMargins().right()) /
                 (_trackWidgets.at(0)->width() + _groupLayout->horizontalSpacing());

    int x = 0;
    int y = 0;

    _groupLayout->setRowStretch(0,0);
    for(SoundboardTrackFrame* trackWidget : _trackWidgets)
    {
        _groupLayout->addWidget(trackWidget, y, x);
        if(++x >= xCount)
        {
            x = 0;
            ++y;
            _groupLayout->setRowStretch(y,0);
        }
    }

    _groupLayout->setRowStretch(y+1,1);
    _groupLayout->setColumnStretch(xCount,1);

    update();
}

void SoundBoardGroupFrame::addTrack(AudioTrack* track)
{
    if((!_group) || (!track))
        return;

    _group->addTrack(track);
    addTrackToLayout(track);
    emit dirty();
}

void SoundBoardGroupFrame::removeTrack(AudioTrack* track)
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
    ui->btnVolume->setIcon(mute ? QIcon(QPixmap(":/img/data/icon_volumeoff.png")) : QIcon(QPixmap(":/img/data/icon_volumeon.png")));
    ui->sliderVolume->setEnabled(!mute);
    ui->btnVolume->setChecked(mute);
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
                addTrack(track);
        }
    }

    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void SoundBoardGroupFrame::toggleContents()
{
    if((_groupLayout->count() <= 0) ||
       (_groupLayout->itemAt(0) == nullptr) ||
       (_groupLayout->itemAt(0)->widget() == nullptr))
        return;

    bool newVisible = !_groupLayout->itemAt(0)->widget()->isVisible();
    for(int i = 0; i < _groupLayout->count(); ++i)
    {
        QLayoutItem* item = _groupLayout->itemAt(i);
        if((item) && (item->widget()))
            item->widget()->setVisible(newVisible);
    }

    QPixmap expandIcon(":/img/data/icon_downarrow.png");
    if(newVisible)
        expandIcon = expandIcon.transformed(QTransform().rotate(180));
    ui->btnExpand->setIcon(QIcon(expandIcon));
}

void SoundBoardGroupFrame::toggleMute()
{
    bool newMute = ui->btnVolume->isChecked();
    setMute(newMute);
    _localMute = newMute;
    emit muteChanged(newMute);
    emit dirty();
}

void SoundBoardGroupFrame::addTrackToLayout(AudioTrack* track)
{
    SoundboardTrackFrame* trackFrame = new SoundboardTrackFrame(track);
    connect(this, &SoundBoardGroupFrame::muteChanged, trackFrame, &SoundboardTrackFrame::parentMuteChanged);
    connect(this, &SoundBoardGroupFrame::overrideChildMute, trackFrame, &SoundboardTrackFrame::setMute);
    connect(trackFrame, &SoundboardTrackFrame::muteChanged, this, &SoundBoardGroupFrame::trackMuteChanged);
    connect(trackFrame, &SoundboardTrackFrame::removeTrack, this, &SoundBoardGroupFrame::removeTrack);
    _trackWidgets.append(trackFrame);
    updateTrackLayout();
}
