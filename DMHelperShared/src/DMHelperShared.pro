#-------------------------------------------------
#
# Project created by QtCreator 2018-09-02T10:06:47
#
#-------------------------------------------------

QT       += widgets network xml multimedia

TARGET = DMHelperShared
TEMPLATE = lib
#CONFIG += staticlib change to a dll

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += DMHSHARED_LIB

#DEFINES += DMH_VERBOSE

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/../inc

SOURCES += \
    dmhobjectbase.cpp \
    dmhpayload.cpp \
    dmhnetworkobserver.cpp \
    dmhpayload_private.cpp \
    dmhnetworkmanager.cpp \
    dmhnetworkmanager_private.cpp \
    dmhnetworkobserver_private.cpp \
    dmhlogon.cpp \
    dmhlogon_private.cpp \
    dmhnetworkdatafactory.cpp \
    dmhnetworkdata.cpp \
    dmhnetworkdata_private.cpp \
    dmhshared.cpp \
    dmhobjectbase_private.cpp

HEADERS += \
    ../inc/dmhglobal.h \
    ../inc/dmhobjectbase.h \
    ../inc/dmhpayload.h \
    ../inc/dmhnetworkobserver.h \
    ../inc/dmhnetworkmanager.h \
    ../inc/dmhnetworkdata.h \
    ../inc/dmhlogon.h \
    dmhnetworkmanager_private.h \
    dmhnetworkobserver_private.h \
    dmhlogon_private.h \
    dmhnetworkdatafactory.h \
    dmhpayload_private.h \
    dmhnetworkdata_private.h \
    dmhshared.h \
    dmhobjectbase_private.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

# using shell_path() to correct path depending on platform
# escaping quotes and backslashes for file paths

CONFIG( debug, debug|release ) {
    # debug
    copydata1.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-32_bit-debug\\debug\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperClient\\build-32_bit-debug\\debug\"
    copydata2.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-64_bit-debug\\debug\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperClient\\build-64_bit-debug\\debug\"
    copydata3.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-32_bit-debug\\debug\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperTest\\build-32_bit-debug\\debug\"
    copydata4.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-64_bit-debug\\debug\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperTest\\build-64_bit-debug\\debug\"
    copydata5.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-32_bit-debug\\debug\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelper\\build-32_bit-debug\\debug\"
    copydata6.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-64_bit-debug\\debug\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelper\\build-64_bit-debug\\debug\"
    first.depends = $(first) copydata1 copydata2 copydata3 copydata4 copydata5 copydata6
    export(first.depends)
    export(copydata1.commands)
    export(copydata2.commands)
    export(copydata3.commands)
    export(copydata4.commands)
    export(copydata5.commands)
    export(copydata6.commands)
    QMAKE_EXTRA_TARGETS += first copydata1 copydata2 copydata3 copydata4 copydata5 copydata6
} else {
    # release
    copydata1.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-32_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperClient\\build-32_bit-release\\release\"
    copydata2.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-64_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperClient\\build-64_bit-release\\release\"
    copydata3.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-32_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperTest\\build-32_bit-release\\release\"
    copydata4.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-64_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperTest\\build-64_bit-release\\release\"
    copydata5.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-32_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelper\\build-32_bit-release\\release\"
    copydata6.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-64_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelper\\build-64_bit-release\\release\"
    copydata7.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-32_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperClient\\bin32\"
    copydata8.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-64_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperClient\\bin64\"
    copydata9.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-32_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperTest\\bin32\"
    copydata10.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-64_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelperTest\\bin64\"
    copydata11.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-32_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelper\\bin32\"
    copydata12.commands = $(COPY_FILE) \"$$shell_path($$OUT_PWD\\..\\build-64_bit-release\\release\\DMHelperShared.dll)\" \"$$shell_path($$PWD)\\..\\..\\DMHelper\\bin64\"
    first.depends = $(first) copydata1 copydata2 copydata3 copydata4 copydata5 copydata6 copydata7 copydata8 copydata9 copydata10 copydata11 copydata12
    export(first.depends)
    export(copydata1.commands)
    export(copydata2.commands)
    export(copydata3.commands)
    export(copydata4.commands)
    export(copydata5.commands)
    export(copydata6.commands)
    export(copydata7.commands)
    export(copydata8.commands)
    export(copydata9.commands)
    export(copydata10.commands)
    export(copydata11.commands)
    export(copydata12.commands)
    QMAKE_EXTRA_TARGETS += first copydata1 copydata2 copydata3 copydata4 copydata5 copydata6 copydata7 copydata8 copydata9 copydata10 copydata11 copydata12
}


