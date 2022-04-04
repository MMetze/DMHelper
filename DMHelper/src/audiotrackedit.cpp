#include "audiotrackedit.h"
#include "ui_audiotrackedit.h"
#include "audiotrack.h"
#include "dmconstants.h"
//#include "campaign.h"
//#include "audiofactory.h"
//#include <QInputDialog>
//#include <QFileInfo>
//#include <QFileDialog>
#include <QTimer>
#include <QDebug>

AudioTrackEdit::AudioTrackEdit(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::AudioTrackEdit),
//    _campaign(nullptr)
    _track(nullptr)
{
    ui->setupUi(this);
//    enableButtons(false);

//    connect(ui->btnAddLocal, SIGNAL(clicked(bool)), this, SLOT(addLocalFile()));
//    connect(ui->btnAddURL, SIGNAL(clicked(bool)), this, SLOT(addGlobalUrl()));
//    connect(ui->btnAddSyrinscape, SIGNAL(clicked(bool)), this, SLOT(addSyrinscape()));
//    connect(ui->btnRemove, SIGNAL(clicked(bool)), this, SLOT(removeTrack()));
//    connect(ui->lstTracks, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(itemSelected(QListWidgetItem *)));

//#ifdef Q_OS_MAC
//    ui->btnAddSyrinscape->hide();
//#endif
}

AudioTrackEdit::~AudioTrackEdit()
{
    delete ui;
}

void AudioTrackEdit::activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer)
{
    Q_UNUSED(currentRenderer);

    AudioTrack* track = dynamic_cast<AudioTrack*>(object);
    if(!track)
        return;

    if(_track != nullptr)
    {
        qDebug() << "[AudioTrackEdit] ERROR: New audio track activated without deactivating the previous track object first!";
        deactivateObject();
    }

    setTrack(track);
}

void AudioTrackEdit::deactivateObject()
{
    if(!_track)
    {
        qDebug() << "[AudioTrackEdit] WARNING: Invalid (nullptr) track object deactivated!";
        return;
    }

    setTrack(nullptr);
}

void AudioTrackEdit::setTrack(AudioTrack* track)
{
    _track = track;

    if(_track)
    {
        emit trackTypeChanged(_track->getAudioType());
        emit playChanged(_track->isPlaying());
        emit repeatChanged(_track->isRepeat());
        emit muteChanged(_track->isMuted());
        emit volumeChanged(_track->getVolume());
    }
}

void AudioTrackEdit::setPlay(bool checked)
{
    if(!_track)
        return;

    if(checked)
    {
        _track->play();

        if(_track->getAudioType() == DMHelper::AudioType_Syrinscape)
            QTimer::singleShot(500, this, &AudioTrackEdit::unclick);
    }
    else
    {
        _track->stop();
    }
}

void AudioTrackEdit::setRepeat(bool checked)
{
    if(_track)
        _track->setRepeat(checked);
}

void AudioTrackEdit::setMute(bool checked)
{
    if(_track)
        _track->setMute(checked);
}

void AudioTrackEdit::setVolume(int volume)
{
    if(_track)
        _track->setVolume(volume);
}

void AudioTrackEdit::unclick()
{
    emit playChanged(false);
}


/*
void AudioTrackEdit::addTrack(const QUrl& url)
{
    if((!_campaign) || (!url.isValid()))
        return;

    QFileInfo fileInfo(url.path());
    bool ok;
    QString trackName = QInputDialog::getText(this, QString("Enter track name"), QString("New Track Name"), QLineEdit::Normal, fileInfo.baseName(), &ok);
    if((!ok)||(trackName.isEmpty()))
        return;

    AudioTrack* newTrack = AudioFactory().createTrackFromUrl(url, trackName);

    _campaign->addObject(newTrack);
    addTrackToList(newTrack);
}

void AudioTrackEdit::removeTrack()
{
    // TODO: is this even needed?
//    if(!_campaign)
//        return;

//    QListWidgetItem* currentItem = ui->lstTracks->currentItem();
//    if(!currentItem)
//        return;

//    AudioTrack* removeTrack = getCurrentTrack();
//    if(!removeTrack)
//        return;

    //_campaign->removeTrack(removeTrack->getID());

//    delete removeTrack;
//    delete currentItem;
}

void AudioTrackEdit::setCampaign(Campaign* campaign)
{
    ui->lstTracks->clear();
    _campaign = campaign;

    enableButtons(_campaign != nullptr);

    if(!_campaign)
        return;

    QList<AudioTrack*> trackList = _campaign->findChildren<AudioTrack*>();
//    for(int i = 0; i < _campaign->getTrackCount(); ++i)
//        addTrackToList(_campaign->getTrackByIndex(i));
    for(AudioTrack* track : trackList)
    {
        addTrackToList(track);
    }
}

void AudioTrackEdit::addLocalFile()
{
    if(!_campaign)
        return;

    QString filename = QFileDialog::getOpenFileName(this,QString("Select music file.."));
    if(!filename.isEmpty())
        addTrack(QUrl(filename));
}

void AudioTrackEdit::addGlobalUrl()
{
    if(!_campaign)
        return;

    QUrl url = QFileDialog::getOpenFileUrl(this,QString("Select music file.."));
    if(!url.isEmpty())
        addTrack(url);
}

void AudioTrackEdit::addSyrinscape()
{
    if(!_campaign)
        return;

    QString syrinscapeInstructions("To add a link to a Syrinscape sound:\n\n1) Hit the '+' key or select ""3rd party app integration"" ENABLE in the settings menu\n2) Little pluses will appear next to all the MOODs and OneShots\n3) Click one of these pluses to copy a URI shortcut to the clipboard\n4) Paste this URI into the text box here:\n");

    bool ok;
    QString urlName = QInputDialog::getText(this, QString("Enter Syrinscape Audio URI"), syrinscapeInstructions, QLineEdit::Normal, QString(), &ok);
    if((!ok)||(urlName.isEmpty()))
        return;

    addTrack(QUrl(urlName));
}

void AudioTrackEdit::itemSelected(QListWidgetItem *item)
{
    if(!item)
        return;

    AudioTrack* track = item->data(Qt::UserRole).value<AudioTrack*>();
    if(!track)
        return;

    emit trackSelected(track);

    // TODO: play something...
    //setPlayerEnabled(true);
    //ui->lblCurrent->setText(track->getName());
    //ui->sliderPlayback->setValue(0);
    //ui->btnPlay->setChecked(false);
    //_player->setMedia(QMediaContent(track->getUrl()));
}

AudioTrack* AudioTrackEdit::getCurrentTrack()
{
    QListWidgetItem* currentItem = ui->lstTracks->currentItem();
    return currentItem ? currentItem->data(Qt::UserRole).value<AudioTrack*>() : nullptr;
}

void AudioTrackEdit::addTrackToList(AudioTrack* track)
{
    if(!track)
        return;

    QListWidgetItem* newListItem = new QListWidgetItem(track->getName());
    newListItem->setData(Qt::UserRole, QVariant::fromValue(track));
    ui->lstTracks->addItem(newListItem);
}

void AudioTrackEdit::enableButtons(bool enable)
{
    ui->btnAddLocal->setEnabled(enable);
    ui->btnAddURL->setEnabled(enable);
    ui->btnRemove->setEnabled(enable);
}
*/
