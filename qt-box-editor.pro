TEMPLATE = app
VERSION = 1.06
TARGET = qt-box-editor-$${VERSION}

DEPENDPATH += ./ \
    resource/images \
    resource

INCLUDEPATH += ./ \
    dialogs \
    src/include \
    src

QT += network \
    svg \
    # testlib

CONFIG += release

OBJECTS_DIR += temp
MOC_DIR += temp
UI_DIR += temp
RCC_DIR += temp
DEFINES += VERSION=\\\"$${VERSION}\\\"

FORMS += \
    dialogs/ShortCutDialog.ui \
    dialogs/GetRowIDDialog.ui \
    src/SettingsDialog.ui \
    dialogs/FindDialog.ui

SOURCES += dialogs/GetRowIDDialog.cpp \
    dialogs/ShortCutsDialog.cpp \
    src/DelegateEditors.cpp \
    src/ChildWidget.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/SettingsDialog.cpp \
    dialogs/FindDialog.cpp

HEADERS += dialogs/GetRowIDDialog.h \
    dialogs/ShortCutsDialog.h \
    src/include/DelegateEditors.h \
    src/include/MainWindow.h \
    src/include/ChildWidget.h \
    src/include/Settings.h \
    src/include/SettingsDialog.h \
    dialogs/FindDialog.h

RESOURCES = resources/application.qrc

win32: {
    DESTDIR = ./win32
    #DIR_SEPARATOR = \
    CONFIG += static release embed_manifest_exe
    TMAKE_CXXFLAGS += -DQT_NODLL
    TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti
    QTPLUGIN += qsvg # image formats
    DEFINES += WINDOWS _COMPOSE_STATIC_
    RC_FILE = resources/win.rc
}



