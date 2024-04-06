#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T10:30:29
#
#-------------------------------------------------

QT       += core gui xml multimedia multimediawidgets opengl network openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DMHelper
TEMPLATE = app

#install_it.path = $$PWD/../bin
#win32:install_it.files = $$PWD/bin-win32/*
#win64:install_it.files = $$PWD/bin-win64/*
#
#INSTALLS += \
#    install_it

win32:RC_ICONS += dmhelper.ico
macx:ICON=data/macimg/DMHelper.icns

SOURCES += main.cpp\
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
    audiofactory.cpp \
    audiotrackfile.cpp \
    audiotracksyrinscape.cpp \
    audiotracksyrinscapeonline.cpp \
    audiotrackurl.cpp \
    audiotrackyoutube.cpp \
    basicdateserver.cpp \
    battlecombatantframe.cpp \
    battledialogeffectsettingsbase.cpp \
    battledialogeffectsettingsobjectvideo.cpp \
    battledialoggraphicsscenemousehandler.cpp \
    battledialogmodeleffectobject.cpp \
    battledialogmodeleffectobjectvideo.cpp \
    battledialogmodeleffectshape.cpp \
    battledialogmodelobject.cpp \
    battleframe.cpp \
    battleframemapdrawer.cpp \
    battleframestate.cpp \
    battleframestatemachine.cpp \
    bestiaryexportdialog.cpp \
    bestiarypopulatetokensdialog.cpp \
    camerarect.cpp \
    camerascene.cpp \
    campaignexporter.cpp \
    campaignnotesdialog.cpp \
    campaignobjectfactory.cpp \
    campaignobjectframe.cpp \
    campaignobjectframestack.cpp \
    campaigntreeactivestack.cpp \
    campaigntreeitem.cpp \
    characterimportdialog.cpp \
    characterimportheroforge.cpp \
    characterimportheroforgedata.cpp \
    characterimportheroforgedialog.cpp \
    colorpushbutton.cpp \
    combatantrolloverframe.cpp \
    combatantwidget.cpp \
    conditionseditdialog.cpp \
    configurelockedgriddialog.cpp \
    customtable.cpp \
    customtableentry.cpp \
    customtableframe.cpp \
    bestiaryfindtokendialog.cpp \
    discordposter.cpp \
    dmh_vlc.cpp \
    dmhcache.cpp \
    dmhelperribbon.cpp \
    dmhfilereader.cpp \
    dmhlogger.cpp \
    dmhwaitingdialog.cpp \
    emptycampaignframe.cpp \
    encountertextlinked.cpp \
    equipmentserver.cpp \
    expertisedialog.cpp \
    exportdialog.cpp \
    exportworker.cpp \
    gridconfig.cpp \
    layer.cpp \
    layerblank.cpp \
    layereffect.cpp \
    layereffectsettings.cpp \
    layerfow.cpp \
    layerframe.cpp \
    layergrid.cpp \
    layerimage.cpp \
    layerreference.cpp \
    layerscene.cpp \
    layerseditdialog.cpp \
    layertokens.cpp \
    layervideo.cpp \
    layervideoeffect.cpp \
    layervideoeffectsettings.cpp \
    legaldialog.cpp \
    mainwindow.cpp \
    mapblankdialog.cpp \
    mapcolorizedialog.cpp \
    mapcolorizefilter.cpp \
    mapfactory.cpp \
    mapframescene.cpp \
    objectfactory.cpp \
    objectimportdialog.cpp \
    objectimportworker.cpp \
    optionsaccessor.cpp \
    party.cpp \
    partycharactergridframe.cpp \
    partyframe.cpp \
    partyframecharacter.cpp \
    placeholder.cpp \
    presentupdatedialog.cpp \
    publishbuttonframe.cpp \
    publishbuttonproxy.cpp \
    publishbuttonribbon.cpp \
    publishglbattlebackground.cpp \
    publishglbattleeffect.cpp \
    publishglbattleeffectvideo.cpp \
    publishglbattlegrid.cpp \
    publishglbattleimagerenderer.cpp \
    publishglbattleobject.cpp \
    publishglbattlerenderer.cpp \
    publishglbattletoken.cpp \
    publishglbattlevideorenderer.cpp \
    publishglframe.cpp \
    publishglimage.cpp \
    publishglimagerenderer.cpp \
    publishglmaprenderer.cpp \
    publishglobject.cpp \
    publishglrect.cpp \
    publishglrenderer.cpp \
    publishglscene.cpp \
    publishgltextrenderer.cpp \
    publishgltokenhighlight.cpp \
    publishgltokenhighlighteffect.cpp \
    publishgltokenhighlightref.cpp \
    publishwindow.cpp \
    randommarketdialog.cpp \
    ribbonframe.cpp \
    ribbonframetext.cpp \
    ribbonmain.cpp \
    ribbontabaudio.cpp \
    ribbontabbattle.cpp \
    ribbontabbattlemap.cpp \
    ribbontabbattleview.cpp \
    ribbontabcampaign.cpp \
    ribbontabfile.cpp \
    ribbontabmap.cpp \
    ribbontabtext.cpp \
    ribbontabtools.cpp \
    ribbontabworldmap.cpp \
    selectcombatantdialog.cpp \
    selectitemdialog.cpp \
    selectstringdialog.cpp \
    soundboardframe.cpp \
    soundboardgroup.cpp \
    soundboardgroupframe.cpp \
    soundboardtrackframe.cpp \
    spell.cpp \
    spellbook.cpp \
    spellbookdialog.cpp \
    spellslotlevelbutton.cpp \
    spellslotradiobutton.cpp \
    textbrowsermargins.cpp \
    texteditmargins.cpp \
    tokeneditdialog.cpp \
    tokeneditor.cpp \
    undofowbase.cpp \
    undofowfill.cpp \
    undofowpath.cpp \
    dicerolldialog.cpp \
    map.cpp \
    character.cpp \
    campaign.cpp \
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
    monster.cpp \
    combatantfactory.cpp \
    bestiarydialog.cpp \
    monsterclass.cpp \
    bestiary.cpp \
    campaigntree.cpp \
    campaigntreemodel.cpp \
    textpublishdialog.cpp \
    complicationdata.cpp \
    combatantselectdialog.cpp \
    quickref.cpp \
    quickrefdatawidget.cpp \
    mapmarkerdialog.cpp \
    undofowpoint.cpp \
    undofowshape.cpp \
    undomarker.cpp \
    mapcontent.cpp \
    mapmarkergraphicsitem.cpp \
    scaledpixmap.cpp \
    characterframe.cpp \
    mapframe.cpp \
    optionsdialog.cpp \
    optionscontainer.cpp \
    selectzoom.cpp \
    itemselectdialog.cpp \
    grid.cpp \
    clockframe.cpp \
    timeanddateframe.cpp \
    basicdate.cpp \
    unselectedellipse.cpp \
    unselectedpixmap.cpp \
    publishframe.cpp \
    quickrefframe.cpp \
    dmscreentabwidget.cpp \
    dicerollframe.cpp \
    battledialogmodel.cpp \
    battledialoggraphicsview.cpp \
    battledialoggraphicsscene.cpp \
    battledialogeffectsettings.cpp \
    unselectedpolygon.cpp \
    unselectedrect.cpp \
    updatechecker.cpp \
    videoplayer.cpp \
    videoplayergl.cpp \
    videoplayerglplayer.cpp \
    videoplayerglscreenshot.cpp \
    videoplayerglvideo.cpp \
    videoplayerscreenshot.cpp \
    welcomeframe.cpp \
    whatsnewdialog.cpp \
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
    battledialoglogger.cpp \
    battledialogevent.cpp \
    battledialogeventnewround.cpp \
    battledialogeventdamage.cpp \
    characterimporter.cpp \
    battledialoglogview.cpp \
    monsteractioneditdialog.cpp \
    aboutdialog.cpp \
    combatantreference.cpp \
    texttranslatedialog.cpp

