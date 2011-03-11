/**********************************************************************
* File:        MainWindow.cpp
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

#include "MainWindow.h"

MainWindow::MainWindow()
{
  tabWidget = new QTabWidget;

#if QT_VERSION >= 0x040500
  tabWidget->setTabsClosable(true);
  tabWidget->setMovable(true);
#endif

  connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(handleClose(int)));
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateMenus()));
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateCommandActions()));
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateSaveAction()));

  setCentralWidget(tabWidget);

  windowMapper = new QSignalMapper(this);
  connect(windowMapper, SIGNAL(mapped(int)), tabWidget, SLOT(setCurrentIndex(int)));

  createActions();
  createMenus();
  createToolBars();
  updateMenus();
  updateCommandActions();
  updateSaveAction();
  readSettings();
  setUnifiedTitleAndToolBarOnMac(true);
  setWindowIcon(QIcon(":/images/qbe.png"));
  setWindowTitle(tr("%1 - v%2").arg(SETTING_APPLICATION).arg(VERSION));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  if (closeAllTabs())
    {
      writeSettings();
      event->accept();
    }
  else
    {
      event->ignore();
    }
}

ChildWidget* MainWindow::activeChild()
{
  if (QWidget* currentWidget = tabWidget->currentWidget())
    return qobject_cast<ChildWidget*> (currentWidget);
  return 0;
}

void MainWindow::open()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);
  QString last_path = settings.value("last_path").toString();   // try to get path from settings

  QString imageFile = QFileDialog::getOpenFileName(
                        this,
                        tr("Select image file..."),
                        last_path,
                        tr("Image files (*.bmp *.png *.jpeg *.jpg *.tif *.tiff);;Tiff files (*.tif *.tiff);;All files (*.*)"));

  addChild(imageFile);
}

void MainWindow::addChild(const QString& imageFileName)
{
  if (!imageFileName.isEmpty())
    {
      QString canonicalImageFileName = QFileInfo(imageFileName).canonicalFilePath();
      for (int i = 0; i < tabWidget->count(); ++i)
        {
          ChildWidget* child = qobject_cast<ChildWidget*> (tabWidget->widget(i));
          if (canonicalImageFileName == child->canonicalImageFileName())
            {
              tabWidget->setCurrentIndex(i);
              return;
            }
        }

      ChildWidget* child = new ChildWidget(this);
      if (child->loadImage(imageFileName))
        {
          statusBar()->showMessage(tr("File loaded"), 2000);
          tabWidget->setCurrentIndex(tabWidget->addTab(child, child->userFriendlyCurrentFile()));
          connect(child, SIGNAL(boxChanged()), this, SLOT(updateCommandActions()));
          connect(child, SIGNAL(modifiedChanged()), this, SLOT(updateTabTitle()));
          connect(child, SIGNAL(modifiedChanged()), this, SLOT(updateSaveAction()));

          // save path of open image file
          QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);
          QString filePath = QFileInfo(imageFileName).absolutePath();
          settings.setValue("last_path", filePath);

          QStringList files = settings.value("recentFileList").toStringList();
          files.removeAll(imageFileName);
          files.prepend(imageFileName);
          while (files.size() > MaxRecentFiles)
            files.removeLast();

          settings.setValue("recentFileList", files);

          foreach(QWidget * widget, QApplication::topLevelWidgets())
          {
            MainWindow* mainWin = qobject_cast<MainWindow*>(widget);
            if (mainWin)
              mainWin->updateRecentFileActions();
          }
        }
      else
        {
          child->close();
        }
    }
}

void MainWindow::updateRecentFileActions()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);
  QStringList files = settings.value("recentFileList").toStringList();

  int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

  for (int i = 0; i < numRecentFiles; ++i)
    {
      QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());

      recentFileActs[i]->setText(text);
      recentFileActs[i]->setData(files[i]);
      recentFileActs[i]->setVisible(true);
    }
  for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    recentFileActs[j]->setVisible(false);

  fSeparatorAct->setVisible(numRecentFiles > 0);
}

void MainWindow::save()
{
  QString fileName = activeChild()->currentBoxFile();

  if (activeChild() && activeChild()->save(fileName))
    statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::saveAs()
{
  // Make a copy but do not update title of tab etc.
  // because theere is not correcponding image file
  // or should be SaveAs image?
  QString currentFileName = activeChild()->currentBoxFile();
  QString fileName = QFileDialog::getSaveFileName(this,
                     tr("Save a copy of box file..."),
                     currentFileName,
                     tr("Tesseract-ocr box files (*.box);;All files (*)"));

  if (fileName.isEmpty())
    return;

  if (activeChild() && activeChild()->save(fileName))
    statusBar()->showMessage(tr("File saved"), 2000);
}

bool MainWindow::closeActiveTab()
{
  if (tabWidget->currentWidget() && tabWidget->currentWidget()->close())
    {
      tabWidget->removeTab(tabWidget->currentIndex());
      return true;
    }
  return false;
}

bool MainWindow::closeAllTabs()
{
  while (tabWidget->currentWidget())
    {
      if (!closeActiveTab())
        return false;
    }
  return true;
}

void MainWindow::openRecentFile()
{
  QAction* action = qobject_cast<QAction*>(sender());

  if (action)
    addChild(action->data().toString());
}

void MainWindow::nextTab()
{
  if (tabWidget->count() > 0)
    tabWidget->setCurrentIndex((tabWidget->currentIndex() + 1) % tabWidget->count());
}

void MainWindow::previousTab()
{
  if (tabWidget->count() > 0)
    {
      tabWidget->setCurrentIndex((tabWidget->currentIndex() + tabWidget->count() - 1) % tabWidget->count());
    }
}

void MainWindow::bold(bool checked)
{
  if (activeChild())
    {
      activeChild()->setBolded(checked);
    }
}

void MainWindow::italic(bool checked)
{
  if (activeChild())
    {
      activeChild()->setItalic(checked);
    }
}

void MainWindow::underline(bool checked)
{
  if (activeChild())
    {
      activeChild()->setUnderline(checked);
    }
}

void MainWindow::zoomOriginal()
{
  if (activeChild())
    {
      activeChild()->zoomOriginal();
      statusBar()->showMessage(tr("Zoomed to original size"), 2000);
    }
}

void MainWindow::zoomToSelection()
{
  if (activeChild())
    {
      activeChild()->zoomToSelection();
      statusBar()->showMessage(tr("Zoomed to fit in view"), 2000);
    }
}

void MainWindow::zoomIn()
{
  if (activeChild())
    {
      activeChild()->zoomIn();
      statusBar()->showMessage(tr("Zoomed in"), 2000);
    }
}

void MainWindow::zoomOut()
{
  if (activeChild())
    {
      activeChild()->zoomOut();
      statusBar()->showMessage(tr("Zoomed out"), 2000);
    }
}

void MainWindow::zoomToFit()
{
  if (activeChild())
    {
      activeChild()->zoomToFit();
      statusBar()->showMessage(tr("Zoomed to fit in image view"), 2000);
    }
}

void MainWindow::zoomToHeight()
{
  if (activeChild())
    {
      activeChild()->zoomToHeight();
      statusBar()->showMessage(tr("Zoomed to fit image height to current view"), 2000);
    }
}

void MainWindow::zoomToWidth()
{
  if (activeChild())
    {
      activeChild()->zoomToWidth();
      statusBar()->showMessage(tr("Zoomed to fit image width to current view"), 2000);
    }
}

void MainWindow::drawBoxes()
{
  if (activeChild())
    {
      activeChild()->drawBoxes();
    }
}

void MainWindow::insertSymbol()
{
  if (activeChild())
    {
      activeChild()->insertSymbol();
      statusBar()->showMessage(tr("Insert symbol"), 2000);
    }
}

void MainWindow::splitSymbol()
{
  if (activeChild())
    {
      activeChild()->splitSymbol();
      statusBar()->showMessage(tr("Split symbol"), 2000);
    }
}

void MainWindow::joinSymbol()
{
  if (activeChild())
    {
      activeChild()->joinSymbol();
      statusBar()->showMessage(tr("Join symbol"), 2000);
    }
}

void MainWindow::deleteSymbol()
{
  if (activeChild())
    {
      activeChild()->deleteSymbol();
      statusBar()->showMessage(tr("Delete symbol"), 2000);
    }
}

void MainWindow::slotSettings()
{
  runSettingsDialog = new SettingsDialog(this);
  runSettingsDialog->exec();
}

void MainWindow::checkForUpdate()
{
  statusBar()->showMessage(tr("Checking for new version..."), 2000);
  QNetworkRequest request;

  QNetworkAccessManager* manager = new QNetworkAccessManager();
  request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");
  request.setUrl(QUrl(UPDATE_URL));

  QNetworkReply* reply = manager->get(request);

  QEventLoop* loop = new QEventLoop;

  QObject::connect(manager, SIGNAL(finished(QNetworkReply*)),
                   loop, SLOT(quit()));
  loop->exec();

  checkVersion(reply);
  delete manager;
}

void MainWindow::requestFinished(QNetworkReply* reply)
{
  checkVersion(reply);
}

void MainWindow::checkVersion(QNetworkReply* reply)
{
  if (reply->error() == QNetworkReply::NoError)
    {
      float current_version = QString(reply->readAll()).toFloat();
      float app_version = (QString("%1").arg(VERSION).replace("dev", "", Qt::CaseInsensitive)).toFloat();

      QString messageText;

      if (app_version == current_version)
        {
          messageText = tr("<p>No newer version is available.</p>");
        }
      else if (app_version > current_version)
        {
          messageText = tr("<p>Your version ('%1') is higher than released stable version ('%2').<p>").arg(VERSION).arg(current_version);
          messageText += tr("<p>Do you use develepment version? ");
          messageText += tr("Don't forget to install stable version manually!</p>");
        }
      else
        {
          messageText = tr("<p>New version '%1' is available!<br/>Please visit ").arg(current_version);
          messageText += tr("<a href=%1/downloads>downloads on project homepage!</a></p>").arg(PROJECT_URL);
        }

      QMessageBox::information(this, tr("Version info"), messageText);
    }
  else
    {
      QMessageBox::critical(this, tr("Network"), tr("ERROR: %1").arg(reply->errorString()));
    }
}

void MainWindow::about()
{
  QString abouttext = tr("<h1>%1 %3</h1>").arg(SETTING_APPLICATION).arg(VERSION);

  abouttext.append(tr("<p>QT4 editor of tesseract-ocr box files</p>"));
  abouttext.append(tr("<p>Project page: <a href=%1>%2</a></p>").arg(PROJECT_URL).arg(PROJECT_URL_NAME));
  abouttext.append(tr("<p>Copyright 2010 Marcel Kolodziejczyk<br/>Copyright 2011 Zdenko Podobný</p>"));
  abouttext.append(tr("<p>This software is released under "
                      "<a href=\"http://www.apache.org/licenses/LICENSE-2.0\">Apache License 2.0</a></p>"));
  QMessageBox::about(this, tr("About application"), abouttext);
}

void MainWindow::aboutQt()
{
  QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::handleClose(int i)
{
  if (tabWidget->widget(i) && tabWidget->widget(i)->close())
    tabWidget->removeTab(i);
}

void MainWindow::updateMenus()
{
  saveAsAct->setEnabled((activeChild()) != 0);
  closeAct->setEnabled(activeChild() != 0);
  closeAllAct->setEnabled(activeChild() != 0);
  nextAct->setEnabled(activeChild() != 0);
  previousAct->setEnabled(activeChild() != 0);
  separatorAct->setVisible(activeChild() != 0);
  zoomOriginalAct->setEnabled(activeChild() != 0);
  zoomInAct->setEnabled(activeChild() != 0);
  zoomOutAct->setEnabled(activeChild() != 0);
  zoomToFitAct->setEnabled(activeChild() != 0);
  zoomToHeightAct->setEnabled(activeChild() != 0);
  zoomToWidthAct->setEnabled(activeChild() != 0);
  drawBoxesAct->setEnabled(activeChild() != 0);
}

void MainWindow::updateCommandActions()
{
  bool enable = (activeChild()) ? activeChild()->isBoxSelected() : false;

  zoomToSelectionAct->setEnabled(enable);
  boldAct->setEnabled(enable);
  boldAct->setChecked((activeChild()) ? activeChild()->isBold() : false);
  italicAct->setEnabled(enable);
  italicAct->setChecked((activeChild()) ? activeChild()->isItalic() : false);
  underlineAct->setEnabled(enable);
  underlineAct->setChecked((activeChild()) ? activeChild()->isUnderLine() : false);
  insertAct->setEnabled(enable);
  splitAct->setEnabled(enable);
  joinAct->setEnabled(enable);
  deleteAct->setEnabled(enable);
}

void MainWindow::updateSaveAction()
{
  saveAct->setEnabled((activeChild()) ? activeChild()->isModified() : false);
}

void MainWindow::updateTabTitle()
{
  if (activeChild())
    {
      QString title = activeChild()->userFriendlyCurrentFile();
      if (activeChild()->isModified())
        title += " *";
      tabWidget->setTabText(tabWidget->currentIndex(), title);
    }
}

void MainWindow::updateViewMenu()
{
  viewMenu->clear();
  viewMenu->addAction(nextAct);
  viewMenu->addAction(previousAct);
  viewMenu->addAction(separatorAct);

  separatorAct->setVisible(tabWidget->count() > 0);

  for (int i = 0; i < tabWidget->count(); ++i)
    {
      ChildWidget* child = qobject_cast<ChildWidget*> (tabWidget->widget(i));

      QString text;
      if (i < 9)
        {
          text = tr("&%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
        }
      else
        {
          text = tr("%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
        }
      QAction* action = viewMenu->addAction(text);
      action->setCheckable(true);
      action->setChecked(child == activeChild());
      connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
      windowMapper->setMapping(action, i);
    }

  viewMenu->addSeparator();
  viewMenu->addAction(zoomInAct);
  viewMenu->addAction(zoomOutAct);
  viewMenu->addAction(zoomOriginalAct);
  viewMenu->addAction(zoomToFitAct);
  viewMenu->addAction(zoomToHeightAct);
  viewMenu->addAction(zoomToWidthAct);
  viewMenu->addAction(zoomToSelectionAct);
  viewMenu->addSeparator();
  viewMenu->addAction(drawBoxesAct);
}

void MainWindow::createActions()
{
  openAct = new QAction(QIcon(":/images/fileopen.png"), tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  saveAct = new QAction(QIcon(":/images/filesave.png"), tr("&Save"), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the document to disk"));
  saveAct->setEnabled(false);
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  saveAsAct = new QAction(QIcon(":/images/fileopenas.png"), tr("Save &As"), this);
  saveAsAct->setShortcut(tr("Ctrl+Shift+S"));
  saveAsAct->setStatusTip(tr("Save document after prompting the user for a file name."));
  saveAsAct->setEnabled(false);
  connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  closeAct = new QAction(QIcon(":/images/window-close.png"), tr("Cl&ose"), this);
  closeAct->setShortcut(QKeySequence::Close);
  closeAct->setStatusTip(tr("Close the active tab"));
  connect(closeAct, SIGNAL(triggered()), this, SLOT(closeActiveTab()));

  closeAllAct = new QAction(tr("Close &All"), this);
  closeAllAct->setShortcut(tr("Ctrl+Shift+W"));
  closeAllAct->setStatusTip(tr("Close all the tabs"));
  connect(closeAllAct, SIGNAL(triggered()), this, SLOT(closeAllTabs()));

  separatorAct = new QAction(this);
  separatorAct->setSeparator(true);

  exitAct = new QAction(QIcon(":/images/exit.png"), tr("E&xit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  boldAct = new QAction(QIcon(":/images/text_bold.png"), tr("&Bold"), this);
  boldAct->setShortcut(QKeySequence::Bold);
  boldAct->setCheckable(true);
  connect(boldAct, SIGNAL(triggered(bool)), this, SLOT(bold(bool)));

  italicAct = new QAction(QIcon(":/images/text_italic.png"), tr("&Italic"), this);
  italicAct->setShortcut(QKeySequence::Italic);
  italicAct->setCheckable(true);
  connect(italicAct, SIGNAL(triggered(bool)), this, SLOT(italic(bool)));

  underlineAct = new QAction(QIcon(":/images/text_under.png"), tr("&Underline"), this);
  underlineAct->setShortcut(QKeySequence::Underline);
  underlineAct->setCheckable(true);
  connect(underlineAct, SIGNAL(triggered(bool)), this, SLOT(underline(bool)));

  zoomInAct = new QAction(QIcon(":/images/zoom-in.png"), tr("Zoom &in"), this);
  zoomInAct->setShortcut(QKeySequence::ZoomIn);
  connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomOutAct = new QAction(QIcon(":/images/zoom-out.png"), tr("Zoom &out"), this);
  zoomOutAct->setShortcut(QKeySequence::ZoomOut);
  connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

  zoomOriginalAct = new QAction(QIcon(":/images/zoom-original.png"), tr("Zoom &1:1"), this);
  zoomOriginalAct->setShortcut(tr("Ctrl+*"));
  connect(zoomOriginalAct, SIGNAL(triggered()), this, SLOT(zoomOriginal()));

  zoomToFitAct = new QAction(QIcon(":/images/zoom-fit.png"), tr("Zoom to fit"), this);
  zoomToFitAct->setShortcut(tr("Ctrl+."));
  connect(zoomToFitAct, SIGNAL(triggered()), this, SLOT(zoomToFit()));

  zoomToHeightAct = new QAction(QIcon(":/images/zoom-height.png"), tr("Zoom to height"), this);
  zoomToHeightAct->setShortcut(tr("Ctrl+>"));
  connect(zoomToHeightAct, SIGNAL(triggered()), this, SLOT(zoomToHeight()));

  zoomToWidthAct = new QAction(QIcon(":/images/zoom-width.png"), tr("Zoom to width"), this);
  zoomToWidthAct->setShortcut(tr("Ctrl+<"));
  connect(zoomToWidthAct, SIGNAL(triggered()), this, SLOT(zoomToWidth()));

  zoomToSelectionAct = new QAction(QIcon(":/images/zoom-selection.png"), tr("Zoom to selection"), this);
  zoomToSelectionAct->setCheckable(true);
  zoomToSelectionAct->setShortcut(tr("Ctrl+/"));
  zoomToSelectionAct->setStatusTip(tr("Zoom to selected box"));
  connect(zoomToSelectionAct, SIGNAL(triggered()), this, SLOT(zoomToSelection()));

  drawBoxesAct = new QAction(QIcon(":/images/drawRect.png"), tr("S&how boxes"), this);
  drawBoxesAct->setCheckable(true);
  drawBoxesAct->setShortcut(tr("Ctrl+H"));
  drawBoxesAct->setStatusTip(tr("Show/hide rectangles for all boxes"));
  connect(drawBoxesAct, SIGNAL(triggered()), this, SLOT(drawBoxes()));

  nextAct = new QAction(QIcon(":/images/next.png"), tr("Ne&xt"), this);
  nextAct->setShortcuts(QKeySequence::NextChild);
  nextAct->setStatusTip(tr("Move the focus to the next window"));
  connect(nextAct, SIGNAL(triggered()), this, SLOT(nextTab()));

  previousAct = new QAction(QIcon(":/images/previous.png"), tr("Pre&vious"), this);
  previousAct->setShortcuts(QKeySequence::PreviousChild);
  previousAct->setStatusTip(tr("Move the focus to the previous window"));
  connect(previousAct, SIGNAL(triggered()), this, SLOT(previousTab()));

  insertAct = new QAction(tr("&Insert symbol"), this);
  insertAct->setShortcut(tr("Ctrl+0"));
  connect(insertAct, SIGNAL(triggered()), this, SLOT(insertSymbol()));

  splitAct = new QAction(tr("&Split symbol"), this);
  splitAct->setShortcut(tr("Ctrl+2"));
  connect(splitAct, SIGNAL(triggered()), this, SLOT(splitSymbol()));

  joinAct = new QAction(tr("&Join with Next Symbol"), this);
  joinAct->setShortcut(tr("Ctrl+1"));
  connect(joinAct, SIGNAL(triggered()), this, SLOT(joinSymbol()));

  deleteAct = new QAction(tr("&Delete symbol"), this);
  deleteAct->setShortcut(QKeySequence::Delete);
  connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteSymbol()));

  settingsAct = new QAction(tr("&Settings..."), this);
  settingsAct->setShortcut(tr("Ctrl+T"));
  settingsAct->setStatusTip(tr("Programm settings"));
  connect(settingsAct, SIGNAL(triggered()), this, SLOT(slotSettings()));

  checkForUpdateAct = new QAction(tr("&Check for update"), this);
  checkForUpdateAct->setStatusTip(tr("Check whether a newer version exits."));
  connect(checkForUpdateAct, SIGNAL(triggered()), this, SLOT(checkForUpdate()));

  aboutAct = new QAction(QIcon(":/images/help-about.png"), tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  connect(aboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
  for (int i = 0; i < MaxRecentFiles; ++i)
    {
      recentFileActs[i] = new QAction(this);
      recentFileActs[i]->setVisible(false);
      connect(recentFileActs[i], SIGNAL(triggered()),
              this, SLOT(openRecentFile()));
    }

  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);
  fileMenu->addAction(saveAsAct);
  fileMenu->addSeparator();
  fileMenu->addAction(closeAct);
  fileMenu->addAction(closeAllAct);
  fSeparatorAct = fileMenu->addSeparator();
  for (int i = 0; i < MaxRecentFiles; ++i)
    fileMenu->addAction(recentFileActs[i]);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);
  updateRecentFileActions();

  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(boldAct);
  editMenu->addAction(italicAct);
  editMenu->addAction(underlineAct);
  editMenu->addSeparator();
  editMenu->addAction(insertAct);
  editMenu->addAction(splitAct);
  editMenu->addAction(joinAct);
  editMenu->addAction(deleteAct);
  editMenu->addSeparator();
  editMenu->addAction(settingsAct);

  viewMenu = menuBar()->addMenu(tr("&View"));
  updateViewMenu();
  connect(viewMenu, SIGNAL(aboutToShow()), this, SLOT(updateViewMenu()));

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
#ifndef WINDOWS   // this does not work on Windows: TODO
  helpMenu->addAction(checkForUpdateAct);
#endif
  helpMenu->addSeparator();
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->setObjectName("fileToolBar");
  fileToolBar->addAction(exitAct);
  fileToolBar->addAction(openAct);
  fileToolBar->addAction(saveAct);
  fileToolBar->addAction(closeAct);

  viewToolBar = addToolBar(tr("View"));
  viewToolBar->setObjectName("viewToolBar");
  viewToolBar->addAction(previousAct);
  viewToolBar->addAction(nextAct);
  viewToolBar->addSeparator();
  viewToolBar->addAction(zoomInAct);
  viewToolBar->addAction(zoomOutAct);
  viewToolBar->addAction(zoomOriginalAct);
  viewToolBar->addAction(zoomToFitAct);
  viewToolBar->addAction(zoomToHeightAct);
  viewToolBar->addAction(zoomToWidthAct);
  viewToolBar->addAction(zoomToSelectionAct);
  viewToolBar->addSeparator();
  viewToolBar->addAction(drawBoxesAct);

  editToolBar = addToolBar(tr("Edit"));
  editToolBar->setObjectName("editToolBar");
  editToolBar->addAction(boldAct);
  editToolBar->addAction(italicAct);
  editToolBar->addAction(underlineAct);
}

void MainWindow::readSettings()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);

  settings.beginGroup("mainWindow");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("state").toByteArray());
  settings.endGroup();
}

void MainWindow::writeSettings()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);

  settings.beginGroup("mainWindow");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("state", saveState());
  settings.endGroup();
}
