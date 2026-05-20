cmake_minimum_required(VERSION 3.16)

project(DMHelper LANGUAGES CXX)

# -------------------------------------------------
# C++ standard
# -------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------
# Qt 6
# -------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Xml
    Multimedia
    MultimediaWidgets
    OpenGL
    OpenGLWidgets
    Network
    UiTools
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# -------------------------------------------------
# Sources (explicit, no globbing!)
# -------------------------------------------------
set(DMHELPER_SOURCES
    # --- Shared library sources ---
    ../../DMHelperShared/src/dmhlogon.cpp
    ../../DMHelperShared/src/dmhlogon_private.cpp
    ../../DMHelperShared/src/dmhnetworkdata.cpp
    ../../DMHelperShared/src/dmhnetworkdata_private.cpp
    ../../DMHelperShared/src/dmhnetworkdatafactory.cpp
    ../../DMHelperShared/src/dmhnetworkmanager.cpp
    ../../DMHelperShared/src/dmhnetworkmanager_private.cpp
    ../../DMHelperShared/src/dmhnetworkobserver.cpp
    ../../DMHelperShared/src/dmhnetworkobserver_private.cpp
    ../../DMHelperShared/src/dmhobjectbase.cpp
    ../../DMHelperShared/src/dmhobjectbase_private.cpp
    ../../DMHelperShared/src/dmhpayload.cpp
    ../../DMHelperShared/src/dmhpayload_private.cpp
    ../../DMHelperShared/src/dmhshared.cpp

    # --- Application sources ---
    aboutdialog.cpp
    attack.cpp
    audiofactory.cpp
    audioplaybackframe.cpp
    audiotrack.cpp
    audiotrackedit.cpp
    audiotrackfile.cpp
    audiotracksyrinscape.cpp
    audiotracksyrinscapeonline.cpp
    audiotrackurl.cpp
    audiotrackyoutube.cpp
    basicdate.cpp
    basicdateserver.cpp
    battlecombatantwidget.cpp
    battledialogeffectsettings.cpp
    battledialogeffectsettingsbase.cpp
    battledialogeffectsettingsfire.cpp
    battledialogeffectsettingslight.cpp
    battledialogeffectsettingsobjectvideo.cpp
    battledialogeffectsettingssmoke.cpp
    battledialogeffectsettingssparks.cpp
    battledialogevent.cpp
    battledialogeventdamage.cpp
    battledialogeventnewround.cpp
    battledialogeventresetresources.cpp
    battledialoggraphicsscene.cpp
    battledialoggraphicsscenemousehandler.cpp
    battledialoggraphicsview.cpp
    battledialoglogger.cpp
    battledialoglogview.cpp
    battledialogmodel.cpp
    battledialogmodelcharacter.cpp
    battledialogmodelcombatant.cpp
    battledialogmodelcombatantgroup.cpp
    battledialogmodelinitiativeevent.cpp
    battledialogmodeleffect.cpp
    battledialogmodeleffectcone.cpp
    battledialogmodeleffectcube.cpp
    battledialogmodeleffectfactory.cpp
    battledialogmodeleffectline.cpp
    battledialogmodeleffectobject.cpp
    battledialogmodeleffectobjectvideo.cpp
    battledialogmodeleffectfire.cpp
    battledialogmodeleffectlight.cpp
    battledialogmodeleffectsmoke.cpp
    battledialogmodeleffectsparks.cpp
    battledialogmodeleffectradius.cpp
    battledialogmodeleffectshape.cpp
    battledialogmodelmonsterbase.cpp
    battledialogmodelmonsterclass.cpp
    battledialogmodelmonstercombatant.cpp
    battledialogmodelobject.cpp
    battleframe.cpp
    battleframemapdrawer.cpp
    battleframestate.cpp
    battleframestatemachine.cpp
    battletokenhealthbar.cpp
    bestiary.cpp
    bestiaryexportdialog.cpp
    bestiaryfindtokendialog.cpp
    bestiarypopulatetokensdialog.cpp
    bestiarytemplatedialog.cpp
    camerarect.cpp
    camerascene.cpp
    campaign.cpp
    campaignexporter.cpp
    campaignnotesdialog.cpp
    campaignobjectbase.cpp
    campaignobjectfactory.cpp
    campaignobjectframe.cpp
    campaignobjectframestack.cpp
    campaigntree.cpp
    campaigntreeactivestack.cpp
    campaigntreeitem.cpp
    campaigntreemodel.cpp
    character.cpp
    characterimportdialog.cpp
    characterimporter.cpp
    characterimportheroforge.cpp
    characterimportheroforgedata.cpp
    characterimportheroforgedialog.cpp
    charactertemplateframe.cpp
    characterv2.cpp
    characterv2converter.cpp
    clockframe.cpp
    colorpushbutton.cpp
    combatant.cpp
    combatantdialog.cpp
    combatantfactory.cpp
    combatantreference.cpp
    combatantrolloverframe.cpp
    combatantselectdialog.cpp
    combatantgroupwidget.cpp
    combatanttemplateadapter.cpp
    combatanttemplateframe.cpp
    combatantwidget.cpp
    combatantwidgetbase.cpp
    complicationdata.cpp
    conditions.cpp
    conditionseditdialog.cpp
    configurelockedgriddialog.cpp
    countdownframe.cpp
    countdownsubframe.cpp
    customtable.cpp
    customtableentry.cpp
    customtableframe.cpp
    dice.cpp
    dicerolldialog.cpp
    dicerolldialogcombatants.cpp
    dicerollframe.cpp
    discordposter.cpp
    dmh_opengl.cpp
    dmh_vlc.cpp
    dmhcache.cpp
    dmhelperribbon.cpp
    dmhfilereader.cpp
    dmhlogger.cpp
    dmhwaitingdialog.cpp
    dmscreentabwidget.cpp
    emptycampaignframe.cpp
    encounterbattle.cpp
    encounterfactory.cpp
    encountertext.cpp
    encountertextedit.cpp
    encountertextlinked.cpp
    equipmentserver.cpp
    expertisedialog.cpp
    exportdialog.cpp
    exportworker.cpp
    fearframe.cpp
    fowgraphicsitem.cpp
    globalsearch.cpp
    globalsearchframe.cpp
    grid.cpp
    gridconfig.cpp
    gridsizer.cpp
    helpdialog.cpp
    initiativelistcombatantwidget.cpp
    initiativelistdialog.cpp
    initiativeeventwidget.cpp
    itemselectdialog.cpp
    layer.cpp
    layerblank.cpp
    layerdraw.cpp
    layerdrawengine.cpp
    layerdrawobject.cpp
    layerdrawshape.cpp
    layerdrawstate.cpp
    layerdrawtooldialog.cpp
    layereffect.cpp
    layereffectsettings.cpp
    layerfow.cpp
    layerfowsettings.cpp
    layerframe.cpp
    layergrid.cpp
    layerimage.cpp
    layerparticle.cpp
    layerparticlesettings.cpp
    layerreference.cpp
    layerscene.cpp
    layerseditdialog.cpp
    layertokens.cpp
    layertokensdarkeneffect.cpp
    layervideo.cpp
    layervideoeffect.cpp
    layervideoeffectsettings.cpp
    legaldialog.cpp
    main.cpp
    mainwindow.cpp
    map.cpp
    mapblankdialog.cpp
    mapcolorizedialog.cpp
    mapcolorizefilter.cpp
    mapcontent.cpp
    mapfactory.cpp
    mapframe.cpp
    mapframescene.cpp
    mapmanagerdialog.cpp
    mapmarkerdialog.cpp
    mapmarkergraphicsitem.cpp
    mappartyiconitem.cpp
    mapselectdialog.cpp
    monster.cpp
    monsteraction.cpp
    monsteractioneditdialog.cpp
    monsteractionframe.cpp
    monsterclass.cpp
    monsterclassv2.cpp
    monsterclassv2converter.cpp
    monsterfactory.cpp
    mruaction.cpp
    mruhandler.cpp
    networkcontroller.cpp
    newcampaigndialog.cpp
    newentrydialog.cpp
    objectfactory.cpp
    objectimportdialog.cpp
    objectimportworker.cpp
    optionsaccessor.cpp
    optionscontainer.cpp
    optionsdialog.cpp
    overlay.cpp
    overlaycounter.cpp
    overlayfear.cpp
    overlayframe.cpp
    overlayrenderer.cpp
    overlayseditdialog.cpp
    overlaytimer.cpp
    party.cpp
    partycharactergridframe.cpp
    partyframe.cpp
    partyframecharacter.cpp
    perroundresource.cpp
    placeholder.cpp
    popup.cpp
    popupaudio.cpp
    popupspreviewframe.cpp
    presentupdatedialog.cpp
    publishbuttonframe.cpp
    publishbuttonproxy.cpp
    publishbuttonribbon.cpp
    publishframe.cpp
    publishglbattlebackground.cpp
    publishglbattleeffect.cpp
    publishglbattleeffectvideo.cpp
    publishglbattleeffectanimated.cpp
    publishglbattleeffectsmoke.cpp
    publishglbattleeffectfire.cpp
    publishglbattleeffectsparks.cpp
    publishglbattleeffectlight.cpp
    publishglbattlegrid.cpp
    publishglbattleimagerenderer.cpp
    publishglbattleobject.cpp
    publishglbattlerenderer.cpp
    publishglbattletoken.cpp
    publishglbattlevideorenderer.cpp
    publishglframe.cpp
    publishglimage.cpp
    publishglimagerenderer.cpp
    publishglmaprenderer.cpp
    publishglobject.cpp
    publishglrect.cpp
    publishglrenderer.cpp
    publishglscene.cpp
    publishgltextrenderer.cpp
    publishgltokenhighlight.cpp
    publishgltokenhighlighteffect.cpp
    publishgltokenhighlighthealthbar.cpp
    publishgltokenhighlightref.cpp
    publishwindow.cpp
    quickref.cpp
    quickrefdatawidget.cpp
    quickrefframe.cpp
    randommarketdialog.cpp
    ribbonframe.cpp
    ribbonframetext.cpp
    ribbonmain.cpp
    ribbontabaudio.cpp
    ribbontabbattle.cpp
    ribbontabbattlemap.cpp
    ribbontabbattleview.cpp
    ribbontabcampaign.cpp
    ribbontabfile.cpp
    ribbontabmap.cpp
    ribbontabtext.cpp
    ribbontabtools.cpp
    ribbontabworldmap.cpp
    rulefactory.cpp
    rulehealth.cpp
    rulehealth5e.cpp
    rulehealthdaggerheart.cpp
    ruleinitiative.cpp
    ruleinitiative2e.cpp
    ruleinitiative5e.cpp
    ruleinitiativegroup.cpp
    ruleinitiativegroupmonsters.cpp
    ruleinitiativenone.cpp
    ruleset.cpp
    scaledpixmap.cpp
    selectcombatantdialog.cpp
    selectitemdialog.cpp
    selectstringdialog.cpp
    selectzoom.cpp
    smoketestrunner.cpp
    soundboardframe.cpp
    soundboardgroup.cpp
    soundboardgroupframe.cpp
    soundboardtrack.cpp
    soundboardtrackframe.cpp
    spell.cpp
    spellbook.cpp
    spellbooktemplatedialog.cpp
    spellbookfactory.cpp
    spellslotlevelbutton.cpp
    spellslotradiobutton.cpp
    spellv2.cpp
    spellv2converter.cpp
    templatefactory.cpp
    templateframe.cpp
    templateobject.cpp
    templateobjectnotifier.cpp
    templatefieldformat.cpp
    intfieldkeyhandler.cpp
    templateresourcelayout.cpp
    textbrowsermargins.cpp
    texteditformatterframe.cpp
    texteditmargins.cpp
    textpublishdialog.cpp
    texttranslatedialog.cpp
    timeanddateframe.cpp
    tokeneditdialog.cpp
    tokeneditor.cpp
    undodrawaddobject.cpp
    undofowbase.cpp
    undofowfill.cpp
    undofowpath.cpp
    undofowpoint.cpp
    undofowpolygon.cpp
    undofowshape.cpp
    undomarker.cpp
    unselectedellipse.cpp
    unselectedpixmap.cpp
    unselectedpolygon.cpp
    unselectedrect.cpp
    updatechecker.cpp
    videoplayer.cpp
    videoplayergl.cpp
    videoplayerglplayer.cpp
    videoplayerglscreenshot.cpp
    videoplayerglvideo.cpp
    videoplayerscreenshot.cpp
    welcomeframe.cpp
    whatsnewdialog.cpp
)

