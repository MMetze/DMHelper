#include "overlayspreviewframe.h"
#include "ui_overlayspreviewframe.h"
#include "campaign.h"
#include "overlay.h"

OverlaysPreviewFrame::OverlaysPreviewFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::OverlaysPreviewFrame),
    _campaign(nullptr)
{
    ui->setupUi(this);

    QVBoxLayout* overlaysLayout = new QVBoxLayout;
    overlaysLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    overlaysLayout->setContentsMargins(0, 0, 0, 0);
    overlaysLayout->setSpacing(3);
    ui->scrollAreaWidgetContents->setLayout(overlaysLayout);
    ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    ui->dividerLayout->setContentsMargins(0, 0, 0, 0);
    ui->dividerLayout->setAlignment(Qt::AlignCenter);
    ui->btnCollapse->setArrowType(Qt::DownArrow);
    connect(ui->btnCollapse, &QToolButton::clicked, this, &OverlaysPreviewFrame::toggleOverlayFrame);

    ui->frameCollapse->setVisible(false);
    ui->scrollOverlays->setVisible(false);
}

OverlaysPreviewFrame::~OverlaysPreviewFrame()
{
    clearOverlays();
    delete ui;
}

void OverlaysPreviewFrame::setCampaign(Campaign* campaign)
{
    if(_campaign == campaign)
        return;

    disconnect(_campaign, nullptr, this, nullptr);
    _campaign = campaign;

    ui->frameCollapse->setVisible((_campaign) && (_campaign->getOverlayCount() > 0));
    ui->scrollOverlays->setVisible((_campaign) && (_campaign->getOverlayCount() > 0));

    populateOverlays();

    connect(_campaign, &Campaign::overlaysChanged, this, &OverlaysPreviewFrame::populateOverlays);
}

void OverlaysPreviewFrame::toggleOverlayFrame()
{
    ui->scrollOverlays->setVisible(!ui->scrollOverlays->isVisible());
    ui->btnCollapse->setArrowType(ui->scrollOverlays->isVisible() ? Qt::DownArrow : Qt::UpArrow);
}

void OverlaysPreviewFrame::populateOverlays()
{
    clearOverlays();

    if(!_campaign)
        return;

    QBoxLayout* overlayLayout = dynamic_cast<QBoxLayout*>(ui->scrollAreaWidgetContents->layout());
    if(!overlayLayout)
        return;

    int overlayMaxSize = 0;
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
            overlayMaxSize += overlayFrame->sizeHint().height();
            overlayLayout->addWidget(overlayFrame);
        }
    }

    ui->scrollOverlays->widget()->adjustSize();
    ui->scrollAreaWidgetContents->layout()->setSizeConstraint(QLayout::SetMinimumSize);
    ui->scrollOverlays->updateGeometry();

    // Clamp to the chosen maximum
    int effectiveHeight = qMin(overlayMaxSize, 400);
    ui->scrollOverlays->setMinimumHeight(effectiveHeight);
    ui->scrollOverlays->setMaximumHeight(effectiveHeight);
    ui->scrollOverlays->updateGeometry();
}

void OverlaysPreviewFrame::clearOverlays()
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
}

