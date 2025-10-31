#include "popupspreviewframe.h"
#include "ui_popupspreviewframe.h"
#include "campaign.h"
#include "overlay.h"
#include "audiotrackfile.h"
#include "popupaudio.h"

PopupsPreviewFrame::PopupsPreviewFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PopupsPreviewFrame),
    _campaign(nullptr),
    _popups()
{
    ui->setupUi(this);

    QVBoxLayout* popupsLayout = new QVBoxLayout;
    popupsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    popupsLayout->setContentsMargins(3, 3, 3, 3);
    popupsLayout->setSpacing(3);
    ui->scrollAreaWidgetContents->setLayout(popupsLayout);
    ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    ui->dividerLayout->setContentsMargins(0, 0, 0, 0);
    ui->dividerLayout->setAlignment(Qt::AlignCenter);
    ui->btnCollapse->setArrowType(Qt::DownArrow);
    connect(ui->btnCollapse, &QToolButton::clicked, this, &PopupsPreviewFrame::toggleOverlayFrame);

    ui->frameCollapse->setVisible(false);
    ui->scrollPopups->setVisible(false);
}

PopupsPreviewFrame::~PopupsPreviewFrame()
{
    clearPopups();
    delete ui;
}

void PopupsPreviewFrame::setCampaign(Campaign* campaign)
{
    if(_campaign == campaign)
        return;

    disconnect(_campaign, nullptr, this, nullptr);
    _campaign = campaign;

    populatePopups();

    connect(_campaign, &Campaign::overlaysChanged, this, &PopupsPreviewFrame::populatePopups);
}

void PopupsPreviewFrame::trackAdded(CampaignObjectBase* trackObject)
{
    AudioTrackFile* track = dynamic_cast<AudioTrackFile*>(trackObject);
    if(!track)
        return;

    connect(track, &AudioTrack::trackStatusChanged, this, &PopupsPreviewFrame::audioPopupChanged);
}

void PopupsPreviewFrame::toggleOverlayFrame()
{
    ui->scrollPopups->setVisible(!ui->scrollPopups->isVisible());
    ui->btnCollapse->setArrowType(ui->scrollPopups->isVisible() ? Qt::DownArrow : Qt::UpArrow);
}

void PopupsPreviewFrame::populatePopups()
{
    clearPopups();

    if(!_campaign)
        return;

    QBoxLayout* popupLayout = dynamic_cast<QBoxLayout*>(ui->scrollAreaWidgetContents->layout());
    if(!popupLayout)
        return;

    QList<Overlay*> overlays = _campaign->getOverlays();
    for(Overlay* overlay : overlays)
    {
        if(overlay)
        {
            QFrame* overlayFrame = new QFrame();
            QHBoxLayout* frameLayout = new QHBoxLayout();
            frameLayout->setContentsMargins(0, 0, 0, 0);
            frameLayout->setSpacing(0);
            overlayFrame->setLayout(frameLayout);
            overlayFrame->setFrameStyle(QFrame::Box);
            overlay->prepareFrame(frameLayout, 0);
            popupLayout->addWidget(overlayFrame);
        }
    }

    updateFrameVisibility();
}

void PopupsPreviewFrame::clearPopups()
{
    if(!ui->scrollAreaWidgetContents->layout())
        return;

    QLayoutItem *child;
    while ((child = ui->scrollAreaWidgetContents->layout()->takeAt(0)) != nullptr)
    {
        if(child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    updateFrameVisibility();
}

void PopupsPreviewFrame::audioPopupChanged(AudioTrack::AudioTrackStatus newState)
{
    AudioTrackFile* track = dynamic_cast<AudioTrackFile*>(sender());
    if(!track)
        return;

    // Find if there is a popup in the hash that has this track
    PopupAudio* popup = nullptr;
    for(Popup* p : _popups.keys())
    {
        PopupAudio* audioPopup = dynamic_cast<PopupAudio*>(p);
        if((audioPopup) && (audioPopup->getTrack() == track))
        {
            popup = audioPopup;
            break;
        }
    }

    if(popup)
    {
        if(newState == AudioTrack::AudioTrackStatus_Stop)
        {
            // delete and remove the frame from the hash
            QFrame* trackFrame = _popups.value(popup);
            if(trackFrame)
                trackFrame->deleteLater();
            _popups.remove(popup);
            delete popup;
        }
    }
    else
    {
        QBoxLayout* popupLayout = dynamic_cast<QBoxLayout*>(ui->scrollAreaWidgetContents->layout());
        if(!popupLayout)
            return;

        // create the frame for this track and set the play button to either the pause or play icon
        QFrame* popupFrame = new QFrame();
        QHBoxLayout* frameLayout = new QHBoxLayout();
        frameLayout->setContentsMargins(0, 0, 0, 0);
        frameLayout->setSpacing(0);
        popupFrame->setLayout(frameLayout);
        popupFrame->setFrameStyle(QFrame::Box);

        PopupAudio* newPopup = new PopupAudio(track, this);
        newPopup->prepareFrame(frameLayout, 0);

        popupLayout->addWidget(popupFrame);
        _popups.insert(newPopup, popupFrame);
    }

    updateFrameVisibility();
}

void PopupsPreviewFrame::updateFrameVisibility()
{    
    if(((_campaign) && (_campaign->getOverlayCount() > 0)) ||
       (_popups.size() > 0))
    {
        qDebug() << "[PopupsPreviewFrame] updating frame visibilty";

        ui->frameCollapse->setVisible(true);
        ui->scrollPopups->setVisible(true);

        ui->scrollPopups->widget()->adjustSize();
        ui->scrollAreaWidgetContents->layout()->setSizeConstraint(QLayout::SetMinimumSize);
        ui->scrollPopups->updateGeometry();

        int overlayMaxSize = 6;
        QBoxLayout* popupLayout = dynamic_cast<QBoxLayout*>(ui->scrollAreaWidgetContents->layout());
        if(popupLayout)
        {
            for(int i = 0; i < popupLayout->count(); ++i)
            {
                QLayoutItem* item = popupLayout->itemAt(i);
                if((item) && (item->widget()))
                {
                    int widgetSize = item->widget()->sizeHint().height();
                    qDebug() << "[PopupsPreviewFrame] widget: " << item->widget()->objectName() << " size: " << widgetSize;
                    overlayMaxSize += widgetSize + 3;
                }
            }
        }

        // Clamp to the chosen maximum
        int effectiveHeight = qMin(overlayMaxSize, 400);
        ui->scrollPopups->setMinimumHeight(effectiveHeight);
        ui->scrollPopups->setMaximumHeight(effectiveHeight);
        ui->scrollPopups->updateGeometry();
    }
    else
    {
        ui->frameCollapse->setVisible(false);
        ui->scrollPopups->setVisible(false);
    }
}
