#-------------------------------------------------
#
# Project created by QtCreator 2018-09-01T23:28:10
#
#-------------------------------------------------

QT       += core gui widgets network xml multimedia

TARGET = DMHelperTest
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
    settingsdialog.cpp

HEADERS += \
        mainwindow.h \
    settingsdialog.h

FORMS += \
        mainwindow.ui \
    settingsdialog.ui

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

