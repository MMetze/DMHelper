#include "soundboardframe.h"
#include "ui_soundboardframe.h"
#include "campaign.h"
#include "dmconstants.h"
#include "soundboardgroup.h"
#include "soundboardgroupframe.h"
#include "audiotrack.h"
#include "audiofactory.h"
#include "ribbonframe.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QDrag>
#include <QFileInfo>
#include <QMouseEvent>

SoundboardFrame::SoundboardFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SoundboardFrame),
    _layout(nullptr),
    _groupList(),
    _mouseDown(false),
    _mouseDownPos(),
    _campaign(nullptr)
{
    ui->setupUi(this);
    _layout = new QVBoxLayout();
    ui->scrollAreaWidgetContents->setLayout(_layout);

    ui->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeWidget->setDragEnabled(true);
    ui->treeWidget->viewport()->setAcceptDrops(true);
    ui->treeWidget->setDropIndicatorShown(true);
    ui->treeWidget->setDragDropMode(QAbstractItemView::InternalMove);

    QSizePolicy policy = ui->scrollArea->sizePolicy();
    policy.setHorizontalStretch(10);
    policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    ui->scrollArea->setSizePolicy(policy);
    policy = ui->treeWidget->sizePolicy();
    policy.setHorizontalStretch(1);
    policy.setHorizontalPolicy(QSizePolicy::Preferred);
    ui->treeWidget->setSizePolicy(policy);

    connect(ui->btnAddGroup, &QAbstractButton::clicked, this, &SoundboardFrame::addGroup);
    connect(ui->btnAddSound, &QAbstractButton::clicked, this, &SoundboardFrame::addSound);
    connect(ui->btnAddYoutube, &QAbstractButton::clicked, this, &SoundboardFrame::addYoutube);
    connect(ui->btnAddSyrinscape, &QAbstractButton::clicked, this, &SoundboardFrame::addSyrinscape);
}

SoundboardFrame::~SoundboardFrame()
{
    delete ui;
}

void SoundboardFrame::setCampaign(Campaign* campaign)
{
    ui->treeWidget->clear();

    QLayoutItem *child;
    while((child = _layout->takeAt(0)) != nullptr)
    {
        SoundBoardGroupFrame* groupFrame = dynamic_cast<SoundBoardGroupFrame*>(child->widget());
        if(groupFrame)
        {
            disconnect(groupFrame->getGroup(), &SoundboardGroup::destroyed, groupFrame, &SoundBoardGroupFrame::handleRemove);
            delete groupFrame;
        }
        delete child;
    }

    _campaign = campaign;

    if(!campaign)
        return;

    _layout->addStretch(10);
    for(SoundboardGroup* group : campaign->getSoundboardGroups())
    {
        if(group)
            addGroupToLayout(group);
    }

    QList<CampaignObjectBase*> tracks = campaign->getChildObjectsByType(DMHelper::CampaignType_AudioTrack);
    for(CampaignObjectBase* trackObject : tracks)
    {
        addTrackToTree(dynamic_cast<AudioTrack*>(trackObject));
    }

    updateTrackLayout();

    ui->treeWidget->setMinimumWidth(ui->treeWidget->sizeHint().width());
}

void SoundboardFrame::addTrackToTree(AudioTrack* track)
{
    if(!track)
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(track->getName()));
    item->setData(0, Qt::UserRole, track->getID().toString());
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    switch(track->getAudioType())
    {
        case DMHelper::AudioType_Syrinscape:
            item->setIcon(0, QIcon(QString(":/img/data/icon_syrinscape.png")));
            break;
        case DMHelper::AudioType_Youtube:
            item->setIcon(0, QIcon(QString(":/img/data/icon_playerswindow.png")));
            break;
        default:
            item->setIcon(0, QIcon(QString(":/img/data/icon_soundboard.png")));
            break;
    };

    ui->treeWidget->addTopLevelItem(item);
}

void SoundboardFrame::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    updateTrackLayout();
}

