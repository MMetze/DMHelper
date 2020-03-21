#include "ribbontabfile.h"
#include "ui_ribbontabfile.h"
#include <QMenu>

RibbonTabFile::RibbonTabFile(QWidget *parent) :
    QFrame(parent),
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

    connect(ui->btnPreferences, SIGNAL(clicked(bool)), this, SIGNAL(optionsClicked()));
    connect(ui->btnClose, SIGNAL(clicked(bool)), this, SIGNAL(closeClicked()));
}

RibbonTabFile::~RibbonTabFile()
{
    delete ui;
}

QMenu* RibbonTabFile::getMRUMenu() const
{
    return _mruMenu;
}

void RibbonTabFile::showMRUMenu()
{
    _mruMenu->exec(mapToGlobal(ui->btnRecent->geometry().bottomLeft()));
}

void RibbonTabFile::hideMRUMenu()
{
    ui->btnRecent->setChecked(false);
}



/*
<widget class="QMenu" name="menu_File">
 <property name="title">
  <string>&amp;File</string>
 </property>
 <widget class="QMenu" name="menuRecent_Campaigns">
  <property name="title">
   <string>&amp;Recent Campaigns</string>
  </property>
 </widget>
 <addaction name="action_NewCampaign"/>
 <addaction name="action_OpenCampaign"/>
 <addaction name="menuRecent_Campaigns"/>
 <addaction name="action_SaveCampaign"/>
 <addaction name="actionSave_Campaign_As"/>
 <addaction name="actionClose_Campaign"/>
 <addaction name="separator"/>
 <addaction name="actionE_xit"/>
</widget>


        <widget class="QMenu" name="menu_Edit">
         <property name="title">
          <string>&amp;Edit</string>
         </property>
        </widget>

*/