set(DMHELPER_HEADERS
    # --- Shared library sources ---
    ../../DMHelperShared/inc/dmhglobal.h
    ../../DMHelperShared/inc/dmhlogon.h
    ../../DMHelperShared/inc/dmhnetworkdata.h
    ../../DMHelperShared/inc/dmhnetworkmanager.h
    ../../DMHelperShared/inc/dmhnetworkobserver.h
    ../../DMHelperShared/inc/dmhobjectbase.h
    ../../DMHelperShared/inc/dmhpayload.h
    ../../DMHelperShared/src/dmhlogon_private.h
    ../../DMHelperShared/src/dmhnetworkdata_private.h
    ../../DMHelperShared/src/dmhnetworkdatafactory.h
    ../../DMHelperShared/src/dmhnetworkmanager_private.h
    ../../DMHelperShared/src/dmhnetworkobserver_private.h
    ../../DMHelperShared/src/dmhobjectbase_private.h
    ../../DMHelperShared/src/dmhpayload_private.h
    ../../DMHelperShared/src/dmhshared.h

    # --- Application sources ---
    aboutdialog.h
    attack.h
    audiofactory.h
    audioplaybackframe.h
    audiotrack.h
    audiotrackedit.h
    audiotrackfile.h
    audiotracksyrinscape.h
    audiotracksyrinscapeonline.h
    audiotrackurl.h
    audiotrackyoutube.h
    basicdate.h
    basicdateserver.h
    battlecombatantwidget.h
    battledialogeffectsettings.h
    battledialogeffectsettingsbase.h
    battledialogeffectsettingsfire.h
    battledialogeffectsettingslight.h
    battledialogeffectsettingsobjectvideo.h
    battledialogeffectsettingssmoke.h
    battledialogeffectsettingssparks.h
    battledialogevent.h
    battledialogeventdamage.h
    battledialogeventnewround.h
    battledialogeventresetresources.h
    battledialoggraphicsscene.h
    battledialoggraphicsscenemousehandler.h
    battledialoggraphicsview.h
    battledialoglogger.h
    battledialoglogview.h
    battledialogmodel.h
    battledialogmodelcharacter.h
    battledialogmodelcombatant.h
    battledialogmodelcombatantgroup.h
    battledialogmodelinitiativeevent.h
    battledialogmodeleffect.h
    battledialogmodeleffectcone.h
    battledialogmodeleffectcube.h
    battledialogmodeleffectfactory.h
    battledialogmodeleffectline.h
    battledialogmodeleffectobject.h
    battledialogmodeleffectobjectvideo.h
    battledialogmodeleffectfire.h
    battledialogmodeleffectlight.h
    battledialogmodeleffectsmoke.h
    battledialogmodeleffectsparks.h
    battledialogmodeleffectradius.h
    battledialogmodeleffectshape.h
    battledialogmodelmonsterbase.h
    battledialogmodelmonsterclass.h
    battledialogmodelmonstercombatant.h
    battledialogmodelobject.h
    battleframe.h
    battleframemapdrawer.h
    battleframestate.h
    battleframestatemachine.h
    battletokenhealthbar.h
    bestiary.h
    bestiaryexportdialog.h
    bestiaryfindtokendialog.h
    bestiarypopulatetokensdialog.h
    bestiarytemplatedialog.h
    camerarect.h
    camerascene.h
    campaign.h
    campaignexporter.h
    campaignnotesdialog.h
    campaignobjectbase.h
    campaignobjectfactory.h
    campaignobjectframe.h
    campaignobjectframestack.h
    campaigntree.h
    campaigntreeactivestack.h
    campaigntreeitem.h
    campaigntreemodel.h
    character.h
    characterimportdialog.h
    characterimporter.h
    characterimportheroforge.h
    characterimportheroforgedata.h
    characterimportheroforgedialog.h
    charactertemplateframe.h
    characterv2.h
    characterv2converter.h
    clockframe.h
    colorpushbutton.h
    combatant.h
    combatantdialog.h
    combatantfactory.h
    combatantreference.h
    combatantrolloverframe.h
    combatantselectdialog.h
    combatantgroupwidget.h
    combatanttemplateadapter.h
    combatanttemplateframe.h
    combatantwidget.h
    combatantwidgetbase.h
    complicationdata.h
    conditions.h
    conditionseditdialog.h
    configurelockedgriddialog.h
    countdownframe.h
    countdownsubframe.h
    customtable.h
    customtableentry.h
    customtableframe.h
    dice.h
    dicerolldialog.h
    dicerolldialogcombatants.h
    dicerollframe.h
    discordposter.h
    dmconstants.h
    dmh_opengl.h
    dmh_vlc.h
    dmhcache.h
    dmhelperribbon.h
    dmhfilereader.h
    dmhlogger.h
    dmhwaitingdialog.h
    dmscreentabwidget.h
    dmversion.h
    emptycampaignframe.h
    encounterbattle.h
    encounterfactory.h
    encountertext.h
    encountertextedit.h
    encountertextlinked.h
    equipmentserver.h
    expertisedialog.h
    exportdialog.h
    exportworker.h
    fearframe.h
    fowgraphicsitem.h
    globalsearch.h
    globalsearchframe.h
    grid.h
    gridconfig.h
    gridsizer.h
    helpdialog.h
    initiativelistcombatantwidget.h
    initiativelistdialog.h
    initiativeeventwidget.h
    itemselectdialog.h
    layer.h
    layerblank.h
    layerdraw.h
    layerdrawengine.h
    layerdrawobject.h
    layerdrawshape.h
    layerdrawstate.h
    layerdrawtooldialog.h
    layerseditdialog.h
    layereffect.h
    layereffectsettings.h
    layerfow.h
    layerfowsettings.h
    layerframe.h
    layergrid.h
    layerimage.h
    layerparticle.h
    layerparticlesettings.h
    layerreference.h
    layerscene.h
    layertokens.h
    layertokensdarkeneffect.h
    layervideo.h
    layervideoeffect.h
    layervideoeffectsettings.h
    legaldialog.h
    mainwindow.h
    map.h
    mapblankdialog.h
    mapcolorizedialog.h
    mapcolorizefilter.h
    mapcontent.h
    mapfactory.h
    mapframe.h
    mapframescene.h
    mapmanagerdialog.h
    mapmarker.h
    mapmarkerdialog.h
    mapmarkergraphicsitem.h
    mappartyiconitem.h
    mapselectdialog.h
    monster.h
    monsteraction.h
    monsteractioneditdialog.h
    monsteractionframe.h
    monsterclass.h
    monsterclassv2.h
    monsterclassv2converter.h
    monsterfactory.h
    mruaction.h
    mruhandler.h
    networkcontroller.h
    newcampaigndialog.h
    newentrydialog.h
    objectfactory.h
    objectimportdialog.h
    objectimportworker.h
    optionsaccessor.h
    optionscontainer.h
    optionsdialog.h
    overlay.h
    overlaycounter.h
    overlayfear.h
    overlayframe.h
    overlayrenderer.h
    overlayseditdialog.h
    overlaytimer.h
    party.h
    partycharactergridframe.h
    partyframe.h
    partyframecharacter.h
    perroundresource.h
    placeholder.h
    popup.h
    popupaudio.h
    popupspreviewframe.h
    presentupdatedialog.h
    publishbuttonframe.h
    publishbuttonproxy.h
    publishbuttonribbon.h
    publishframe.h
    publishglbattlebackground.h
    publishglbattleeffect.h
    publishglbattleeffectvideo.h
    publishglbattleeffectanimated.h
    publishglbattleeffectsmoke.h
    publishglbattleeffectfire.h
    publishglbattleeffectsparks.h
    publishglbattleeffectlight.h
    publishglbattlegrid.h
    publishglbattleimagerenderer.h
    publishglbattleobject.h
    publishglbattlerenderer.h
    publishglbattletoken.h
    publishglbattlevideorenderer.h
    publishglframe.h
    publishglimage.h
    publishglimagerenderer.h
    publishglmaprenderer.h
    publishglobject.h
    publishglrect.h
    publishglrenderer.h
    publishglscene.h
    publishgltextrenderer.h
    publishgltokenhighlight.h
    publishgltokenhighlighteffect.h
    publishgltokenhighlighthealthbar.h
    publishgltokenhighlightref.h
    publishwindow.h
    quickref.h
    quickrefdatawidget.h
    quickrefframe.h
    randommarketdialog.h
    ribbonframe.h
    ribbonframetext.h
    ribbonmain.h
    ribbontabaudio.h
    ribbontabbattle.h
    ribbontabbattlemap.h
    ribbontabbattleview.h
    ribbontabcampaign.h
    ribbontabfile.h
    ribbontabmap.h
    ribbontabtext.h
    ribbontabtools.h
    ribbontabworldmap.h
    rulefactory.h
    rulehealth.h
    rulehealth5e.h
    rulehealthdaggerheart.h
    ruleinitiative.h
    ruleinitiative2e.h
    ruleinitiative5e.h
    ruleinitiativegroup.h
    ruleinitiativegroupmonsters.h
    ruleinitiativenone.h
    ruleset.h
    scaledpixmap.h
    selectcombatantdialog.h
    selectitemdialog.h
    selectstringdialog.h
    selectzoom.h
    smoketestrunner.h
    soundboardframe.h
    soundboardgroup.h
    soundboardgroupframe.h
    soundboardtrack.h
    soundboardtrackframe.h
    spell.h
    spellbook.h
    spellbooktemplatedialog.h
    spellbookfactory.h
    spellslotlevelbutton.h
    spellslotradiobutton.h
    spellv2.h
    spellv2converter.h
    templatefactory.h
    templateframe.h
    templateobject.h
    templateobjectnotifier.h
    templatefieldformat.h
    intfieldkeyhandler.h
    templateresourcelayout.h
    textbrowsermargins.h
    texteditformatterframe.h
    texteditmargins.h
    textpublishdialog.h
    texttranslatedialog.h
    timeanddateframe.h
    tokeneditdialog.h
    tokeneditor.h
    undodrawaddobject.h
    undofowbase.h
    undofowfill.h
    undofowpath.h
    undofowpoint.h
    undofowpolygon.h
    undofowshape.h
    undomarker.h
    unselectedellipse.h
    unselectedpixmap.h
    unselectedpolygon.h
    unselectedrect.h
    updatechecker.h
    videoplayer.h
    videoplayergl.h
    videoplayerglplayer.h
    videoplayerglscreenshot.h
    videoplayerglvideo.h
    videoplayerscreenshot.h
    welcomeframe.h
    whatsnewdialog.h

)

