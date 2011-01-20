/**********************************************************************
 * File:        MainWindow.h
 * Description: MainWindow functions
 * Author:      Marcel Kolodziejczyk
 * Created:     2010-01-04
 *
 * (C) Copyright 2010, Marcel Kolodziejczyk
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

#include <QMainWindow>

class ChildWidget;
class QAction;
class QMenu;
class QTabWidget;
class QSignalMapper;

class MainWindow: public QMainWindow
{
Q_OBJECT

public:
  MainWindow();

  void addChild(const QString &imageFileName);

protected:

  void closeEvent(QCloseEvent *event);

private slots:
    void open();
    void save();
    bool closeActiveTab();
    bool closeAllTabs();
    void nextTab();
    void previousTab();
    void bold(bool checked);
    void italic(bool checked);
    void underline(bool checked);
    void zoomIn();
    void zoomOut();
    void splitSymbol();
    void joinSymbol();
    void deleteSymbol();
    void about();
    void handleClose(int i);
    void updateMenus();
    void updateFileMenu();
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

    QTabWidget *tabWidget;

    QSignalMapper *windowMapper;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *viewToolBar;

    QAction *openAct;
    QAction *saveAct;
    QAction *closeAct;
    QAction *closeAllAct;
    QAction *exitAct;

    QAction *boldAct;
    QAction *italicAct;
    QAction *underlineAct;

    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *separatorAct;

    QAction *splitAct;
    QAction *joinAct;
    QAction *deleteAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif /* MAINWINDOW_H_ */
