TEMPLATE = app
VERSION = 1.13dev
TARGET = qt-box-editor-$${VERSION}

DEPENDPATH += ./ \
    resource/images \
    resource

INCLUDEPATH += ./ \
    dialogs \
    src/include \
    src

QT += network svg
#QT += testlib

#CONFIG += debug warn_on
CONFIG += release warn_off

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
    resources/QBE-GNOME.qrc \
    resources/QBE-Faenza.qrc \
    resources/QBE-Oxygen.qrc \
    resources/QBE-Tango.qrc

LIBS += -llept -ltesseract

win32 {
    DESTDIR = ./win32
    CONFIG += release embed_manifest_exe
    TMAKE_CXXFLAGS += -DQT_NODLL
    TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti -static
    #QTPLUGIN += qsvg # image formats
     DEFINES += WINDOWS
    #QMAKE_LFLAGS.gcc += -static-libgcc # -static
    RC_FILE = resources/win.rc
    INCLUDEPATH += $$PWD/win32-external/include/
    LIBS += -lws2_32 -L$$PWD/win32-external/lib
}

unix:!macx {
    message(Starting UNIX build...)
    greaterThan(QT_MAJOR_VERSION, 5) {
      message(Qt $$[QT_VERSION] was detected.)
      QT += widgets
      INCLUDEPATH += /opt/include/
      LIBS += -L/opt/lib
    }
}

# Libraries may be installed this way on Mac OS X:
# brew install leptonica
# brew install tesseract
macx {
    message(Starting OSX build...)
    QT += widgets
    INCLUDEPATH += /usr/local/include/
    LIBS += -L/usr/local/lib
}