set(DMHELPER_FORMS
    aboutdialog.ui
    audioplaybackframe.ui
    audiotrackedit.ui
    battlecombatantwidget.ui
    battledialogeffectsettings.ui
    battledialogeffectsettingsfire.ui
    battledialogeffectsettingslight.ui
    battledialogeffectsettingsobjectvideo.ui
    battledialogeffectsettingssmoke.ui
    battledialogeffectsettingssparks.ui
    battledialoglogview.ui
    battleframe.ui
    bestiaryexportdialog.ui
    bestiaryfindtokendialog.ui
    bestiarypopulatetokensdialog.ui
    bestiarytemplatedialog.ui
    campaignnotesdialog.ui
    characterimportdialog.ui
    characterimportheroforgedialog.ui
    charactertemplateframe.ui
    clockframe.ui
    combatantdialog.ui
    combatantrolloverframe.ui
    combatantselectdialog.ui
    combatantgroupwidget.ui
    combatantwidgetbase.ui
    configurelockedgriddialog.ui
    countdownframe.ui
    countdownsubframe.ui
    customtableframe.ui
    dicerolldialog.ui
    dicerolldialogcombatants.ui
    dicerollframe.ui
    dmhwaitingdialog.ui
    dmscreentabwidget.ui
    emptycampaignframe.ui
    encountertextedit.ui
    expertisedialog.ui
    exportdialog.ui
    fearframe.ui
    globalsearchframe.ui
    helpdialog.ui
    initiativelistcombatantwidget.ui
    initiativelistdialog.ui
    initiativeeventwidget.ui
    itemselectdialog.ui
    layerdrawtooldialog.ui
    layereffectsettings.ui
    layerfowsettings.ui
    layerframe.ui
    layerparticlesettings.ui
    layerseditdialog.ui
    layervideoeffectsettings.ui
    legaldialog.ui
    mainwindow.ui
    mapblankdialog.ui
    mapcolorizedialog.ui
    mapframe.ui
    mapmanagerdialog.ui
    mapmarkerdialog.ui
    mapselectdialog.ui
    monsteractioneditdialog.ui
    monsteractionframe.ui
    newcampaigndialog.ui
    newentrydialog.ui
    objectimportdialog.ui
    optionsdialog.ui
    overlayframe.ui
    overlayseditdialog.ui
    partycharactergridframe.ui
    partyframe.ui
    partyframecharacter.ui
    popupspreviewframe.ui
    presentupdatedialog.ui
    publishbuttonframe.ui
    publishbuttonribbon.ui
    quickrefdatawidget.ui
    quickrefframe.ui
    randommarketdialog.ui
    ribbonframetext.ui
    ribbontabaudio.ui
    ribbontabbattle.ui
    ribbontabbattlemap.ui
    ribbontabbattleview.ui
    ribbontabcampaign.ui
    ribbontabfile.ui
    ribbontabmap.ui
    ribbontabtext.ui
    ribbontabtools.ui
    ribbontabworldmap.ui
    selectcombatantdialog.ui
    selectitemdialog.ui
    selectstringdialog.ui
    selectzoom.ui
    soundboardframe.ui
    soundboardgroupframe.ui
    soundboardtrackframe.ui
    spellbooktemplatedialog.ui
    textpublishdialog.ui
    texttranslatedialog.ui
    timeanddateframe.ui
    tokeneditdialog.ui
    welcomeframe.ui
    whatsnewdialog.ui
)

