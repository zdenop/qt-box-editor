cmake_minimum_required(VERSION 3.5)

project(qt-box-editor CXX)
set(CMAKE_CXX_STANDARD 17)

set(QTB_VERSION "1.13.0")
add_definitions(-DAPP_VERSION="${QTB_VERSION}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED Widgets Svg Network)
find_package(tesseract REQUIRED)
find_package(leptonica REQUIRED)

include_directories(${Tesseract_INCLUDE_DIRS} ${Leptonica_INCLUDE_DIRS})
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs ${CMAKE_CURRENT_SOURCE_DIR}/src
)
link_directories(${Tesseract_LIBRARY_DIRS} ${leptonica_LIBRARY_DIRS})
add_definitions(${Tesseract_DEFINITIONS} ${leptonica_DEFINITIONS})

set(project_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ChildWidget.cpp
    src/DelegateEditors.cpp
    src/TessTools.cpp
    dialogs/SettingsDialog.cpp
    dialogs/GetRowIDDialog.cpp
    dialogs/ShortCutsDialog.cpp
    dialogs/FindDialog.cpp
    dialogs/DrawRectangle.cpp)
set(project_HEADERS
    src/MainWindow.h
    src/ChildWidget.h
    src/Settings.h
    src/TessTools.h
    src/DelegateEditors.h
    dialogs/SettingsDialog.h
    dialogs/GetRowIDDialog.h
    dialogs/ShortCutsDialog.h
    dialogs/FindDialog.h
    dialogs/DrawRectangle.h)
set(project_FORMS
    dialogs/ShortCutDialog.ui dialogs/GetRowIDDialog.ui
    dialogs/SettingsDialog.ui dialogs/FindDialog.ui dialogs/DrawRectangle.ui)
set(project_RESOURCES
    resources/application.qrc resources/QBE-GNOME.qrc resources/QBE-Faenza.qrc
    resources/QBE-Oxygen.qrc resources/QBE-Tango.qrc)

# qt5_add_resources(project_SOURCES ${project_RESOURCES})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set (WIN32_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/win.rc)
endif()

add_executable(
  ${CMAKE_PROJECT_NAME}-${QTB_VERSION} ${project_SOURCES} ${project_FORMS}
                               ${project_HEADERS} ${project_RESOURCES} ${WIN32_RESOURCES})
target_link_libraries(
  ${CMAKE_PROJECT_NAME}-${QTB_VERSION} Qt5::Widgets Qt5::Svg Qt5::Network
  ${Leptonica_LIBRARIES} ${Tesseract_LIBRARIES} )
