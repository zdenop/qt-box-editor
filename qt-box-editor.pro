TEMPLATE = app
VERSION = 1.09dev
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
    testlib

# CONFIG += release

OBJECTS_DIR += temp
MOC_DIR += temp
UI_DIR += temp
RCC_DIR += temp
DEFINES += VERSION=\\\"$${VERSION}\\\"

FORMS += \
    dialogs/ShortCutDialog.ui \
    dialogs/GetRowIDDialog.ui \
    dialogs/SettingsDialog.ui \
    dialogs/FindDialog.ui \
    dialogs/DrawRectangle.ui

SOURCES += src/main.cpp \
    src/MainWindow.cpp \
    src/ChildWidget.cpp \
    src/DelegateEditors.cpp \
    src/TessTools.cpp \
    dialogs/SettingsDialog.cpp \
    dialogs/GetRowIDDialog.cpp \
    dialogs/ShortCutsDialog.cpp \
    dialogs/FindDialog.cpp \
    dialogs/DrawRectangle.cpp

HEADERS += src/MainWindow.h \
    src/ChildWidget.h \
    src/Settings.h \
    src/TessTools.h \
    src/DelegateEditors.h \
    dialogs/SettingsDialog.h \
    dialogs/GetRowIDDialog.h \
    dialogs/ShortCutsDialog.h \
    dialogs/FindDialog.h \
    dialogs/DrawRectangle.h

RESOURCES = resources/application.qrc \
    resources/faenza.qrc \
    resources/tango.qrc

win32: {
    DESTDIR = ./win32
    #DIR_SEPARATOR = \
    CONFIG += static release embed_manifest_exe
    TMAKE_CXXFLAGS += -DQT_NODLL
    TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti -static
    #QTPLUGIN += qsvg # image formats
    #DEFINES += _COMPOSE_STATIC_
    DEFINES += WINDOWS __BLOB_T_DEFINED
    #QMAKE_LFLAGS.gcc += -static-libgcc # -static
    RC_FILE = resources/win.rc
    INCLUDEPATH +=  win32-external/include/tesseract/ \
                    win32-external/include/leptonica/
    LIBS += -ltesseract -llept -lws2_32 -Lwin32-external/lib
}

unix: {
    INCLUDEPATH += /usr/include/leptonica /usr/local/include/tesseract/
    LIBS += -llept -ltesseract
}

