/**********************************************************************
* File:        MainWindow.h
* Description: MainWindow functions
* Author:      Marcel Kolodziejczyk
* Created:     2010-01-04
*
* (C) Copyright 2010, Marcel Kolodziejczyk
* (C) Copyright 2011-2012, Zdenko Podobny
* (C) Copyright 2012, Zohar Gofer
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**        http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*
**********************************************************************/

#ifndef SRC_INCLUDE_MAINWINDOW_H_
#define SRC_INCLUDE_MAINWINDOW_H_

#include <QEventLoop>
#include <QList>
#include <QSettings>
#include <QSignalMapper>
#include <QUrl>
#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFont>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QStyle>
#include <QStyleFactory>
#include <QToolBar>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "ChildWidget.h"
#include "Settings.h"
#include "SettingsDialog.h"

class ChildWidget;
class QAction;
class QMenu;
class QTabWidget;
class QSignalMapper;
class ShortCutsDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
    QNetworkAccessManager manager;

  public:
    MainWindow();

    void addChild(const QString& imageFileName);
    SettingsDialog* runSettingsDialog;
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent* event);

  public slots:
    void checkForUpdate();
    void requestFinished(QNetworkReply* reply);

  protected:
    void closeEvent(QCloseEvent* event);

  private slots:
    void open();
    void openRecentFile();
    void save();
    void splitToFeatureBF();
    void saveAs();
    void genBoxFile();
    void getBinImage();
    void reLoad();
    void reLoadImg();
    void importPLSym();
    void importTextSym();
    void symbolPerLine();
    void rowPerLine();
    void paragraphPerLine();
    bool closeActiveTab();
    bool closeAllTabs();
    void nextTab();
    void previousTab();
    void bold(bool checked);
    void italic(bool checked);
    void underline(bool checked);
    void directTypingMode(bool checked);
    void showFontColumns(bool checked);
    void zoomToFit();
    void zoomToHeight();
    void zoomToWidth();
    void zoomOriginal();
    void zoomToSelection();
    void zoomIn();
    void zoomOut();
    void showSymbol();
    void drawBoxes();
    void insertSymbol();
    void splitSymbol();
    void joinSymbol();
    void deleteSymbol();
    void moveUp();
    void moveDown();
    void moveTo();
    void goToRow();
    void find();
    void drawRect(bool checked);
    void undo();
    void redo();
    void reReadSetting();
    void slotSettings();
    void shortCutList();
    void about();
    void aboutQt();
    void handleClose(int i);
    void updateMenus();
    void updateViewMenu();
    void updateCommandActions();
    void updateTabTitle();
    void updateSaveAction();
    void zoomRatioChanged(qreal);

  private:
    ShortCutsDialog* shortCutsDialog;
    ChildWidget* activeChild();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void checkVersion(QNetworkReply* reply);
    void updateRecentFileActions();

    QTabWidget* tabWidget;

    QSignalMapper* windowMapper;

    QMenu* fileMenu;
    QMenu* importMenu;
    QMenu* exportMenu;
    QMenu* editMenu;
    QMenu* viewMenu;
    QMenu* tessMenu;
    QMenu* helpMenu;

    enum { MaxRecentFiles = 8 };
    QAction* recentFileActs[MaxRecentFiles];

    QToolBar* fileToolBar;
    QToolBar* editToolBar;
    QToolBar* viewToolBar;

    QAction* openAct;
    QAction* saveAct;
    QAction* splitToFeatureBFAct;
    QAction* saveAsAct;
    QAction* reLoadAct;
    QAction* reLoadImgAct;
    QAction* importPLSymAct;
    QAction* importTextSymAct;
    QAction* symbolPerLineAct;
    QAction* rowPerLineAct;
    QAction* paragraphPerLineAct;
    QAction* closeAct;
    QAction* closeAllAct;
    QAction* fSeparatorAct;
    QAction* exitAct;

    QAction* boldAct;
    QAction* italicAct;
    QAction* underlineAct;

    QAction* settingsAct;

    QAction* zoomOriginalAct;
    QAction* zoomToSelectionAct;
    QAction* zoomToFitAct;
    QAction* zoomToHeightAct;
    QAction* zoomToWidthAct;
    QAction* zoomInAct;
    QAction* zoomOutAct;
    QAction* showSymbolAct;
    QAction* drawBoxesAct;
    QAction* DirectTypingAct;
    QAction* showFontColumnsAct;
    QAction* nextAct;
    QAction* previousAct;
    QAction* separatorAct;

    QAction* splitAct;
    QAction* insertAct;
    QAction* joinAct;
    QAction* deleteAct;
    QAction* moveUpAct;
    QAction* moveToAct;
    QAction* moveDownAct;
    QAction* goToRowAct;
    QAction* findAct;
    QAction* drawRectAct;
    QAction* undoAct;
    QAction* redoAct;
    QAction* genBoxAct;
    QAction* getBinAct;
    QAction* checkForUpdateAct;
    QAction* shortCutListAct;
    QAction* aboutAct;
    QAction* aboutQtAct;
    QLabel* _utfCodeLabel;
    QLabel* _boxsize;
    QLabel* _zoom;
};

#endif  // SRC_INCLUDE_MAINWINDOW_H_
