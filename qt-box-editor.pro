TEMPLATE = app
VERSION = 1.04
TARGET = qt-box-editor-$${VERSION}

DEPENDPATH += ./ \
    resource/images \
    resource

INCLUDEPATH += ./ \
    dialogs \
    src/include \
    src

QT += network svg
   # testlib

CONFIG += release 

OBJECTS_DIR += temp
MOC_DIR += temp
UI_DIR += temp
RCC_DIR += temp
DEFINES += VERSION=\\\"$${VERSION}\\\"

FORMS += \
    dialogs/ShortCutDialog.ui \
    dialogs/GetRowIDDialog.ui

SOURCES += src/main.cpp \
    src/MainWindow.cpp \
    src/ChildWidget.cpp \
    src/SettingsDialog.cpp \
    dialogs/ShortCutsDialog.cpp \
    dialogs/GetRowIDDialog.cpp

HEADERS += src/include/MainWindow.h \
    src/include/ChildWidget.h \
    src/include/Settings.h \
    src/include/SettingsDialog.h \
    dialogs/ShortCutsDialog.h \
    dialogs/GetRowIDDialog.h

RESOURCES = resources/application.qrc

win32: {
    DESTDIR = ./win32
    DIR_SEPARATOR = \
    CONFIG += static release embed_manifest_exe
    TMAKE_CXXFLAGS += -DQT_NODLL 
    TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti
    QTPLUGIN += qsvg # image formats
    DEFINES += WINDOWS _COMPOSE_STATIC_
    RC_FILE = resources/win.rc
}
