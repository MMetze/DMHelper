#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T10:30:29
#
#-------------------------------------------------

QT       += core gui xml multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DMHelper
TEMPLATE = app

install_it.path = $$PWD/../bin
install_it.files = $$PWD/binsrc/*

INSTALLS += \
    install_it

win32:RC_ICONS += dmhelper.ico

SOURCES += main.cpp\
        mainwindow.cpp \
    publishwindow.cpp \
    undopoint.cpp \
    undopath.cpp \
    undofill.cpp \
    undobase.cpp \
    dicerolldialog.cpp \
    adventure.cpp \
    map.cpp \
    encounter.cpp \
    character.cpp \
    characterwidget.cpp \
    characterdialog.cpp \
    campaign.cpp \
    adventureitem.cpp \
    mruhandler.cpp \
    mruaction.cpp \
    campaignobjectbase.cpp \
    encountertext.cpp \
    encounterbattle.cpp \
    encounterfactory.cpp \
    combatant.cpp \
    dice.cpp \
    attack.cpp \
    combatantdialog.cpp \
    battledialog.cpp \
    monster.cpp \
    monsterwidget.cpp \
    combatantfactory.cpp \
    bestiarydialog.cpp \
    monsterclass.cpp \
    bestiary.cpp \
    addmonstersdialog.cpp \
    combatantwidget.cpp \
    campaigntree.cpp \
    campaigntreemodel.cpp \
    dmscreendialog.cpp \
    textpublishdialog.cpp \
    chasedialog.cpp \
    chaserwidget.cpp \
    chaser.cpp \
    complicationdata.cpp \
    combatantselectdialog.cpp \
    chaserselectiondialog.cpp \
    quickref.cpp \
    quickrefdatawidget.cpp \
    chaseanimationdialog.cpp \
    mapmarkerdialog.cpp \
    undomarker.cpp \
    mapcontent.cpp \
    mapmarkergraphicsitem.cpp \
    scaledpixmap.cpp \
    characterframe.cpp \
    mapframe.cpp \
    addbattlewavedialog.cpp \
    optionsdialog.cpp \
    optionscontainer.cpp \
    selectzoom.cpp \
    itemselectdialog.cpp \
    grid.cpp \
    clockframe.cpp \
    timeanddateframe.cpp \
    basicdate.cpp \
    unselectedpixmap.cpp \
    scrolltabwidget.cpp \
    scrolloverlaywidget.cpp \
    publishframe.cpp \
    quickrefframe.cpp \
    dmscreentabwidget.cpp \
    scrolltabtransitionfactory.cpp \
    scrolltabtransitionbase.cpp \
    scrolltabtransitionrotate.cpp \
    scrolltabtransitionanimation.cpp \
    scrolltabtransitionsmoke.cpp \
    scrolltabtransitionzap.cpp \
    scrolltabtransitioninstant.cpp \
    encounterscrollingtext.cpp \
    encounterscrollingtextedit.cpp \
    scrollingtextwindow.cpp \
    dicerollframe.cpp \
    battledialogmodel.cpp \
    battledialogmanager.cpp \
    battledialoggraphicsview.cpp \
    battledialoggraphicsscene.cpp \
    battledialogeffectsettings.cpp \
    widgetbattlecombatant.cpp \
    dicerolldialogcombatants.cpp \
    widgetcharacter.cpp \
    widgetcombatantbase.cpp \
    widgetcharacterinternal.cpp \
    widgetmonster.cpp \
    widgetmonsterinternal.cpp \
    battledialogmodelcharacter.cpp \
    battledialogmodelcombatant.cpp \
    battledialogmodelmonsterbase.cpp \
    battledialogmodelmonsterclass.cpp \
    battledialogmodelmonstercombatant.cpp \
    battledialogmodeleffect.cpp \
    texteditformatterframe.cpp \
    encountertextedit.cpp \
    battledialogmodeleffectfactory.cpp \
    battledialogmodeleffectradius.cpp \
    battledialogmodeleffectcone.cpp \
    battledialogmodeleffectcube.cpp \
    battledialogmodeleffectline.cpp \
    mapselectdialog.cpp \
    countdownframe.cpp \
    countdownsubframe.cpp \
    audioplaybackframe.cpp \
    audiotrack.cpp \
    audiotrackedit.cpp \
    audioplayer.cpp \
    networkcontroller.cpp \
    monsteractionframe.cpp \
    monsteraction.cpp \
    encounterbattleedit.cpp \
    battledialoglogger.cpp \
    battledialogevent.cpp \
    battledialogeventnewround.cpp \
    battledialogeventdamage.cpp \
    characterimporter.cpp \
    battledialoglogview.cpp \
    monsteractioneditdialog.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    publishwindow.h \
    undopoint.h \
    undopath.h \
    undofill.h \
    undobase.h \
    dicerolldialog.h \
    adventure.h \
    map.h \
    encounter.h \
    character.h \
    characterwidget.h \
    characterdialog.h \
    campaign.h \
    dmconstants.h \
    adventureitem.h \
    mruhandler.h \
    mruaction.h \
    campaignobjectbase.h \
    encountertext.h \
    encounterbattle.h \
    encounterfactory.h \
    combatant.h \
    dice.h \
    attack.h \
    combatantdialog.h \
    battledialog.h \
    monster.h \
    monsterwidget.h \
    combatantfactory.h \
    bestiarydialog.h \
    monsterclass.h \
    bestiary.h \
    addmonstersdialog.h \
    combatantwidget.h \
    campaigntree.h \
    campaigntreemodel.h \
    dmscreendialog.h \
    textpublishdialog.h \
    chasedialog.h \
    chaserwidget.h \
    chaser.h \
    complicationdata.h \
    combatantselectdialog.h \
    chaserselectiondialog.h \
    quickref.h \
    quickrefdatawidget.h \
    chaseanimationdialog.h \
    mapmarkerdialog.h \
    undomarker.h \
    mapcontent.h \
    mapmarkergraphicsitem.h \
    scaledpixmap.h \
    characterframe.h \
    mapframe.h \
    addbattlewavedialog.h \
    optionsdialog.h \
    optionscontainer.h \
    selectzoom.h \
    itemselectdialog.h \
    grid.h \
    clockframe.h \
    timeanddateframe.h \
    basicdate.h \
    unselectedpixmap.h \
    scrolltabwidget.h \
    scrolloverlaywidget.h \
    publishframe.h \
    quickrefframe.h \
    dmscreentabwidget.h \
    scrolltabtransitionfactory.h \
    scrolltabtransitionbase.h \
    scrolltabtransitionrotate.h \
    scrolltabtransitionanimation.h \
    scrolltabtransitionsmoke.h \
    scrolltabtransitionzap.h \
    scrolltabtransitioninstant.h \
    encounterscrollingtext.h \
    encounterscrollingtextedit.h \
    scrollingtextwindow.h \
    dicerollframe.h \
    battledialogmodel.h \
    battledialogmanager.h \
    battledialoggraphicsview.h \
    battledialoggraphicsscene.h \
    battledialogeffectsettings.h \
    widgetbattlecombatant.h \
    dicerolldialogcombatants.h \
    widgetcharacter.h \
    widgetcombatantbase.h \
    widgetcharacterinternal.h \
    widgetmonster.h \
    widgetmonsterinternal.h \
    battledialogmodelcharacter.h \
    battledialogmodelcombatant.h \
    battledialogmodelmonsterbase.h \
    battledialogmodelmonsterclass.h \
    battledialogmodelmonstercombatant.h \
    battledialogmodeleffect.h \
    texteditformatterframe.h \
    encountertextedit.h \
    battledialogmodeleffectfactory.h \
    battledialogmodeleffectradius.h \
    battledialogmodeleffectcone.h \
    battledialogmodeleffectcube.h \
    battledialogmodeleffectline.h \
    mapselectdialog.h \
    countdownframe.h \
    countdownsubframe.h \
    audioplaybackframe.h \
    audiotrack.h \
    audiotrackedit.h \
    audioplayer.h \
    networkcontroller.h \
    monsteractionframe.h \
    monsteraction.h \
    encounterbattleedit.h \
    battledialoglogger.h \
    battledialogevent.h \
    battledialogeventnewround.h \
    battledialogeventdamage.h \
    characterimporter.h \
    battledialoglogview.h \
    monsteractioneditdialog.h \
    aboutdialog.h

FORMS    += mainwindow.ui \
    dicerolldialog.ui \
    characterdialog.ui \
    combatantdialog.ui \
    battledialog.ui \
    bestiarydialog.ui \
    addmonstersdialog.ui \
    dmscreendialog.ui \
    textpublishdialog.ui \
    chasedialog.ui \
    combatantselectdialog.ui \
    chaserselectiondialog.ui \
    quickrefdatawidget.ui \
    chaseanimationdialog.ui \
    mapmarkerdialog.ui \
    characterframe.ui \
    mapframe.ui \
    addbattlewavedialog.ui \
    optionsdialog.ui \
    selectzoom.ui \
    itemselectdialog.ui \
    clockframe.ui \
    timeanddateframe.ui \
    quickrefframe.ui \
    dmscreentabwidget.ui \
    encounterscrollingtextedit.ui \
    scrollingtextwindow.ui \
    dicerollframe.ui \
    battledialogeffectsettings.ui \
    widgetbattlecombatant.ui \
    dicerolldialogcombatants.ui \
    widgetcharacter.ui \
    widgetmonster.ui \
    texteditformatterframe.ui \
    encountertextedit.ui \
    mapselectdialog.ui \
    countdownframe.ui \
    countdownsubframe.ui \
    audioplaybackframe.ui \
    audiotrackedit.ui \
    monsteractionframe.ui \
    encounterbattleedit.ui \
    battledialoglogview.ui \
    monsteractioneditdialog.ui \
    aboutdialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    requirements.txt \
    bugs.txt

DISTFILES += \
    deploy.cmd \
    deploy.cmd \
    binsrc/quickref_data.xml \
    bugs.txt \
    buildanddeploy.cmd \
    release_notes.txt

CONFIG( debug, debug|release ) {
    # debug
    QMAKE_LIBDIR += $$PWD/../../DMHelperShared/build-debug/debug/
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../DMHelperShared/build-debug/debug/DMHelperShared.lib
    else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../DMHelperShared/build-debug/debug/libDMHelperShared.a
} else {
    # release
    QMAKE_LIBDIR += $$PWD/../../DMHelperShared/build-release/release/
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../DMHelperShared/build-release/release/DMHelperShared.lib
    else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../DMHelperShared/build-release/release/libDMHelperShared.a
}
unix|win32: LIBS += -lDMHelperShared

INCLUDEPATH += $$PWD/../../DMHelperShared/inc
DEPENDPATH += $$PWD/../../DMHelperShared/inc

