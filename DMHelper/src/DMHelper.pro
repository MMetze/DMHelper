#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T10:30:29
#
#-------------------------------------------------

QT       += core gui xml multimedia multimediawidgets opengl network openglwidgets uitools

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DMHelper
TEMPLATE = app

message("Building DMHelper")

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
    battlecombatantwidget.cpp \
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
    bestiarytemplatedialog.cpp \
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
    charactertemplateframe.cpp \
    characterv2.cpp \
    characterv2converter.cpp \
    colorpushbutton.cpp \
    combatantrolloverframe.cpp \
    combatantwidget.cpp \
    combatantwidgetcharacter.cpp \
    combatantwidgetinternals.cpp \
    combatantwidgetinternalscharacter.cpp \
    combatantwidgetinternalsmonster.cpp \
    combatantwidgetmonster.cpp \
    conditionseditdialog.cpp \
    configurelockedgriddialog.cpp \
    customtable.cpp \
    customtableentry.cpp \
    customtableframe.cpp \
    bestiaryfindtokendialog.cpp \
    discordposter.cpp \
    dmh_opengl.cpp \
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
    fearframe.cpp \
    globalsearch.cpp \
    globalsearchframe.cpp \
    gridconfig.cpp \
    gridsizer.cpp \
    helpdialog.cpp \
    initiativelistcombatantwidget.cpp \
    initiativelistdialog.cpp \
    layer.cpp \
    layerblank.cpp \
    layereffect.cpp \
    layereffectsettings.cpp \
    layerfow.cpp \
    layerfowsettings.cpp \
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
    monsterclassv2.cpp \
    monsterclassv2converter.cpp \
    monsterfactory.cpp \
    newcampaigndialog.cpp \
    newentrydialog.cpp \
    objectfactory.cpp \
    objectimportdialog.cpp \
    objectimportworker.cpp \
    optionsaccessor.cpp \
    overlay.cpp \
    overlaycounter.cpp \
    overlayfear.cpp \
    overlayframe.cpp \
    overlaymanager.cpp \
    overlayseditdialog.cpp \
    overlaytimer.cpp \
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
    rulefactory.cpp \
    ruleinitiative.cpp \
    ruleinitiative2e.cpp \
    ruleinitiative5e.cpp \
    ruleinitiativegroup.cpp \
    ruleinitiativegroupmonsters.cpp \
    ruleset.cpp \
    selectcombatantdialog.cpp \
    selectitemdialog.cpp \
    selectstringdialog.cpp \
    soundboardframe.cpp \
    soundboardgroup.cpp \
    soundboardgroupframe.cpp \
    soundboardtrack.cpp \
    soundboardtrackframe.cpp \
    spell.cpp \
    spellbook.cpp \
    spellbookdialog.cpp \
    spellslotlevelbutton.cpp \
    spellslotradiobutton.cpp \
    templatefactory.cpp \
    templateframe.cpp \
    templateobject.cpp \
    templateresourcelayout.cpp \
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
    dicerolldialogcombatants.cpp \
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
    battlecombatantwidget.h \
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
    bestiarytemplatedialog.h \
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
    charactertemplateframe.h \
    characterv2.h \
    characterv2converter.h \
    colorpushbutton.h \
    combatantrolloverframe.h \
    combatantwidget.h \
    combatantwidgetcharacter.h \
    combatantwidgetinternals.h \
    combatantwidgetinternalscharacter.h \
    combatantwidgetinternalsmonster.h \
    combatantwidgetmonster.h \
    conditionseditdialog.h \
    configurelockedgriddialog.h \
    customtable.h \
    customtableentry.h \
    customtableframe.h \
    bestiaryfindtokendialog.h \
    discordposter.h \
    dmh_opengl.h \
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
    fearframe.h \
    globalsearch.h \
    globalsearchframe.h \
    gridconfig.h \
    gridsizer.h \
    helpdialog.h \
    initiativelistcombatantwidget.h \
    initiativelistdialog.h \
    layer.h \
    layerblank.h \
    layereffect.h \
    layereffectsettings.h \
    layerfow.h \
    layerfowsettings.h \
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
    monsterclassv2.h \
    monsterclassv2converter.h \
    monsterfactory.h \
    newcampaigndialog.h \
    newentrydialog.h \
    objectfactory.h \
    objectimportdialog.h \
    objectimportworker.h \
    optionsaccessor.h \
    overlay.h \
    overlaycounter.h \
    overlayfear.h \
    overlayframe.h \
    overlaymanager.h \
    overlayseditdialog.h \
    overlaytimer.h \
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
    rulefactory.h \
    ruleinitiative.h \
    ruleinitiative2e.h \
    ruleinitiative5e.h \
    ruleinitiativegroup.h \
    ruleinitiativegroupmonsters.h \
    ruleset.h \
    selectcombatantdialog.h \
    selectitemdialog.h \
    selectstringdialog.h \
    soundboardframe.h \
    soundboardgroup.h \
    soundboardgroupframe.h \
    soundboardtrack.h \
    soundboardtrackframe.h \
    spell.h \
    spellbook.h \
    spellbookdialog.h \
    spellslotlevelbutton.h \
    spellslotradiobutton.h \
    templatefactory.h \
    templateframe.h \
    templateobject.h \
    templateresourcelayout.h \
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
    dicerolldialogcombatants.h \
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
    battlecombatantwidget.ui \
    battledialogeffectsettingsobjectvideo.ui \
    battleframe.ui \
    bestiaryexportdialog.ui \
    bestiarypopulatetokensdialog.ui \
    bestiarytemplatedialog.ui \
    campaignnotesdialog.ui \
    characterimportdialog.ui \
    characterimportheroforgedialog.ui \
    charactertemplateframe.ui \
    combatantrolloverframe.ui \
    combatantwidgetcharacter.ui \
    combatantwidgetmonster.ui \
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
    fearframe.ui \
    globalsearchframe.ui \
    helpdialog.ui \
    initiativelistcombatantwidget.ui \
    initiativelistdialog.ui \
    layereffectsettings.ui \
    layerfowsettings.ui \
    layerframe.ui \
    layerseditdialog.ui \
    layervideoeffectsettings.ui \
    legaldialog.ui \
    mapblankdialog.ui \
    mapcolorizedialog.ui \
    newcampaigndialog.ui \
    newentrydialog.ui \
    objectimportdialog.ui \
    overlayframe.ui \
    overlayseditdialog.ui \
    partycharactergridframe.ui \
    partyframe.ui \
    partyframecharacter.ui \
    presentupdatedialog.ui \
    publishbuttonframe.ui \
    publishbuttonribbon.ui \
    randommarketdialog.ui \
    resources/ui/action.ui \
    resources/ui/character-daggerheart.ui \
    resources/ui/character2e.ui \
    resources/ui/character5e-2024.ui \
    resources/ui/character5e.ui \
    resources/ui/monster-daggerheart.ui \
    resources/ui/monster2e.ui \
    resources/ui/monster5e-2024.ui \
    resources/ui/monster5e.ui \
    resources/ui/spellSlot.ui \
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
    dicerolldialogcombatants.ui \
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
    resources/character-daggerheart.xml \
    resources/character.xml \
    resources/character2e.xml \
    resources/character5e-2024.xml \
    resources/character5e.xml \
    resources/equipment.xml \
    resources/monster-daggerheart.xml \
    resources/monster2e.xml \
    resources/monster5e.xml \
    resources/quickref_data.xml \
    resources/ruleset.xml \
    resources/shops.xml \
    resources/tables/Indefinite Madness.xml \
    resources/tables/Indefinite Madness.xml \
    resources/tables/Long Term Madness.xml \
    resources/tables/Long Term Madness.xml \
    resources/tables/Short Term Madness.xml \
    resources/tables/Short Term Madness.xml \
    resources/tables/table.xsd \
    todos \
    todos_new_bugs

