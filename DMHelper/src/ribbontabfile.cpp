#include "ribbontabfile.h"
#include "ui_ribbontabfile.h"
#include <QMenu>

RibbonTabFile::RibbonTabFile(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabFile),
    _mruMenu(new QMenu(this))
{
    ui->setupUi(this);

    connect(ui->btnRecent, SIGNAL(clicked(bool)), this, SLOT(showMRUMenu()));
    connect(_mruMenu, SIGNAL(aboutToHide()), this, SLOT(hideMRUMenu()));

    connect(ui->btnNew, SIGNAL(clicked(bool)), this, SIGNAL(newClicked()));
    connect(ui->btnOpen, SIGNAL(clicked(bool)), this, SIGNAL(openClicked()));
    connect(ui->btnSave, SIGNAL(clicked(bool)), this, SIGNAL(saveClicked()));
    connect(ui->btnSaveAs, SIGNAL(clicked(bool)), this, SIGNAL(saveAsClicked()));

    connect(ui->btnNetworkActive, SIGNAL(toggled(bool)), this, SLOT(setNetworkActiveImage(bool)));
    connect(ui->btnNetworkActive, SIGNAL(toggled(bool)), ui->btnSession, SLOT(setEnabled(bool)));
    connect(ui->btnNetworkActive, SIGNAL(toggled(bool)), this, SIGNAL(networkActiveClicked(bool)));
    connect(ui->btnSession, SIGNAL(clicked(bool)), this, SIGNAL(sessionClicked()));

    connect(ui->btnPreferences, SIGNAL(clicked(bool)), this, SIGNAL(optionsClicked()));
    connect(ui->btnClose, SIGNAL(clicked(bool)), this, SIGNAL(closeClicked()));

    connect(ui->btnUsersGuide, SIGNAL(clicked(bool)), this, SIGNAL(userGuideClicked()));
    connect(ui->btnGettingStarted, SIGNAL(clicked(bool)), this, SIGNAL(gettingStartedClicked()));
    connect(ui->btnCheckForUpdates, SIGNAL(clicked(bool)), this, SIGNAL(checkForUpdatesClicked()));
    connect(ui->btnAbout, SIGNAL(clicked(bool)), this, SIGNAL(aboutClicked()));
}

RibbonTabFile::~RibbonTabFile()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabFile::getPublishRibbon()
{
    return nullptr;
}

void RibbonTabFile::setNetworkActive(bool checked)
{
    if(checked != ui->btnNetworkActive->isChecked())
        ui->btnNetworkActive->setChecked(checked);
}

QMenu* RibbonTabFile::getMRUMenu() const
{
    return _mruMenu;
}

void RibbonTabFile::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblNew, *ui->btnNew, frameHeight);
    setStandardButtonSize(*ui->lblOpen, *ui->btnOpen, frameHeight);
    setStandardButtonSize(*ui->lblSave, *ui->btnSave, frameHeight);
    setStandardButtonSize(*ui->lblSaveAs, *ui->btnSaveAs, frameHeight);
    setLineHeight(*ui->line, frameHeight);
    setStandardButtonSize(*ui->lblPreferences, *ui->btnPreferences, frameHeight);
    setStandardButtonSize(*ui->lblClose, *ui->btnClose, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setStandardButtonSize(*ui->lblNetworkActive, *ui->btnNetworkActive, frameHeight);
    setStandardButtonSize(*ui->lblSession, *ui->btnSession, frameHeight);

    int openIconHeight = ui->btnOpen->iconSize().height();
    int mruHeight = height() - ui->lblOpen->height() - openIconHeight;
    ui->btnRecent->setMinimumHeight(mruHeight);
    ui->btnRecent->setMaximumHeight(mruHeight);
    ui->btnRecent->setMinimumWidth(ui->btnOpen->width());
    ui->btnRecent->setMaximumWidth(ui->btnOpen->width());
    ui->btnOpen->setMinimumHeight(openIconHeight);
    ui->btnOpen->setMaximumHeight(openIconHeight);

    setStandardButtonSize(*ui->lblUsersGuide, *ui->btnUsersGuide, frameHeight);
    setStandardButtonSize(*ui->lblGettingStarted, *ui->btnGettingStarted, frameHeight);
    setStandardButtonSize(*ui->lblCheckForUpdates, *ui->btnCheckForUpdates, frameHeight);
    setStandardButtonSize(*ui->lblAbout, *ui->btnAbout, frameHeight);
}

void RibbonTabFile::showMRUMenu()
{
    _mruMenu->exec(mapToGlobal(ui->btnRecent->geometry().bottomLeft()));
}

void RibbonTabFile::hideMRUMenu()
{
    ui->btnRecent->setChecked(false);
}

void RibbonTabFile::setNetworkActiveImage(bool active)
{
    ui->btnNetworkActive->setIcon(QIcon(active ? QString(":/img/data/icon-die-connected.png") : QString(":/img/data/icon-die-disconnected.png")));
}
