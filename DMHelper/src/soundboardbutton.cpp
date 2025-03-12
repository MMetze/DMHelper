#include "soundboardbutton.h"
#include "audiotrack.h"
#include "campaign.h"
#include "dmconstants.h"
#include <QMouseEvent>
#include <QMenu>
#include <QDirIterator>
#include <QDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QPainter>
#include <QMimeData>

const int SOUNDBOARDBUTTON_ICONSIZE = 32;
const int SOUNDBOARDBUTTON_BUTTONSIZE = 36;

SoundboardButton::SoundboardButton(Campaign* campaign, const QString& iconResource, AudioTrack* track, QWidget *parent) :
    QPushButton(parent),
    _campaign(campaign),
    _iconResource(iconResource),
    _track(nullptr)
{
    setAcceptDrops(true);

    setTrack(track);
    setText(QString());
    setIconSize(QSize(SOUNDBOARDBUTTON_ICONSIZE, SOUNDBOARDBUTTON_ICONSIZE));
    setFixedSize(SOUNDBOARDBUTTON_BUTTONSIZE, SOUNDBOARDBUTTON_BUTTONSIZE);
    updateIcon(false);

    connect(this, &QPushButton::clicked, this, &SoundboardButton::handleClick);
}

SoundboardButton::~SoundboardButton()
{
}

QString SoundboardButton::getIconResource() const
{
    return _iconResource;
}

AudioTrack* SoundboardButton::getTrack() const
{
    return _track;
}

void SoundboardButton::setIconResource(const QString& iconResource)
{
    if(_iconResource != iconResource)
    {
        _iconResource = iconResource;
        updateIcon(false);
        emit iconResourceChanged(_iconResource);
    }
}

void SoundboardButton::setTrack(AudioTrack* track)
{
    if(_track != track)
    {
        disconnect(_track, &AudioTrack::playingChanged, this, &SoundboardButton::playingChanged);
        _track = track;
        if(_track)
            connect(_track, &AudioTrack::playingChanged, this, &SoundboardButton::playingChanged);

        updateIcon(false);
        emit trackChanged(_track);
    }
}

void SoundboardButton::handleClick()
{
    if(_track)
    {
        if(_track->isPlaying())
        {
            _track->stop();
        }
        else
        {
            _track->setRepeat(false);
            _track->play();
        }
        updateIcon(_track->isPlaying());
    }
}

void SoundboardButton::playingChanged(bool playing)
{
    Q_UNUSED(playing);
    updateIcon(playing);
}

void SoundboardButton::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
        showContextMenu(event->pos());
    else
        QPushButton::mousePressEvent(event);
}

void SoundboardButton::dragEnterEvent(QDragEnterEvent *event)
{
    if((!event) || (!event->mimeData()))
        return;

    if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void SoundboardButton::dropEvent(QDropEvent *event)
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
                setTrack(track);
        }
    }

    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void SoundboardButton::showContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    QAction *changeIconAction = new QAction("Change Icon...", this);
    connect(changeIconAction, &QAction::triggered, this, &SoundboardButton::changeIcon);
    menu.addAction(changeIconAction);

    if(_campaign)
    {
        QAction *changeTrackAction = new QAction("Change Track...", this);
        connect(changeTrackAction, &QAction::triggered, this, &SoundboardButton::changeTrack);
        menu.addAction(changeTrackAction);
    }

    menu.exec(mapToGlobal(pos));
}

void SoundboardButton::changeIcon()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Select Icon");
    QVBoxLayout layout(&dialog);

    QListWidget listWidget;
    QDirIterator it(":/img/data/img/", QStringList("*.png"), QDir::Files);
    while(it.hasNext())
    {
        QString filePath = it.next();
        QListWidgetItem *item = new QListWidgetItem(QIcon(filePath), QFileInfo(filePath).fileName());
        item->setData(Qt::UserRole, filePath);
        listWidget.addItem(item);
    }
    layout.addWidget(&listWidget);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout.addWidget(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(&listWidget, &QListWidget::itemDoubleClicked, [&](QListWidgetItem *item){ Q_UNUSED(item); dialog.accept(); });

    if(dialog.exec() == QDialog::Accepted)
    {
        QListWidgetItem *selectedItem = listWidget.currentItem();
        if(selectedItem)
            setIconResource(selectedItem->data(Qt::UserRole).toString());
    }
}

void SoundboardButton::changeTrack()
{
    if(!_campaign)
        return;

    QDialog dialog(this);
    dialog.setWindowTitle("Select Track");
    QVBoxLayout layout(&dialog);

    QListWidget listWidget;
    QList<CampaignObjectBase*> tracks = _campaign->getChildObjectsByType(DMHelper::CampaignType_AudioTrack);
    for(CampaignObjectBase* trackObject : tracks)
    {
        AudioTrack* track = dynamic_cast<AudioTrack*>(trackObject);
        if(!track)
            continue;

        QListWidgetItem* item = new QListWidgetItem(track->getName());
        item->setData(Qt::UserRole, QVariant::fromValue(track));
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
        switch(track->getAudioType())
        {
        case DMHelper::AudioType_Syrinscape:
            item->setIcon(QIcon(QString(":/img/data/icon_syrinscape.png")));
            break;
        case DMHelper::AudioType_Youtube:
            item->setIcon(QIcon(QString(":/img/data/icon_playerswindow.png")));
            break;
        default:
            item->setIcon(QIcon(QString(":/img/data/icon_soundboard.png")));
            break;
        };
        listWidget.addItem(item);
    }

    layout.addWidget(&listWidget);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout.addWidget(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(&listWidget, &QListWidget::itemDoubleClicked, [&](QListWidgetItem *item){ Q_UNUSED(item); dialog.accept(); });

    if(dialog.exec() == QDialog::Accepted)
    {
        AudioTrack* newTrack = listWidget.currentItem() ? listWidget.currentItem()->data(Qt::UserRole).value<AudioTrack*>() : nullptr;
        if(newTrack)
            setTrack(newTrack);
    }
}

void SoundboardButton::updateIcon(bool playing)
{
    if(!_track)
    {
        setIcon(QIcon(QString(":/img/data/icon_select.png")));
    }
    else
    {
        QPixmap basePixmap(_iconResource);
        if(basePixmap.isNull())
            basePixmap = QPixmap(":/img/data/icon_soundboard.png");
        if(playing)
        {
            QPixmap overlayPixmap(":/img/data/icon_play.png");
            QPainter painter(&basePixmap);
            int overlaySize = basePixmap.width() / 2;
            QRect overlayRect(basePixmap.width() - overlaySize, basePixmap.height() - overlaySize, overlaySize, overlaySize);
            painter.drawPixmap(overlayRect, overlayPixmap);
        }
        setIcon(QIcon(basePixmap));
    }
}
