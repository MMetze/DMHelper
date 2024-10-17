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
    connect(ui->btnClose, SIGNAL(clicked(bool)), this, SIGNAL(closeClicked()));

    connect(ui->btnUsersGuide, SIGNAL(clicked(bool)), this, SIGNAL(userGuideClicked()));
    connect(ui->btnGettingStarted, SIGNAL(clicked(bool)), this, SIGNAL(gettingStartedClicked()));
    connect(ui->btnPreferences, SIGNAL(clicked(bool)), this, SIGNAL(optionsClicked()));
    connect(ui->btnCheckForUpdates, SIGNAL(clicked(bool)), this, SIGNAL(checkForUpdatesClicked()));
    connect(ui->btnHelp, SIGNAL(clicked(bool)), this, SIGNAL(helpClicked()));
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
    setStandardButtonSize(*ui->lblClose, *ui->btnClose, frameHeight);

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
    setStandardButtonSize(*ui->lblPreferences, *ui->btnPreferences, frameHeight);
    setStandardButtonSize(*ui->lblCheckForUpdates, *ui->btnCheckForUpdates, frameHeight);
    setStandardButtonSize(*ui->lblHelp, *ui->btnHelp, frameHeight);
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
