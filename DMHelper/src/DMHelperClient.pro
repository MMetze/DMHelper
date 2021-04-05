#-------------------------------------------------
#
# Project created by QtCreator 2018-09-01T23:28:10
#
#-------------------------------------------------

QT       += core gui widgets network multimedia xml

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
        audiofactory.cpp \
        audioplayer.cpp \
        audiotrack.cpp \
        audiotrackfile.cpp \
        audiotrackfiledownload.cpp \
        audiotracksyrinscape.cpp \
        audiotrackurl.cpp \
        audiotrackyoutube.cpp \
        campaignobjectbase.cpp \
        campaignobjectfactory.cpp \
        campaignobjectrenderer.cpp \
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
        encountertext.cpp \
        encountertextdownload.cpp \
        encountertextrenderer.cpp \
        objectfactory.cpp \
        placeholder.cpp \
        remoteaudioplayer.cpp \
        remoterenderer.cpp \
        videoplayer.cpp

HEADERS += \
    audiofactory.h \
    audioplayer.h \
    audiotrack.h \
    audiotrackfile.h \
    audiotrackfiledownload.h \
    audiotracksyrinscape.h \
    audiotrackurl.h \
    audiotrackyoutube.h \
    campaignobjectbase.h \
    campaignobjectfactory.h \
    campaignobjectrenderer.h \
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
    encountertext.h \
    encountertextdownload.h \
    encountertextrenderer.h \
    objectfactory.h \
    placeholder.h \
    remoteaudioplayer.h \
    remoterenderer.h \
    videoplayer.h


FORMS += \
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