HEADERS  += mainwindow.h \
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
    audiofactory.h \
    audiotrackfile.h \
    audiotracksyrinscape.h \
    audiotracksyrinscapeonline.h \
    audiotrackurl.h \
    audiotrackyoutube.h \
    basicdateserver.h \
    battlecombatantframe.h \
    battledialogeffectsettingsbase.h \
    battledialogeffectsettingsobjectvideo.h \
    battledialoggraphicsscenemousehandler.h \
    battledialogmodeleffectobject.h \
    battledialogmodeleffectobjectvideo.h \
    battledialogmodeleffectshape.h \
    battledialogmodelobject.h \
    battleframe.h \
    battleframemapdrawer.h \
    battleframestate.h \
    battleframestatemachine.h \
    bestiaryexportdialog.h \
    bestiarypopulatetokensdialog.h \
    camerarect.h \
    camerascene.h \
    campaignexporter.h \
    campaignnotesdialog.h \
    campaignobjectfactory.h \
    campaignobjectframe.h \
    campaignobjectframestack.h \
    campaigntreeactivestack.h \
    campaigntreeitem.h \
    characterimportdialog.h \
    characterimportheroforge.h \
    characterimportheroforgedata.h \
    characterimportheroforgedialog.h \
    colorpushbutton.h \
    combatantrolloverframe.h \
    combatantwidget.h \
    conditionseditdialog.h \
    configurelockedgriddialog.h \
    customtable.h \
    customtableentry.h \
    customtableframe.h \
    bestiaryfindtokendialog.h \
    discordposter.h \
    dmh_vlc.h \
    dmhcache.h \
    dmhelperribbon.h \
    dmhfilereader.h \
    dmhlogger.h \
    dmhwaitingdialog.h \
    dmversion.h \
    emptycampaignframe.h \
    encountertextlinked.h \
    equipmentserver.h \
    expertisedialog.h \
    exportdialog.h \
    exportworker.h \
    gridconfig.h \
    layer.h \
    layerblank.h \
    layereffect.h \
    layereffectsettings.h \
    layerfow.h \
    layerframe.h \
    layergrid.h \
    layerimage.h \
    layerreference.h \
    layerscene.h \
    layerseditdialog.h \
    layertokens.h \
    layervideo.h \
    layervideoeffect.h \
    layervideoeffectsettings.h \
    legaldialog.h \
    mapblankdialog.h \
    mapcolorizedialog.h \
    mapcolorizefilter.h \
    mapfactory.h \
    mapframescene.h \
    mapmarker.h \
    objectfactory.h \
    objectimportdialog.h \
    objectimportworker.h \
    optionsaccessor.h \
    party.h \
    partycharactergridframe.h \
    partyframe.h \
    partyframecharacter.h \
    placeholder.h \
    presentupdatedialog.h \
    publishbuttonframe.h \
    publishbuttonproxy.h \
    publishbuttonribbon.h \
    publishglbattlebackground.h \
    publishglbattleeffect.h \
    publishglbattleeffectvideo.h \
    publishglbattlegrid.h \
    publishglbattleimagerenderer.h \
    publishglbattleobject.h \
    publishglbattlerenderer.h \
    publishglbattletoken.h \
    publishglbattlevideorenderer.h \
    publishglframe.h \
    publishglimage.h \
    publishglimagerenderer.h \
    publishglmaprenderer.h \
    publishglobject.h \
    publishglrect.h \
    publishglrenderer.h \
    publishglscene.h \
    publishgltextrenderer.h \
    publishgltokenhighlight.h \
    publishgltokenhighlighteffect.h \
    publishgltokenhighlightref.h \
    publishwindow.h \
    randommarketdialog.h \
    ribbonframe.h \
    ribbonframetext.h \
    ribbonmain.h \
    ribbontabaudio.h \
    ribbontabbattle.h \
    ribbontabbattlemap.h \
    ribbontabbattleview.h \
    ribbontabcampaign.h \
    ribbontabfile.h \
    ribbontabmap.h \
    ribbontabtext.h \
    ribbontabtools.h \
    ribbontabworldmap.h \
    selectcombatantdialog.h \
    selectitemdialog.h \
    selectstringdialog.h \
    soundboardframe.h \
    soundboardgroup.h \
    soundboardgroupframe.h \
    soundboardtrackframe.h \
    spell.h \
    spellbook.h \
    spellbookdialog.h \
    spellslotlevelbutton.h \
    spellslotradiobutton.h \
    textbrowsermargins.h \
    texteditmargins.h \
    tokeneditdialog.h \
    tokeneditor.h \
    undofowbase.h \
    undofowfill.h \
    undofowpath.h \
    dicerolldialog.h \
    map.h \
    character.h \
    campaign.h \
    dmconstants.h \
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
    monster.h \
    combatantfactory.h \
    bestiarydialog.h \
    monsterclass.h \
    bestiary.h \
    campaigntree.h \
    campaigntreemodel.h \
    textpublishdialog.h \
    complicationdata.h \
    combatantselectdialog.h \
    quickref.h \
    quickrefdatawidget.h \
    mapmarkerdialog.h \
    undofowpoint.h \
    undofowshape.h \
    undomarker.h \
    mapcontent.h \
    mapmarkergraphicsitem.h \
    scaledpixmap.h \
    characterframe.h \
    mapframe.h \
    optionsdialog.h \
    optionscontainer.h \
    selectzoom.h \
    itemselectdialog.h \
    grid.h \
    clockframe.h \
    timeanddateframe.h \
    basicdate.h \
    unselectedellipse.h \
    unselectedpixmap.h \
    publishframe.h \
    quickrefframe.h \
    dmscreentabwidget.h \
    dicerollframe.h \
    battledialogmodel.h \
    battledialoggraphicsview.h \
    battledialoggraphicsscene.h \
    battledialogeffectsettings.h \
    unselectedpolygon.h \
    unselectedrect.h \
    updatechecker.h \
    videoplayer.h \
    videoplayergl.h \
    videoplayerglplayer.h \
    videoplayerglscreenshot.h \
    videoplayerglvideo.h \
    videoplayerscreenshot.h \
    welcomeframe.h \
    whatsnewdialog.h \
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
    battledialoglogger.h \
    battledialogevent.h \
    battledialogeventnewround.h \
    battledialogeventdamage.h \
    characterimporter.h \
    battledialoglogview.h \
    monsteractioneditdialog.h \
    aboutdialog.h \
    combatantreference.h \
    texttranslatedialog.h

