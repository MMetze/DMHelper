#include "soundboardframegroupbox.h"
#include "ui_soundboardframegroupbox.h"
#include "soundboardtrack.h"
#include "campaign.h"
#include <QGridLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUuid>
#include <QDebug>

SoundBoardFrameGroupBox::SoundBoardFrameGroupBox(const QString& groupName, Campaign* campaign, QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::SoundBoardFrameGroupBox),
    _groupLayout(nullptr),
    _trackWidgets(),
    _localMute(false),
    _campaign(campaign)
{
    ui->setupUi(this);

    setAcceptDrops(true);

    setTitle(groupName);
    _groupLayout = new QGridLayout();
    setLayout(_groupLayout);
    _groupLayout->setSpacing(20);
    _groupLayout->setHorizontalSpacing(20);
    _groupLayout->setVerticalSpacing(20);

    ui->btnExpand->setIcon(QIcon(QPixmap(":/img/data/icon_downarrow.png").transformed(QTransform().rotate(180))));

    connect(ui->btnExpand, &QPushButton::clicked, this, &SoundBoardFrameGroupBox::toggleContents);
    connect(ui->btnVolume, &QPushButton::clicked, this, &SoundBoardFrameGroupBox::toggleMute);
}

SoundBoardFrameGroupBox::~SoundBoardFrameGroupBox()
{
    delete ui;
}

bool SoundBoardFrameGroupBox::isMuted() const
{
    return _localMute;
}

int SoundBoardFrameGroupBox::getVolume() const
{
    return _localMute ? 0 : ui->sliderVolume->value();
}

void SoundBoardFrameGroupBox::updateTrackLayout()
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
    for(SoundboardTrack* trackWidget : _trackWidgets)
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
}

void SoundBoardFrameGroupBox::clearTracks()
{
    while(_groupLayout->count() > 0)
    {
        _groupLayout->takeAt(0);
    }

    qDeleteAll(_trackWidgets);
    _trackWidgets.clear();
}

void SoundBoardFrameGroupBox::addTrack(SoundboardTrack* track)
{
    if(track)
    {
        connect(this, &SoundBoardFrameGroupBox::muteChanged, track, &SoundboardTrack::parentMuteChanged);
        connect(this, &SoundBoardFrameGroupBox::overrideChildMute, track, &SoundboardTrack::setMute);
        connect(track, &SoundboardTrack::muteChanged, this, &SoundBoardFrameGroupBox::trackMuteChanged);
        _trackWidgets.append(track);
        updateTrackLayout();
    }
}

void SoundBoardFrameGroupBox::setMute(bool mute)
{
    ui->btnVolume->setIcon(mute ? QIcon(QPixmap(":/img/data/icon_volumeoff.png")) : QIcon(QPixmap(":/img/data/icon_volumeon.png")));
    ui->sliderVolume->setEnabled(!mute);
    ui->btnVolume->setChecked(mute);
}

void SoundBoardFrameGroupBox::trackMuteChanged(bool mute)
{
    if((_localMute)&&(!mute))
    {
        setMute(false);
        _localMute = false;
        emit overrideChildMute(true);
    }
}

void SoundBoardFrameGroupBox::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    toggleContents();
}

void SoundBoardFrameGroupBox::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    ui->frameControls->move(width() - ui->frameControls->width(), 0);
}

void SoundBoardFrameGroupBox::dragEnterEvent(QDragEnterEvent *event)
{
    if((!_campaign) || (!event) || (!event->mimeData()))
        return;

    if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void SoundBoardFrameGroupBox::dropEvent(QDropEvent *event)
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
                addTrack(new SoundboardTrack(track));
        }
    }

    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void SoundBoardFrameGroupBox::toggleContents()
{
    if((_groupLayout->count() > 0) ||
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

void SoundBoardFrameGroupBox::toggleMute()
{
    bool newMute = ui->btnVolume->isChecked();
    setMute(newMute);
    _localMute = newMute;
    emit muteChanged(newMute);
}