set(DMHELPER_RESOURCES
    resources.qrc
)

# -------------------------------------------------
# Executable
# -------------------------------------------------
add_executable(DMHelper
    ${DMHELPER_SOURCES}
    ${DMHELPER_HEADERS}
    ${DMHELPER_FORMS}
    ${DMHELPER_RESOURCES}
)

# -------------------------------------------------
# Include paths
# -------------------------------------------------
target_include_directories(DMHelper PRIVATE
    ./
    ../../DMHelperShared/inc
    ../../DMHelperShared/src
)

# -------------------------------------------------
# Qt link libraries
# -------------------------------------------------
target_link_libraries(DMHelper PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Xml
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Network
    Qt6::UiTools
)

# -------------------------------------------------
# Platform-specific configuration
# -------------------------------------------------
if (WIN32)
    target_compile_definitions(DMHelper PRIVATE WIN32)

    target_compile_options(DMHelper PRIVATE
        $<$<CONFIG:Debug>:/FS>
        $<$<CONFIG:Release>:/O2>
    )

    # Windows icon — dmhelper.rc is compiled by rc.exe and embeds dmhelper.ico
    target_sources(DMHelper PRIVATE dmhelper.rc)
    set_target_properties(DMHelper PROPERTIES WIN32_EXECUTABLE TRUE)

    # VLC
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(STATUS "Using 32-bit VLC")
        target_include_directories(DMHelper PRIVATE vlc32)
        target_link_directories(DMHelper PRIVATE vlc32)
    else()
        message(STATUS "Using 64-bit VLC")
        target_include_directories(DMHelper PRIVATE vlc64)
        target_link_directories(DMHelper PRIVATE vlc64)
    endif()

    target_link_libraries(DMHelper PRIVATE libvlc)