INCLUDEPATH += $$PWD/../../DMHelperShared/inc
DEPENDPATH += $$PWD/../../DMHelperShared/inc
DEPENDPATH += $$PWD/../../DMHelperShared/src

# Compiler settings
win32:CONFIG(debug, debug|release) {
    message("Windows Debug build detected")
    QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_DEBUG                      # Use debug flags
    QMAKE_CXXFLAGS += /FS                                       # enable multi-threaded file access
    QMAKE_LFLAGS += /DEBUG                                      # ensures debug info is linked in
    QMAKE_LFLAGS -= /INCREMENTAL                                # Optional, improves debugger stability
#    QMAKE_PRE_LINK += cmd /C del /Q /S \"$$OUT_PWD\\*.pdb\"     # delete old PDB files
}

win32:CONFIG(release, debug|release) {
    message("Windows Release build detected")
    QMAKE_CXXFLAGS += /O2   # use optimized flags
    QMAKE_CXXFLAGS -= /Zi   # ensure to turn off generating full debug info
    QMAKE_LFLAGS -= /DEBUG  # ensure to turn off linking debug info
    DEFINES += NDEBUG       # ensure to turn off assert
}

macx:CONFIG(debug, debug|release) {
    message("MacOS Debug build detected")
    QMAKE_CXXFLAGS += -g    # Use debug flags
}

macx:CONFIG(release, debug|release) {
    message("MacOS Release build detected")
    QMAKE_CXXFLAGS += -O2   # use optimized flags
    DEFINES += NDEBUG       # ensure to turn off assert
}

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