void SoundboardFrame::showEvent(QShowEvent *event)
{
    QFrame::showEvent(event);
    updateTrackLayout();

    int ribbonHeight = RibbonFrame::getRibbonHeight();
    if(ribbonHeight > 0)
    {
        ui->frame->setMinimumHeight(ribbonHeight);
        ui->frame->setMaximumHeight(ribbonHeight);
        ui->frame->resize(width(), ribbonHeight);
    }

    RibbonFrame::setStandardButtonSize(*ui->lblAddGroup, *ui->btnAddGroup, ribbonHeight);
    RibbonFrame::setLineHeight(*ui->line, ribbonHeight);
    RibbonFrame::setStandardButtonSize(*ui->lblAddSound, *ui->btnAddSound, ribbonHeight);
    RibbonFrame::setStandardButtonSize(*ui->lblAddYoutube, *ui->btnAddYoutube, ribbonHeight);
    RibbonFrame::setStandardButtonSize(*ui->lblAddSyrinscape, *ui->btnAddSyrinscape, ribbonHeight);

}

void SoundboardFrame::updateTrackLayout()
{
    if(!_layout)
        return;

    for(int i = 0; i < _layout->count() - 1; ++i)
    {
        QLayoutItem* item = _layout->itemAt(i);
        if(item)
        {
            SoundBoardGroupFrame* group = dynamic_cast<SoundBoardGroupFrame*>(item->widget());
            if(group)
                group->updateTrackLayout();
        }
    }

    ui->scrollArea->update();
}

void SoundboardFrame::addGroup()
{
    if(!_campaign)
        return;

    bool ok = false;
    QString groupName = QInputDialog::getText(this,
                                              QString("Enter Group Name"),
                                              QString("Please enter a name for the new group"),
                                              QLineEdit::Normal,
                                              QString("Sound Effects"),
                                              &ok);
    if((!ok) || (groupName.isEmpty()))
        return;

    SoundboardGroup* newGroup = new SoundboardGroup(groupName);
    _campaign->addSoundboardGroup(newGroup);
    addGroupToLayout(newGroup);
}

void SoundboardFrame::removeGroup(SoundboardGroup* group)
{
    if((!group) || (!_layout) || (!_campaign))
        return;

    for(int i = 0; i < _layout->count() - 1; ++i)
    {
        QLayoutItem* item = _layout->itemAt(i);
        if(item)
        {
            SoundBoardGroupFrame* groupFrame = dynamic_cast<SoundBoardGroupFrame*>(item->widget());
            if((groupFrame) && (groupFrame->getGroup() == group))
            {
                disconnect(group, &SoundboardGroup::destroyed, groupFrame, &SoundBoardGroupFrame::handleRemove);
                delete groupFrame;
            }
        }
    }

    _campaign->removeSoundboardGroup(group);
}

void SoundboardFrame::addSound()
{
    if(!_campaign)
        return;

    QString filename = QFileDialog::getOpenFileName(this, QString("Select music file"));
    if(!filename.isEmpty())
        addTrack(QUrl(filename));
}

void SoundboardFrame::addYoutube()
{
    if(!_campaign)
        return;

    QString youtubeInstructions("To add a YouTube video as an audio file, paste the link/URL into the text box here:\n");

    bool ok;
    QString urlName = QInputDialog::getText(this, QString("Enter Syrinscape Audio URI"), youtubeInstructions, QLineEdit::Normal, QString(), &ok);
    if((!ok)||(urlName.isEmpty()))
        return;

    addTrack(QUrl(urlName));
}

void SoundboardFrame::addSyrinscape()
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

void SoundboardFrame::addTrack(const QUrl& url)
{
    if((!_campaign) || (!url.isValid()))
        return;

    QFileInfo fileInfo(url.path());
    bool ok;
    QString trackName = QInputDialog::getText(this, QString("Enter track name"), QString("New Track Name"), QLineEdit::Normal, fileInfo.baseName(), &ok);
    if((!ok)||(trackName.isEmpty()))
        return;

    AudioTrack* newTrack = AudioFactory().createTrackFromUrl(url, trackName);

    emit trackCreated(newTrack);
    addTrackToTree(newTrack);
}

void SoundboardFrame::addGroupToLayout(SoundboardGroup* group)
{
    if((!_campaign) || (!_layout) || (!group))
        return;

    SoundBoardGroupFrame* newGroupBox = new SoundBoardGroupFrame(group, _campaign);
    connect(newGroupBox, &SoundBoardGroupFrame::dirty, this, &SoundboardFrame::dirty);
    connect(newGroupBox, &SoundBoardGroupFrame::removeGroup, this, &SoundboardFrame::removeGroup);
    _layout->insertWidget(_layout->count() - 1, newGroupBox);
}
