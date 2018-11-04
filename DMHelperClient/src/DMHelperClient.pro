#-------------------------------------------------
#
# Project created by QtCreator 2018-09-01T23:28:10
#
#-------------------------------------------------

QT       += core gui widgets network multimedia xml

TARGET = DMHelperClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    settingsdialog.cpp \
    optionscontainer.cpp \
    dmhsrc/battledialoggraphicsscene.cpp \
    dmhsrc/unselectedpixmap.cpp \
    battledialog.cpp \
    dmhsrc/grid.cpp \
    battledialogeffectsettings.cpp \
    dmhsrc/battledialogmodel.cpp \
    dmhsrc/battledialogmodelcharacter.cpp \
    dmhsrc/battledialogmodelcombatant.cpp \
    dmhsrc/battledialogmodeleffect.cpp \
    dmhsrc/battledialogmodeleffectcone.cpp \
    dmhsrc/battledialogmodeleffectcube.cpp \
    dmhsrc/battledialogmodeleffectfactory.cpp \
    dmhsrc/battledialogmodeleffectline.cpp \
    dmhsrc/battledialogmodeleffectradius.cpp \
    dmhsrc/battledialogmodelmonsterbase.cpp \
    battledialoggraphicsview.cpp

HEADERS += \
        mainwindow.h \
    settingsdialog.h \
    optionscontainer.h \
    dmhsrc/battledialoggraphicsscene.h \
    dmhsrc/dmconstants.h \
    dmhsrc/unselectedpixmap.h \
    battledialog.h \
    dmhsrc/grid.h \
    battledialogeffectsettings.h \
    dmhsrc/battledialogmodel.h \
    dmhsrc/battledialogmodelcharacter.h \
    dmhsrc/battledialogmodelcombatant.h \
    dmhsrc/battledialogmodeleffect.h \
    dmhsrc/battledialogmodeleffectcone.h \
    dmhsrc/battledialogmodeleffectcube.h \
    dmhsrc/battledialogmodeleffectfactory.h \
    dmhsrc/battledialogmodeleffectline.h \
    dmhsrc/battledialogmodeleffectradius.h \
    dmhsrc/battledialogmodelmonsterbase.h \
    battledialoggraphicsview.h

FORMS += \
        mainwindow.ui \
    settingsdialog.ui \
    battledialog.ui \
    battledialogeffectsettings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

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

