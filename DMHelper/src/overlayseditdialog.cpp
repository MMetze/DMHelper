#include "overlayseditdialog.h"
#include "ui_overlayseditdialog.h"
#include "campaign.h"
#include "overlayframe.h"
#include "overlaytimer.h"
#include "overlayfear.h"
#include "overlaycounter.h"
#include <QInputDialog>

OverlaysEditDialog::OverlaysEditDialog(Campaign& campaign, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OverlaysEditDialog),
    _overlayLayout(nullptr),
    _campaign(campaign),
    _selectedFrame(nullptr)
{
    ui->setupUi(this);

    _overlayLayout = new QVBoxLayout;
    _overlayLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->scrollAreaWidgetContents->setLayout(_overlayLayout);

    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->btnNew, &QPushButton::clicked, this, &OverlaysEditDialog::handleNewOverlay);
    connect(ui->btnDelete, &QPushButton::clicked, this, &OverlaysEditDialog::handleDeleteOverlay);
    connect(ui->btnUp, &QPushButton::clicked, this, &OverlaysEditDialog::handleMoveOverlayUp);
    connect(ui->btnDown, &QPushButton::clicked, this, &OverlaysEditDialog::handleMoveOverlayDown);

    readOverlays();
    if(_overlayLayout->count() > 0)
        selectFrame(static_cast<OverlayFrame*>(_overlayLayout->itemAt(0)->widget()));
}

OverlaysEditDialog::~OverlaysEditDialog()
{
    delete ui;
}

bool OverlaysEditDialog::eventFilter(QObject *obj, QEvent *event)
{
    if((event->type() == QEvent::MouseButtonRelease) && (_overlayLayout))
        selectFrame(dynamic_cast<OverlayFrame*>(obj));

    return QDialog::eventFilter(obj, event);
}

void OverlaysEditDialog::selectFrame(OverlayFrame* frame)
{
    if((!_overlayLayout) || (_selectedFrame == frame))
        return;

    if(_selectedFrame)
        _selectedFrame->setSelected(false);

    _selectedFrame = frame;
    if(_selectedFrame)
        _selectedFrame->setSelected(true);
}

void OverlaysEditDialog::handleNewOverlay()
{
    QStringList items;
    items << tr("Counter") << tr("Timer") << tr("Fear");

    bool ok;
    QString selectedItem = QInputDialog::getItem(this, tr("New Overlay"), tr("Select New Overlay Type:"), items, 0, false, &ok);
    if((!ok) || (selectedItem.isEmpty()))
        return;

    Overlay* newOverlay = nullptr;
    if(selectedItem == tr("Counter"))
        newOverlay = new OverlayCounter(10);
    else if(selectedItem == tr("Timer"))
        newOverlay = new OverlayTimer(600);
    else if(selectedItem == tr("Fear"))
        newOverlay = new OverlayFear();

    _campaign.addOverlay(newOverlay);

    OverlayFrame* newFrame = addOverlayFrame(newOverlay);
    if(newFrame)
        selectFrame(newFrame);
}

void OverlaysEditDialog::handleDeleteOverlay()
{
    if(!_selectedFrame)
        return;

    int currentIndex = _campaign.getOverlayIndex(_selectedFrame->getOverlay());
    if((currentIndex == -1) || (currentIndex >= _campaign.getOverlayCount()))
        return;

    _campaign.removeOverlay(_selectedFrame->getOverlay());
    QLayoutItem* child = _overlayLayout->takeAt(currentIndex);
    if(child)
    {
        if(child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

void OverlaysEditDialog::handleMoveOverlayUp()
{
    if(!_selectedFrame)
        return;

    int currentIndex = _campaign.getOverlayIndex(_selectedFrame->getOverlay());
    if((currentIndex == -1) || (currentIndex == 0))
        return;

    if(_campaign.moveOverlay(currentIndex, currentIndex - 1))
    {
        resetLayout();
        if((_overlayLayout) && (_overlayLayout->itemAt(currentIndex - 1)))
            selectFrame(dynamic_cast<OverlayFrame*>(_overlayLayout->itemAt(currentIndex - 1)->widget()));
    }
}

void OverlaysEditDialog::handleMoveOverlayDown()
{
    if(!_selectedFrame)
        return;

    int currentIndex = _campaign.getOverlayIndex(_selectedFrame->getOverlay());
    if((currentIndex == -1) || (currentIndex >= _campaign.getOverlayCount()))
        return;

    if(_campaign.moveOverlay(currentIndex, currentIndex + 1))
    {
        resetLayout();
        if((_overlayLayout) && (_overlayLayout->itemAt(currentIndex + 1)))
            selectFrame(dynamic_cast<OverlayFrame*>(_overlayLayout->itemAt(currentIndex + 1)->widget()));
    }
}

void OverlaysEditDialog::resetLayout()
{
    _selectedFrame = nullptr;
    clearLayout();
    readOverlays();
}

void OverlaysEditDialog::readOverlays()
{
    if(!_overlayLayout)
        return;

    for(Overlay* overlay : _campaign.getOverlays())
    {
        addOverlayFrame(overlay);
    }
}

OverlayFrame* OverlaysEditDialog::addOverlayFrame(Overlay* overlay)
{
    if(!overlay)
        return nullptr;

    OverlayFrame* frame = new OverlayFrame(overlay, this);
    frame->installEventFilter(this);
    _overlayLayout->addWidget(frame);

    return frame;
}

void OverlaysEditDialog::clearLayout()
{
    if(!_overlayLayout)
        return;

    QLayoutItem* child;
    while((child = _overlayLayout->takeAt(0)) != nullptr)
    {
        if(child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}