FORMS    += mainwindow.ui \
    battlecombatantframe.ui \
    battledialogeffectsettingsobjectvideo.ui \
    battleframe.ui \
    bestiaryexportdialog.ui \
    bestiarypopulatetokensdialog.ui \
    campaignnotesdialog.ui \
    characterimportdialog.ui \
    characterimportheroforgedialog.ui \
    combatantrolloverframe.ui \
    conditionseditdialog.ui \
    configurelockedgriddialog.ui \
    customtableframe.ui \
    bestiaryfindtokendialog.ui \
    dicerolldialog.ui \
    combatantdialog.ui \
    bestiarydialog.ui \
    dmhwaitingdialog.ui \
    emptycampaignframe.ui \
    expertisedialog.ui \
    layereffectsettings.ui \
    layerframe.ui \
    layerseditdialog.ui \
    layervideoeffectsettings.ui \
    legaldialog.ui \
    mapblankdialog.ui \
    mapcolorizedialog.ui \
    objectimportdialog.ui \
    partycharactergridframe.ui \
    partyframe.ui \
    partyframecharacter.ui \
    presentupdatedialog.ui \
    publishbuttonframe.ui \
    publishbuttonribbon.ui \
    randommarketdialog.ui \
    ribbonframetext.ui \
    ribbontabaudio.ui \
    ribbontabbattle.ui \
    ribbontabbattlemap.ui \
    ribbontabbattleview.ui \
    ribbontabcampaign.ui \
    ribbontabfile.ui \
    ribbontabmap.ui \
    ribbontabtext.ui \
    ribbontabtools.ui \
    ribbontabworldmap.ui \
    selectcombatantdialog.ui \
    selectitemdialog.ui \
    selectstringdialog.ui \
    soundboardframe.ui \
    soundboardgroupframe.ui \
    soundboardtrackframe.ui \
    spellbookdialog.ui \
    textpublishdialog.ui \
    combatantselectdialog.ui \
    quickrefdatawidget.ui \
    mapmarkerdialog.ui \
    characterframe.ui \
    mapframe.ui \
    optionsdialog.ui \
    selectzoom.ui \
    itemselectdialog.ui \
    clockframe.ui \
    timeanddateframe.ui \
    quickrefframe.ui \
    dmscreentabwidget.ui \
    dicerollframe.ui \
    battledialogeffectsettings.ui \
    tokeneditdialog.ui \
    welcomeframe.ui \
    whatsnewdialog.ui \
    widgetbattlecombatant.ui \
    dicerolldialogcombatants.ui \
    widgetcharacter.ui \
    widgetmonster.ui \
    encountertextedit.ui \
    mapselectdialog.ui \
    countdownframe.ui \
    countdownsubframe.ui \
    audioplaybackframe.ui \
    audiotrackedit.ui \
    monsteractionframe.ui \
    battledialoglogview.ui \
    monsteractioneditdialog.ui \
    aboutdialog.ui \
    exportdialog.ui \
    texttranslatedialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    requirements.txt \
    bugs.txt

