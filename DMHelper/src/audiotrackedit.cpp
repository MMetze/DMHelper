#include "audiotrackedit.h"
#include "ui_audiotrackedit.h"
#include "audiotrack.h"
#include "campaign.h"
#include <QInputDialog>
#include <QFileInfo>
#include <QFileDialog>



AudioTrackEdit::AudioTrackEdit(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AudioTrackEdit),
    _campaign(nullptr)
{
    ui->setupUi(this);
    enableButtons(false);

    connect(ui->btnAddLocal, SIGNAL(clicked(bool)), this, SLOT(addLocalFile()));
    connect(ui->btnAddURL, SIGNAL(clicked(bool)), this, SLOT(addGlobalUrl()));
    connect(ui->btnAddSyrinscape, SIGNAL(clicked(bool)), this, SLOT(addSyrinscape()));
    connect(ui->btnRemove, SIGNAL(clicked(bool)), this, SLOT(removeTrack()));
    connect(ui->lstTracks, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(itemSelected(QListWidgetItem *)));

#ifdef Q_OS_MAC
    ui->btnAddSyrinscape->hide();
#endif
}

AudioTrackEdit::~AudioTrackEdit()
{
    delete ui;
}

void AudioTrackEdit::addTrack(const QUrl& url)
{
    if((!_campaign) || (!url.isValid()))
        return;

    QFileInfo fileInfo(url.path());
    bool ok;
    QString trackName = QInputDialog::getText(this, QString("Enter track name"), QString("New Track Name"), QLineEdit::Normal, fileInfo.baseName(), &ok);
    if((!ok)||(trackName.isEmpty()))
        return;

    AudioTrack* newTrack = new AudioTrack(trackName, url);
    _campaign->addTrack(newTrack);
    addTrackToList(newTrack);
}

void AudioTrackEdit::removeTrack()
{
    if(!_campaign)
        return;

    QListWidgetItem* currentItem = ui->lstTracks->currentItem();
    if(!currentItem)
        return;

    AudioTrack* removeTrack = getCurrentTrack();
    if(!removeTrack)
        return;

    _campaign->removeTrack(removeTrack->getID());
    delete removeTrack;
    delete currentItem;
}

void AudioTrackEdit::setCampaign(Campaign* campaign)
{
    ui->lstTracks->clear();
    _campaign = campaign;

    enableButtons(_campaign != nullptr);

    if(!_campaign)
        return;

    for(int i = 0; i < _campaign->getTrackCount(); ++i)
        addTrackToList(_campaign->getTrackByIndex(i));
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

    /*
     * TODO: play something...
    setPlayerEnabled(true);
    ui->lblCurrent->setText(track->getName());
    ui->sliderPlayback->setValue(0);
    ui->btnPlay->setChecked(false);
    _player->setMedia(QMediaContent(track->getUrl()));
    */
}

AudioTrack* AudioTrackEdit::getCurrentTrack()
{
    QListWidgetItem* currentItem = ui->lstTracks->currentItem();
    return currentItem ? currentItem->data(Qt::UserRole).value<AudioTrack*>() : nullptr;
}

void AudioTrackEdit::addTrackToList(AudioTrack* track)
{
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
