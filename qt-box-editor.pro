TEMPLATE = app
VERSION = 1.04dev
TARGET = qt-box-editor-$${VERSION}

DEPENDPATH += ./ \
    resource/images \
    resource

INCLUDEPATH += ./ \
    dialogs \
    src/include \
    src

QT += network svg \
    testlib

OBJECTS_DIR += temp
MOC_DIR += temp
UI_DIR += temp
RCC_DIR += temp
DEFINES += VERSION=\\\"$${VERSION}\\\"

FORMS += \
    dialogs/ShortCutDialog.ui

SOURCES += src/main.cpp \
    src/MainWindow.cpp \
    src/ChildWidget.cpp \
    src/SettingsDialog.cpp \
    dialogs/ShortCutsDialog.cpp

HEADERS += src/include/MainWindow.h \
    src/include/ChildWidget.h \
    src/include/Settings.h \
    src/include/SettingsDialog.h \
    dialogs/ShortCutsDialog.h

RESOURCES = resources/application.qrc

win32: {
    DESTDIR = ./win32
    DIR_SEPARATOR = \
    CONFIG += static release embed_manifest_exe
    QTPLUGIN += qsvg # image formats
    TMAKE_CXXFLAGS += -DQT_NODLL 
    TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti
    DEFINES += WINDOWS _COMPOSE_STATIC_
    RC_FILE = resources/win.rc
}