DISTFILES += \
    buildanddeploy_msvc32.cmd \
    buildanddeploy_msvc64.cmd \
    buildanddeploymac \
    bugs.txt \
    buildanddeploymac.sh \
    installer/config/config_win32.xml \
    installer/config/config_win64.xml \
    installer/config/config_mac.xml \
    installer/config/dmhelper.ico \
    installer/config/dmhelper_background.png \
    installer/config/dmhelper_banner.png \
    installer/config/dmhelper_icon.png \
    installer/config/parchment.jpg \
    installer/config/style.qss \
    installer/packages/com.dmhelper.app/meta/installscript32.qs \
    installer/packages/com.dmhelper.app/meta/installscript64.qs \
    installer/packages/com.dmhelper.app/meta/license.txt \
    installer/packages/com.dmhelper.app/meta/package.xml \
    preparebuilddirectory_msvc32.cmd \
    preparebuilddirectory_msvc64.cmd \
    release_notes.txt \
    resources/calendar.xml \
    resources/equipment.xml \
    resources/quickref_data.xml \
    resources/shops.xml \
    resources/tables/Indefinite Madness.xml \
    resources/tables/Long Term Madness.xml \
    resources/tables/Short Term Madness.xml \
    todos

INCLUDEPATH += $$PWD/../../DMHelperShared/inc
DEPENDPATH += $$PWD/../../DMHelperShared/inc
DEPENDPATH += $$PWD/../../DMHelperShared/src

# link to libvlc
win32 {
    contains(QT_ARCH, i386) {
        message("32-bit VLC")
        INCLUDEPATH += $$PWD/vlc32
        LIBS += -L$$PWD/vlc32 -llibvlc
    } else {
        message("64-bit VLC")
        INCLUDEPATH += $$PWD/vlc64
        LIBS += -L$$PWD/vlc64 -llibvlc
    }
}
macx {
    message("MacOS 64-bit VLC")
    #INCLUDEPATH += $$PWD/vlc64/VLCKit.framework/Headers
    #LIBS += -F$$PWD/vlc64 -framework VLCKit

    INCLUDEPATH += $$PWD/vlcMac
    # link to the lib:
    LIBS += -L$$PWD/vlcMac -lvlc
    # make the app find the libs:
#    QMAKE_RPATHDIR = @executable_path/Frameworks
    # deploy the libs:
#    libvlc.files = $$OUT_PWD/libvlc/libvlc.12.dylib
#    libvlc.path = Frameworks
#    QMAKE_BUNDLE_DATA += libvlc


    MediaFiles.files += vlcMac/libvlc.dylib
    MediaFiles.files += vlcMac/libvlccore.dylib
    MediaFiles.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += MediaFiles
}

