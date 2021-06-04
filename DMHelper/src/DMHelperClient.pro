#-------------------------------------------------
#
# Project created by QtCreator 2018-09-01T23:28:10
#
#-------------------------------------------------

QT       += core gui widgets network multimedia xml # webenginewidgets

TARGET = DMHelperClient
TEMPLATE = app

install_it.path = $$PWD/../bin
install_it.files = $$PWD/binsrc/*

INSTALLS += \
    install_it

win32:RC_ICONS += dmhelper.ico
macx:ICON=data/macimg/DMHelper.icns

CONFIG += c++11

SOURCES += \
        ../../DMHelperShared/src/dmhmessage.cpp \
        attack.cpp \
        audiofactory.cpp \
        audioplayer.cpp \
        audiotrack.cpp \
        audiotrackfile.cpp \
        audiotrackfiledownload.cpp \
        audiotracksyrinscape.cpp \
        audiotrackurl.cpp \
        audiotrackyoutube.cpp \
        basicdate.cpp \
        basicdateserver.cpp \
        battledialogeffectsettings.cpp \
        battledialogevent.cpp \
        battledialogeventdamage.cpp \
        battledialogeventnewround.cpp \
        battledialoggraphicsscene.cpp \
        battledialoggraphicsscenemousehandler.cpp \
        battledialoglogger.cpp \
        battledialogmodel.cpp \
        battledialogmodelcharacter.cpp \
        battledialogmodelcombatant.cpp \
        battledialogmodelcombatantdownload.cpp \
        battledialogmodeleffect.cpp \
        battledialogmodeleffectcone.cpp \
        battledialogmodeleffectcube.cpp \
        battledialogmodeleffectfactory.cpp \
        battledialogmodeleffectline.cpp \
        battledialogmodeleffectobject.cpp \
        battledialogmodeleffectobjectdownload.cpp \
        battledialogmodeleffectradius.cpp \
        battledialogmodeleffectshape.cpp \
        battledialogmodelmonsterbase.cpp \
        battledialogmodelmonsterclass.cpp \
        battledialogmodelmonstercombatant.cpp \
        battlerenderer.cpp \
        bestiary.cpp \
        campaign.cpp \
        campaignobjectbase.cpp \
        campaignobjectfactory.cpp \
        campaignobjectrenderer.cpp \
        character.cpp \
        combatant.cpp \
        combatantfactory.cpp \
        combatantreference.cpp \
        connectionuserdialog.cpp \
        dice.cpp \
        dmc_connectioninviteseditdialog.cpp \
        dmc_connectionsettingsdialog.cpp \
        dmc_connectionsettingsframe.cpp \
        dmc_main.cpp \
        ../../DMHelperShared/src/dmhlogon.cpp \
        ../../DMHelperShared/src/dmhlogon_private.cpp \
        ../../DMHelperShared/src/dmhnetworkdata.cpp \
        ../../DMHelperShared/src/dmhnetworkdata_private.cpp \
        ../../DMHelperShared/src/dmhnetworkdatafactory.cpp \
        ../../DMHelperShared/src/dmhnetworkmanager.cpp \
        ../../DMHelperShared/src/dmhnetworkmanager_private.cpp \
        ../../DMHelperShared/src/dmhnetworkobserver.cpp \
        ../../DMHelperShared/src/dmhnetworkobserver_private.cpp \
        ../../DMHelperShared/src/dmhobjectbase.cpp \
        ../../DMHelperShared/src/dmhobjectbase_private.cpp \
        ../../DMHelperShared/src/dmhpayload.cpp \
        ../../DMHelperShared/src/dmhpayload_private.cpp \
        ../../DMHelperShared/src/dmhshared.cpp \
        dmc_mainwindow.cpp \
        dmc_optionscontainer.cpp \
        dmc_serverconnection.cpp \
        dmc_settingsdialog.cpp \
        dmhlogger.cpp \
        encounterbattle.cpp \
        encounterbattledownload.cpp \
        encounterfactory.cpp \
        encounterscrollingtext.cpp \
        encountertext.cpp \
        encountertextdownload.cpp \
        encountertextrenderer.cpp \
        grid.cpp \
        monster.cpp \
        monsteraction.cpp \
        monsterclass.cpp \
        objectfactory.cpp \
        party.cpp \
        placeholder.cpp \
        remoteaudioplayer.cpp \
        remoterenderer.cpp \
        scaledpixmap.cpp \
        soundboardgroup.cpp \
        spell.cpp \
        spellbook.cpp \
        unselectedellipse.cpp \
        unselectedpixmap.cpp \
        unselectedpolygon.cpp \
        unselectedrect.cpp \
        videoplayer.cpp

HEADERS += \
    ../../DMHelperShared/inc/dmhmessage.h \
    attack.h \
    audiofactory.h \
    audioplayer.h \
    audiotrack.h \
    audiotrackfile.h \
    audiotrackfiledownload.h \
    audiotracksyrinscape.h \
    audiotrackurl.h \
    audiotrackyoutube.h \
    basicdate.h \
    basicdateserver.h \
    battledialogeffectsettings.h \
    battledialogevent.h \
    battledialogeventdamage.h \
    battledialogeventnewround.h \
    battledialoggraphicsscene.h \
    battledialoggraphicsscenemousehandler.h \
    battledialoglogger.h \
    battledialogmodel.h \
    battledialogmodelcharacter.h \
    battledialogmodelcombatant.h \
    battledialogmodelcombatantdownload.h \
    battledialogmodeleffect.h \
    battledialogmodeleffectcone.h \
    battledialogmodeleffectcube.h \
    battledialogmodeleffectfactory.h \
    battledialogmodeleffectline.h \
    battledialogmodeleffectobject.h \
    battledialogmodeleffectobjectdownload.h \
    battledialogmodeleffectradius.h \
    battledialogmodeleffectshape.h \
    battledialogmodelmonsterbase.h \
    battledialogmodelmonsterclass.h \
    battledialogmodelmonstercombatant.h \
    battlerenderer.h \
    bestiary.h \
    campaign.h \
    campaignobjectbase.h \
    campaignobjectfactory.h \
    campaignobjectrenderer.h \
    character.h \
    combatant.h \
    combatantfactory.h \
    combatantreference.h \
    connectionuserdialog.h \
    dice.h \
    dmc_connectioninviteseditdialog.h \
    dmc_connectionsettingsdialog.h \
    dmc_connectionsettingsframe.h \
    dmc_constants.h \
    dmc_mainwindow.h\
    ../../DMHelperShared/inc/dmhglobal.h \
    ../../DMHelperShared/inc/dmhlogon.h \
    ../../DMHelperShared/inc/dmhnetworkdata.h \
    ../../DMHelperShared/inc/dmhnetworkmanager.h \
    ../../DMHelperShared/inc/dmhnetworkobserver.h \
    ../../DMHelperShared/inc/dmhobjectbase.h \
    ../../DMHelperShared/inc/dmhpayload.h \
    ../../DMHelperShared/src/dmhlogon_private.h \
    ../../DMHelperShared/src/dmhnetworkdata_private.h \
    ../../DMHelperShared/src/dmhnetworkdatafactory.h \
    ../../DMHelperShared/src/dmhnetworkmanager_private.h \
    ../../DMHelperShared/src/dmhnetworkobserver_private.h \
    ../../DMHelperShared/src/dmhobjectbase_private.h \
    ../../DMHelperShared/src/dmhpayload_private.h \
    ../../DMHelperShared/src/dmhshared.h \
    dmc_optionscontainer.h \
    dmc_serverconnection.h \
    dmc_settingsdialog.h \
    dmconstants.h \
    dmhlogger.h \
    dmversion.h \
    encounterbattle.h \
    encounterbattledownload.h \
    encounterfactory.h \
    encounterscrollingtext.h \
    encountertext.h \
    encountertextdownload.h \
    encountertextrenderer.h \
    grid.h \
    monster.h \
    monsteraction.h \
    monsterclass.h \
    objectfactory.h \
    party.h \
    placeholder.h \
    remoteaudioplayer.h \
    remoterenderer.h \
    scaledpixmap.h \
    soundboardgroup.h \
    spell.h \
    spellbook.h \
    unselectedellipse.h \
    unselectedpixmap.h \
    unselectedpolygon.h \
    unselectedrect.h \
    videoplayer.h


FORMS += \
    battledialogeffectsettings.ui \
    connectionuserdialog.ui \
    dmc_connectioninviteseditdialog.ui \
    dmc_connectionsettingsdialog.ui \
    dmc_connectionsettingsframe.ui \
    dmc_mainwindow.ui \
    dmc_settingsdialog.ui

RESOURCES += \
    dmc_resources.qrc

INCLUDEPATH += $$PWD/../../DMHelperShared/inc
DEPENDPATH += $$PWD/../../DMHelperShared/inc
DEPENDPATH += $$PWD/../../DMHelperShared/src

win32: LIBS += -L$$PWD/vlc -llibvlc
macx: LIBS += -F$$PWD/vlc/ -framework VLCKit

DISTFILES += \
    buildanddeploy_client_msvc.cmd \
    requirements.txt