elseif (APPLE)
    message(STATUS "Configuring macOS build")

    target_compile_options(DMHelper PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O2>
    )

    target_compile_definitions(DMHelper PRIVATE NDEBUG)

    # Headers are architecture-neutral; dylibs differ by CPU family
    target_include_directories(DMHelper PRIVATE vlcMac)
    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        message(STATUS "Using Apple Silicon VLC (vlcMacArm)")
        target_link_directories(DMHelper PRIVATE vlcMacArm)
    else()
        message(STATUS "Using Intel VLC (vlcMac)")
        target_link_directories(DMHelper PRIVATE vlcMac)
    endif()
    target_link_libraries(DMHelper PRIVATE vlc)

    set_target_properties(DMHelper PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE DMHelper.icns
    )

    set_source_files_properties(
        data/macimg/DMHelper.icns
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
elseif (UNIX AND NOT APPLE)
    message(STATUS "Configuring Linux build")

    target_compile_options(DMHelper PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O2>
    )

    # VLC (bundled VLC 4, matching Win/Mac)
    target_include_directories(DMHelper PRIVATE vlclinux)
    target_link_directories(DMHelper PRIVATE vlclinux)
    target_link_libraries(DMHelper PRIVATE vlc vlccore)
    target_link_options(DMHelper PRIVATE "LINKER:-rpath-link,${CMAKE_CURRENT_SOURCE_DIR}/vlclinux")

    # RPATH so the AppImage can find bundled libraries
    set_target_properties(DMHelper PROPERTIES
        INSTALL_RPATH "$ORIGIN/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# -------------------------------------------------
# Common definitions
# -------------------------------------------------

target_compile_definitions(DMHelper PRIVATE
    $<$<CONFIG:Release>:NDEBUG>
)

# -------------------------------------------------
# Warnings
# -------------------------------------------------
if (MSVC)
    target_compile_options(DMHelper PRIVATE /W3)
else()
    target_compile_options(DMHelper PRIVATE -Wall -Wextra)
endif()
