#include "ribbontabhelp.h"
#include "ui_ribbontabhelp.h"

RibbonTabHelp::RibbonTabHelp(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::RibbonTabHelp)
{
    ui->setupUi(this);

    connect(ui->btnUsersGuide, SIGNAL(clicked(bool)), this, SIGNAL(userGuideClicked()));
    connect(ui->btnGettingStarted, SIGNAL(clicked(bool)), this, SIGNAL(gettingStartedClicked()));
    connect(ui->btnCheckForUpdates, SIGNAL(clicked(bool)), this, SIGNAL(checkForUpdatesClicked()));
    connect(ui->btnAbout, SIGNAL(clicked(bool)), this, SIGNAL(aboutClicked()));
}

RibbonTabHelp::~RibbonTabHelp()
{
    delete ui;
}

/*
 *
 *
        <widget class="QMenu" name="menu_Help">
         <property name="title">
          <string>&amp;Help</string>
         </property>
         <addaction name="actionAbout"/>
         <addaction name="actionCheck_For_Updates"/>
         <addaction name="separator"/>
         <addaction name="action_Users_Guide"/>
         <addaction name="action_Getting_Started"/>
        </widget>
        <addaction name="menu_File"/>
        <addaction name="menu_Edit"/>
        <addaction name="menu_Campaign"/>
        <addaction name="menu_Bestiary"/>
        <addaction name="menu_Help"/>
       </widget>


       */
