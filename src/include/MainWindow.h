/**********************************************************************
 * File:        MainWindow.h
 * Description: MainWindow functions
 * Author:      Marcel Kolodziejczyk
 * Created:     2010-01-04
 *
 * (C) Copyright 2010, Marcel Kolodziejczyk
 * (C) Copyright 2011, Zdenko Podobny
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **    http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QSignalMapper>
#include <QCloseEvent>
#include <QFileDialog>
#include <QStatusBar>
#include <QEventLoop>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QList>
#include <QSettings>
#include <QMessageBox>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

class ChildWidget;
class QAction;
class QMenu;
class QTabWidget;
class QSignalMapper;

class MainWindow: public QMainWindow
{
Q_OBJECT
QNetworkAccessManager manager;

public:
  MainWindow();

  void addChild(const QString &imageFileName);

public slots:
  void checkForUpdate();
  void requestFinished(QNetworkReply *);

protected:

  void closeEvent(QCloseEvent *event);

private slots:
    void open();
    void openRecentFile();
    void save();
    void saveAs();
    bool closeActiveTab();
    bool closeAllTabs();
    void nextTab();
    void previousTab();
    void bold(bool checked);
    void italic(bool checked);
    void underline(bool checked);
    void zoomOriginal();
    void zoomIn();
    void zoomOut();
    void splitSymbol();
    void joinSymbol();
    void deleteSymbol();
    void about();
    void aboutQt();
    void handleClose(int i);
    void updateMenus();
    void updateViewMenu();
    void updateCommandActions();
    void updateTabTitle();
    void updateSaveAction();

private:
    ChildWidget *activeChild();
    void createActions();
    void createMenus();
    void createToolBars();
    void readSettings();
    void writeSettings();
    void checkVersion(QNetworkReply *);
    void updateRecentFileActions();

    QTabWidget *tabWidget;

    QSignalMapper *windowMapper;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    enum { MaxRecentFiles = 8 };
    QAction *recentFileActs[MaxRecentFiles];

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *viewToolBar;

    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *closeAct;
    QAction *closeAllAct;
    QAction *fSeparatorAct;
    QAction *exitAct;

    QAction *boldAct;
    QAction *italicAct;
    QAction *underlineAct;

    QAction *zoomOriginalAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *separatorAct;

    QAction *splitAct;
    QAction *joinAct;
    QAction *deleteAct;
    QAction *checkForUpdateAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif /* MAINWINDOW_H_ */
