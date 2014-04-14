/**********************************************************************
* File:        MainWindow.cpp
* Description: MainWindow functions
* Author:      Marcel Kolodziejczyk
* Created:     2010-01-04
*
* (C) Copyright 2010, Marcel Kolodziejczyk
* (C) Copyright 2011-2014, Zdenko Podobny
* (C) Copyright 2012, Zohar Gofer
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
#include "dialogs/ShortCutsDialog.h"

MainWindow::MainWindow() {
  tabWidget = new QTabWidget;

#if QT_VERSION >= 0x040500
  tabWidget->setTabsClosable(true);
  tabWidget->setMovable(true);
#endif

  connect(tabWidget, SIGNAL(tabCloseRequested(int)), this,
          SLOT(handleClose(int)));
  connect(tabWidget, SIGNAL(currentChanged(int)), this,
          SLOT(updateMenus()));
  connect(tabWidget, SIGNAL(currentChanged(int)), this,
          SLOT(updateCommandActions()));
  connect(tabWidget, SIGNAL(currentChanged(int)), this,
          SLOT(updateSaveAction()));

  setCentralWidget(tabWidget);

  windowMapper = new QSignalMapper(this);
  connect(windowMapper, SIGNAL(mapped(int)), tabWidget,
          SLOT(setCurrentIndex(int)));

  shortCutsDialog = 0;
  setAcceptDrops(true);
  tabWidget->setAcceptDrops(true);
  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  updateMenus();
  updateCommandActions();
  updateSaveAction();
  readSettings();
  setUnifiedTitleAndToolBarOnMac(true);
  setWindowIcon(QIcon(":/icons/qbe.png"));
  setWindowTitle(tr("%1 - v%2").arg(SETTING_APPLICATION).arg(VERSION));
}

void MainWindow::closeEvent(QCloseEvent* event) {
  if (closeAllTabs()) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}

ChildWidget* MainWindow::activeChild() {
  if (QWidget* currentWidget = tabWidget->currentWidget())
    return qobject_cast<ChildWidget*> (currentWidget);
  return 0;
}

void MainWindow::open() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  // try to get path from settings
  QString last_path = settings.value("last_path").toString();
  QString filetype = "Image files (*.bmp *.png *.jpeg *.jpg *.tif *.tiff);;";
  filetype += "Tiff files (*.tif *.tiff);;All files (*.*)";

  QString imageFile = QFileDialog::getOpenFileName(
                        this,
                        tr("Select image file..."),
                        last_path,
                        filetype);

  addChild(imageFile);
}

void MainWindow::addChild(const QString& imageFileName) {
  if (!imageFileName.isEmpty()) {
    QString canonicalImageFileName =
      QFileInfo(imageFileName).canonicalFilePath();
    for (int i = 0; i < tabWidget->count(); ++i) {
      ChildWidget* child = qobject_cast<ChildWidget*> (tabWidget->widget(i));
      if (canonicalImageFileName == child->canonicalImageFileName()) {
        tabWidget->setCurrentIndex(i);
        return;
      }
    }

    ChildWidget* child = new ChildWidget(this);
    if (child->loadImage(imageFileName)) {
      statusBar()->showMessage(tr("File loaded"), 2000);
      tabWidget->setCurrentIndex(tabWidget->addTab(child,
                                 child->userFriendlyCurrentFile()));
      connect(child, SIGNAL(boxChanged()), this, SLOT(updateCommandActions()));
      connect(child, SIGNAL(modifiedChanged()), this, SLOT(updateTabTitle()));
      connect(child, SIGNAL(modifiedChanged()), this, SLOT(updateSaveAction()));
      connect(child, SIGNAL(zoomRatioChanged(qreal)), this,
              SLOT(zoomRatioChanged(qreal)));
      connect(child, SIGNAL(statusBarMessage(QString)), this,
              SLOT(statusBarMessage(QString)));
      child->setZoomStatus();
      // save path of open image file
      QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                         SETTING_ORGANIZATION, SETTING_APPLICATION);
      QString filePath = QFileInfo(imageFileName).absolutePath();
      settings.setValue("last_path", filePath);

      QStringList files = settings.value("recentFileList").toStringList();
      files.removeAll(imageFileName);
      files.prepend(imageFileName);
      while (files.size() > MaxRecentFiles)
        files.removeLast();

      settings.setValue("recentFileList", files);

      foreach(QWidget * widget, QApplication::topLevelWidgets()) {
        MainWindow* mainWin = qobject_cast<MainWindow*>(widget);
        if (mainWin)
          mainWin->updateRecentFileActions();
      }
    } else {
      child->close();
    }
  }
}

void MainWindow::updateRecentFileActions() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  QStringList files = settings.value("recentFileList").toStringList();

  int numRecentFiles = qMin(files.size(), static_cast<int>(MaxRecentFiles));

  for (int i = 0; i < numRecentFiles; ++i) {
    QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());

    recentFileActs[i]->setText(text);
    recentFileActs[i]->setData(files[i]);
    recentFileActs[i]->setVisible(true);
  }
  for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    recentFileActs[j]->setVisible(false);

  fSeparatorAct->setVisible(numRecentFiles > 0);
}

void MainWindow::save() {
  QString fileName = activeChild()->currentBoxFile();

  if (activeChild() && activeChild()->save(fileName))
    statusBar()->showMessage(tr("File saved"), 2000);
}

/**
 * @brief MainWindow::splitToFeatureBF
 * create infividual box file per feature e.g.:
 * eng.timesnormal.exp001.box
 * eng.timesbold.exp001.box
 * eng.timesitalic.exp001.box
 * eng.timesbolditalic.exp001.box
 * eng.timesunderline.exp001.box
 * if input filename is eng.times.exp001.box
 */
void MainWindow::splitToFeatureBF() {
  QString fileName = activeChild()->currentBoxFile();

  if (activeChild() && activeChild()->splitToFeatureBF(fileName))
    statusBar()->showMessage(tr("File saved"), 2000);
}

/**
 * @brief MainWindow::saveAs
 * Make a copy but do not update title of tab etc.
 * because there is not correcponding image file
 */
void MainWindow::saveAs() {
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

void MainWindow::reLoad() {
  QString currentFileName = activeChild()->currentBoxFile();
  if (QFile::exists(currentFileName)) {
      switch (QMessageBox::question(
                this,
                tr("Warning: Request to reload file!"),
                  tr("Do you want to reload file '%1' from disk?\n\n" \
                     "Warning: This operation can not be undone!")
                .arg(currentFileName),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No)) {
    case QMessageBox::Yes: {
          if (activeChild() && activeChild()->reload(currentFileName))
            statusBar()->showMessage(tr("Box file reloaded"), 2000);
          break;
      }
      case QMessageBox::No:
      case QMessageBox::Cancel:
      default:
        break;
      }
    } else {
      QMessageBox::question(this, tr("File was not find."),
                            tr("File '%1' was not find.").arg(currentFileName));
  }
}

void MainWindow::reLoadImg() {
  if (activeChild() && activeChild()->reloadImg())
            statusBar()->showMessage(tr("Image reloaded"), 2000);
}

void MainWindow::importPLSym() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  // try to get path from settings
  QString last_path = settings.value("last_path").toString();

  QString textFile = QFileDialog::getOpenFileName(
                       this,
                       tr("Select text file..."),
                       last_path,
                       tr("Text files (*.txt);;All files (*.*)"));
  if (textFile.isEmpty())
    return;

  if (activeChild() && activeChild()->importSPLToChild(textFile))
    statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::importTextSym() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  // try to get path from settings
  QString last_path = settings.value("last_path").toString();

  QString textFile = QFileDialog::getOpenFileName(
                       this,
                       tr("Select text file..."),
                       last_path,
                       tr("Text files (*.txt);;All files (*.*)"));
  if (textFile.isEmpty())
    return;

  if (activeChild() && activeChild()->importTextToChild(textFile))
    statusBar()->showMessage(tr("File saved"), 2000);
}

// TODO(zdenop): simplified/join code symbolPerLine(), rowPerLine(),
//               paragraphPerLine()
void MainWindow::symbolPerLine() {
  QString currentFileName = activeChild()->currentBoxFile().replace(
                              ".box", ".txt");
  QString fileName = QFileDialog::getSaveFileName(this,
                     tr("Export symbols to file..."),
                     currentFileName,
                     tr("Text files (*.txt);;All files (*)"));

  if (fileName.isEmpty())
    return;

  if (activeChild() && activeChild()->exportTxt(1, fileName))
    statusBar()->showMessage(tr("Data exported"), 2000);
}

void MainWindow::rowPerLine() {
  QString currentFileName = activeChild()->currentBoxFile().replace(
                              ".box", ".txt");
  QString fileName = QFileDialog::getSaveFileName(this,
                     tr("Export symbols to file..."),
                     currentFileName,
                     tr("Text files (*.txt);;All files (*)"));

  if (fileName.isEmpty())
    return;

  if (activeChild() && activeChild()->exportTxt(2, fileName))
    statusBar()->showMessage(tr("Data exported"), 2000);
}

void MainWindow::paragraphPerLine() {
  QString currentFileName = activeChild()->currentBoxFile().replace(
                              ".box", ".txt");
  QString fileName = QFileDialog::getSaveFileName(this,
                     tr("Export symbols to file..."),
                     currentFileName,
                     tr("Text files (*.txt);;All files (*)"));

  if (fileName.isEmpty())
    return;

  if (activeChild() && activeChild()->exportTxt(3, fileName))
    statusBar()->showMessage(tr("Data exported"), 2000);
}
bool MainWindow::closeActiveTab() {
  if (tabWidget->currentWidget() && tabWidget->currentWidget()->close()) {
    tabWidget->removeTab(tabWidget->currentIndex());
    return true;
  }
  return false;
}

bool MainWindow::closeAllTabs() {
  while (tabWidget->currentWidget()) {
    if (!closeActiveTab())
      return false;
  }
  return true;
}

void MainWindow::openRecentFile() {
  QAction* action = qobject_cast<QAction*>(sender());

  if (action)
    addChild(action->data().toString());
}

void MainWindow::nextTab() {
  if (tabWidget->count() > 0)
    tabWidget->setCurrentIndex((tabWidget->currentIndex() + 1)
                               % tabWidget->count());
}

void MainWindow::previousTab() {
  if (tabWidget->count() > 0) {
    tabWidget->setCurrentIndex((tabWidget->currentIndex() + tabWidget->count()
                                - 1) % tabWidget->count());
  }
}

void MainWindow::bold(bool checked) {
  if (activeChild()) {
    activeChild()->setBolded(checked);
  }
}

void MainWindow::italic(bool checked) {
  if (activeChild()) {
    activeChild()->setItalic(checked);
  }
}

void MainWindow::underline(bool checked) {
  if (activeChild()) {
    activeChild()->setUnderline(checked);
  }
}

void MainWindow::zoomOriginal() {
  if (activeChild()) {
    activeChild()->zoomOriginal();
  }
}

void MainWindow::zoomToSelection() {
  if (activeChild()) {
    activeChild()->zoomToSelection();
  }
}

void MainWindow::zoomIn() {
  if (activeChild()) {
    activeChild()->zoomIn();
  }
}

void MainWindow::zoomOut() {
  if (activeChild()) {
    activeChild()->zoomOut();
  }
}

void MainWindow::zoomToFit() {
  if (activeChild()) {
    activeChild()->zoomToFit();
  }
}

void MainWindow::zoomToHeight() {
  if (activeChild()) {
    activeChild()->zoomToHeight();
  }
}

void MainWindow::zoomToWidth() {
  if (activeChild()) {
    activeChild()->zoomToWidth();
  }
}

void MainWindow::showSymbol() {
  if (activeChild()) {
    activeChild()->showSymbol();
  }
}

void MainWindow::directTypingMode(bool checked) {
  if (activeChild()) {
    activeChild()->setDirectTypingMode(checked);
  }
}

void MainWindow::showFontColumns(bool checked) {
  if (activeChild()) {
    activeChild()->setShowFontColumns(checked);
  }
}

void MainWindow::drawBoxes() {
  if (activeChild()) {
    activeChild()->drawBoxes();
  }
}

void MainWindow::insertSymbol() {
  if (activeChild()) {
    activeChild()->insertSymbol();
  }
}

void MainWindow::splitSymbol() {
  if (activeChild()) {
    activeChild()->splitSymbol();
  }
}

void MainWindow::joinSymbol() {
  if (activeChild()) {
    activeChild()->joinSymbol();
  }
}

void MainWindow::deleteSymbol() {
  if (activeChild()) {
    activeChild()->deleteSymbol();
  }
}

void MainWindow::moveUp() {
  if (activeChild()) {
    activeChild()->moveUp();
  }
}

void MainWindow::moveDown() {
  if (activeChild()) {
    activeChild()->moveDown();
  }
}

void MainWindow::moveTo() {
  if (activeChild()) {
    activeChild()->moveTo();
  }
}

void MainWindow::goToRow() {
  if (activeChild()) {
    activeChild()->goToRow();
  }
}

void MainWindow::find() {
  if (activeChild()) {
    activeChild()->find();
  }
}

void MainWindow::drawRect(bool checked) {
  if (activeChild()) {
    activeChild()->drawRectangle(checked);
  }
}

void MainWindow::undo() {
  if (activeChild()) {
    activeChild()->undo();
  }
}

void MainWindow::redo() {
  if (activeChild()) {
    activeChild()->redo();
  }
}

void MainWindow::reReadSetting() {
  for (int i = 0; i < tabWidget->count(); ++i) {
    ChildWidget* child = qobject_cast<ChildWidget*> (tabWidget->widget(i));
    child->readSettings();
  }
}

void MainWindow::slotSettings() {
  runSettingsDialog = new SettingsDialog(this);
  connect(runSettingsDialog, SIGNAL(settingsChanged()), this, SLOT(reReadSetting()));
  runSettingsDialog->exec();
}

void MainWindow::genBoxFile() {
    QString currentFileName = activeChild()->currentBoxFile();
    switch (QMessageBox::question(
                    this,
                    tr("Warning: Request to renerage box file!"),
                      tr("Do you want to regenerate current page in '%1'?\n"
                         "This will replace boxfile stored on disk.\n\n" \
                         "Warning: This operation can not be undone!")
                    .arg(currentFileName),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No)) {
        case QMessageBox::Yes: {
              if (activeChild() && activeChild()->qCreateBoxes(currentFileName))
                statusBar()->showMessage(tr("Boxfile regenerated"), 2000);
              break;
          }
          case QMessageBox::No:
          case QMessageBox::Cancel:
          default:
            break;
          }
}

void MainWindow::getBinImage() {
    if (activeChild()) {
      activeChild()->binarizeImage();
    }
}

void MainWindow::checkForUpdate() {
  statusBar()->showMessage(tr("Checking for new version..."), 2000);

  QNetworkRequest request;
  request.setUrl(QUrl(UPDATE_URL));

  // TODO(zdenop): test for proxy, ask auth.
  QNetworkAccessManager* manager = new QNetworkAccessManager();
  QNetworkReply* reply = manager->get(request);

  QEventLoop* loop = new QEventLoop;

  QObject::connect(manager, SIGNAL(finished(QNetworkReply*)),
                   loop, SLOT(quit()));
  loop->exec();

  checkVersion(reply);
  delete manager;
}

void MainWindow::requestFinished(QNetworkReply* reply) {
  checkVersion(reply);
}

void MainWindow::checkVersion(QNetworkReply* reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QString current_version = reply->readAll().trimmed();
    QString app_version = QString("%1").arg(VERSION).replace("dev", "",
                                                           Qt::CaseInsensitive);
    QString messageText;

    if (app_version == current_version) {
      messageText = tr("<p>No newer version is available.</p>");
    } else if (app_version > current_version) {
      messageText = tr("<p>Your version ('%1') is higher than ").arg(VERSION);
      messageText += tr("released stable version ('%2').").arg(current_version);
      messageText += tr("</p><p>Do you use develepment version? ");
      messageText += tr("Don't forget to install stable version manually!</p>");
    } else {
      messageText = tr("<p>New version '%1' is available!<br/>Please visit ")
                    .arg(current_version);
      messageText +=
        tr("<a href=%1/downloads>downloads on project homepage!</a></p>")
        .arg(PROJECT_URL);
    }

    QMessageBox::information(this, tr("Version info"), messageText);
  } else {
    QMessageBox::critical(this, tr("Network"),
                          tr("ERROR: %1").arg(reply->errorString()));
  }
}

void MainWindow::shortCutList() {
  if (!shortCutsDialog)
    shortCutsDialog = new ShortCutsDialog(this);
  shortCutsDialog -> show();
}

void MainWindow::about() {
  QString abouttext =
    tr("<h1>%1 %3</h1>").arg(SETTING_APPLICATION).arg(VERSION);

  abouttext.append(tr("<p><a href=\"http://qt-project.org/\">QT</a> "));
  abouttext.append(tr("editor of tesseract-ocr box files</p>"));
  abouttext.append(tr("<p>Project page: <a href=%1>%2</a></p>").
                   arg(PROJECT_URL).arg(PROJECT_URL_NAME));
  abouttext.append(tr("<p>Copyright 2010 Marcel Kolodziejczyk<br/>"));
  abouttext.append(tr("Copyright 2012 Zohar Gofer<br/>"));
  abouttext.append(tr("Copyright 2012 Dmitri Silaev<br/>"));
  abouttext.append(tr("Copyright 2011-2014 Zdenko Podobný</p>"));
  abouttext.append(tr("<p>This software is released under "
                      "<a href=\"http://www.apache.org/licenses/LICENSE-2.0\"" \
                      ">Apache License 2.0</a></p>"));
  QMessageBox::about(this, tr("About application"), abouttext);
}

void MainWindow::aboutQt() {
  QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::handleClose(int i) {
  if (tabWidget->widget(i) && tabWidget->widget(i)->close())
    tabWidget->removeTab(i);

  if (!activeChild()) {
    _utfCodeLabel->setText("");
    _boxsize->setText("");
    _zoom->setText("");
  }
  updateMenus();
}

void MainWindow::updateMenus() {
  saveAsAct->setEnabled((activeChild()) != 0);
  reLoadAct->setEnabled((activeChild()) != 0);
  reLoadImgAct->setEnabled((activeChild()) != 0);
  genBoxAct->setEnabled((activeChild()) != 0);
  getBinAct->setEnabled((activeChild()) != 0);
  splitToFeatureBFAct->setEnabled((activeChild()) != 0);
  importPLSymAct->setEnabled((activeChild()) != 0);
  importTextSymAct->setEnabled((activeChild()) != 0);
  symbolPerLineAct->setEnabled((activeChild()) != 0);
  rowPerLineAct->setEnabled((activeChild()) != 0);
  paragraphPerLineAct->setEnabled((activeChild()) != 0);
  closeAct->setEnabled(activeChild() != 0);
  closeAllAct->setEnabled(activeChild() != 0);
  nextAct->setEnabled(tabWidget->count() > 1);
  previousAct->setEnabled(tabWidget->count() > 1);
  separatorAct->setVisible(activeChild() != 0);
  zoomOriginalAct->setEnabled(activeChild() != 0);
  zoomInAct->setEnabled(activeChild() != 0);
  zoomOutAct->setEnabled(activeChild() != 0);
  zoomToFitAct->setEnabled(activeChild() != 0);
  zoomToHeightAct->setEnabled(activeChild() != 0);
  zoomToWidthAct->setEnabled(activeChild() != 0);
  zoomToSelectionAct->setEnabled(activeChild() != 0);
  showSymbolAct->setEnabled(activeChild() != 0);
  goToRowAct->setEnabled(activeChild() != 0);
  findAct->setEnabled(activeChild() != 0);
  undoAct->setEnabled(activeChild() != 0);
  redoAct->setEnabled(activeChild() != 0);
  drawRectAct->setEnabled(activeChild() != 0);
  drawBoxesAct->setEnabled(activeChild() != 0);
  DirectTypingAct->setEnabled(activeChild() != 0);
  showFontColumnsAct->setEnabled(activeChild() != 0);
}

void MainWindow::updateCommandActions() {
  bool enable = (activeChild()) ? activeChild()->isBoxSelected() : false;

  undoAct->setEnabled((activeChild())
                      ? activeChild()->isUndoAvailable() : false);
  redoAct->setEnabled((activeChild())
                      ? activeChild()->isRedoAvailable() : false);
  boldAct->setEnabled(enable);
  boldAct->setChecked((activeChild()) ? activeChild()->isBold() : false);
  italicAct->setEnabled(enable);
  italicAct->setChecked((activeChild()) ? activeChild()->isItalic() : false);
  underlineAct->setEnabled(enable);
  underlineAct->setChecked((activeChild())
                           ? activeChild()->isUnderLine() : false);
  showSymbolAct->setChecked((activeChild())
                            ? activeChild()->isShowSymbol() : false);
  drawBoxesAct->setChecked((activeChild())
                           ? activeChild()->isDrawBoxes() : false);
  drawRectAct->setChecked((activeChild())
                          ? activeChild()->isDrawRect() : false);
  DirectTypingAct->setChecked((activeChild())
                              ? activeChild()->isDirectTypingMode() : false);
  showFontColumnsAct->setChecked((activeChild())
                                 ? activeChild()->isFontColumnsShown() : false);
  moveUpAct->setEnabled(enable);
  moveDownAct->setEnabled(enable);
  moveToAct->setEnabled(enable);
  insertAct->setEnabled(enable);
  splitAct->setEnabled(enable);
  joinAct->setEnabled(enable);
  deleteAct->setEnabled(enable);

  if (activeChild()) {
    _utfCodeLabel->setText(activeChild()->getSymbolHexCode());
    _boxsize->setText(activeChild()->getBoxSize());
    activeChild()->setZoomStatus();
  }
}

void MainWindow::updateSaveAction() {
  saveAct->setEnabled((activeChild()) ? activeChild()->isModified() : false);
}

void MainWindow::updateTabTitle() {
  if (activeChild()) {
    QString title = activeChild()->userFriendlyCurrentFile();
    if (activeChild()->isModified())
      title += " *";
    tabWidget->setTabText(tabWidget->currentIndex(), title);
  }
}

void MainWindow::updateViewMenu() {
  viewMenu->clear();
  viewMenu->addAction(nextAct);
  viewMenu->addAction(previousAct);
  viewMenu->addAction(separatorAct);

  separatorAct->setVisible(tabWidget->count() > 0);

  for (int i = 0; i < tabWidget->count(); ++i) {
    ChildWidget* child = qobject_cast<ChildWidget*> (tabWidget->widget(i));

    QString text;
    if (i < 9) {
      text = tr("&%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
    } else {
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
  viewMenu->addAction(showSymbolAct);
  viewMenu->addAction(showFontColumnsAct);
  viewMenu->addAction(drawBoxesAct);
}

void MainWindow::createActions() {
  openAct = new QAction(QIcon::fromTheme("fileopen"),
                        tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setToolTip(tr("Open an existing file"));
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  saveAct = new QAction(QIcon::fromTheme("filesave"),
                        tr("&Save"), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setToolTip(tr("Save the document to disk"));
  saveAct->setStatusTip(tr("Save the document to disk"));
  saveAct->setEnabled(false);
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  splitToFeatureBFAct = new QAction(tr("&Split to boxfiles"), this);
  splitToFeatureBFAct->setToolTip(
    tr("Create individual boxfile for regular, bold, italic and " \
       "underline boxes."));
  splitToFeatureBFAct->setStatusTip(
              tr("Create individual boxfile for regular, bold, italic and " \
                 "underline boxes."));
  splitToFeatureBFAct->setEnabled(false);
  connect(splitToFeatureBFAct, SIGNAL(triggered()), this,
          SLOT(splitToFeatureBF()));

  saveAsAct = new QAction(QIcon::fromTheme("filesaveas"),
                          tr("Save &As"), this);
  saveAsAct->setShortcut(tr("Ctrl+Shift+S"));
  saveAsAct->setToolTip(
    tr("Save document after prompting the user for a file name."));
  saveAsAct->setStatusTip(
              tr("Save document after prompting the user for a file name."));
  saveAsAct->setEnabled(false);
  connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  reLoadAct = new QAction(tr("Reload boxfile"), this);
  reLoadAct->setShortcut(tr("Ctrl+Alt+R"));
  reLoadAct->setToolTip(tr("Reload file from disk."));
  reLoadAct->setStatusTip(tr("Reload file from disk."));
  reLoadAct->setEnabled(false);
  connect(reLoadAct, SIGNAL(triggered()), this, SLOT(reLoad()));

  reLoadImgAct = new QAction(tr("Reload image"), this);
  reLoadImgAct->setShortcut(QKeySequence::Refresh);
  reLoadImgAct->setToolTip(tr("Reload image file from disk."));
  reLoadImgAct->setStatusTip(tr("Reload image file from disk."));
  reLoadImgAct->setEnabled(false);
  connect(reLoadImgAct, SIGNAL(triggered()), this, SLOT(reLoadImg()));

  importPLSymAct = new QAction(
    tr("I&mport file with one symbol per line"), this);
  importPLSymAct->setToolTip(tr("Import symbols from text document"));
  importPLSymAct->setStatusTip(tr("Import symbols from text document"));
  importPLSymAct->setEnabled(false);
  connect(importPLSymAct, SIGNAL(triggered()), this, SLOT(importPLSym()));

  importTextSymAct = new QAction(QIcon::fromTheme("fileimport"),
                                 tr("Import &text file"), this);
  importTextSymAct->setToolTip(tr("Import symbols from text document"));
  importTextSymAct->setStatusTip(tr("Import symbols from text document"));
  importTextSymAct->setEnabled(false);
  connect(importTextSymAct, SIGNAL(triggered()), this, SLOT(importTextSym()));

  // TODO(zdenop): implementation based on parameter?
  symbolPerLineAct = new QAction(tr("Symbol per line…"), this);
  symbolPerLineAct->setToolTip(tr("Export symbols to text file."));
  symbolPerLineAct->setStatusTip(tr("Export symbols to text file."));
  symbolPerLineAct->setEnabled(false);
  connect(symbolPerLineAct, SIGNAL(triggered()), this, SLOT(symbolPerLine()));

  rowPerLineAct = new QAction(tr("Line by line…"), this);
  rowPerLineAct->setToolTip(tr("Export symbols to text file."));
  rowPerLineAct->setStatusTip(tr("Export symbols to text file."));
  rowPerLineAct->setEnabled(false);
  connect(rowPerLineAct, SIGNAL(triggered()), this, SLOT(rowPerLine()));

  paragraphPerLineAct = new QAction(tr("Paragraph per line…"), this);
  paragraphPerLineAct->setToolTip(tr("Export symbols to text file."));
  paragraphPerLineAct->setStatusTip(tr("Export symbols to text file."));
  paragraphPerLineAct->setEnabled(false);
  connect(paragraphPerLineAct, SIGNAL(triggered()), this,
          SLOT(paragraphPerLine()));

  closeAct = new QAction(QIcon::fromTheme("window-close"),
                         tr("Cl&ose"), this);
  closeAct->setShortcut(QKeySequence::Close);
  closeAct->setToolTip(tr("Close the active tab"));
  closeAct->setStatusTip(tr("Close the active tab"));
  connect(closeAct, SIGNAL(triggered()), this, SLOT(closeActiveTab()));

  closeAllAct = new QAction(tr("Close &All"), this);
  closeAllAct->setShortcut(tr("Ctrl+Shift+W"));
  closeAllAct->setToolTip(tr("Close all the tabs"));
  closeAllAct->setStatusTip(tr("Close all the tabs"));
  connect(closeAllAct, SIGNAL(triggered()), this, SLOT(closeAllTabs()));

  separatorAct = new QAction(this);
  separatorAct->setSeparator(true);

  exitAct = new QAction(QIcon::fromTheme("exit"),
                        tr("E&xit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  exitAct->setToolTip(tr("Exit the application"));
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  boldAct = new QAction(QIcon::fromTheme("text_bold"),
                        tr("&Bold"), this);
  boldAct->setShortcut(QKeySequence::Bold);
  boldAct->setCheckable(true);
  connect(boldAct, SIGNAL(triggered(bool)), this, SLOT(bold(bool)));

  italicAct = new QAction(QIcon::fromTheme("text_italic"),
                          tr("&Italic"), this);
  italicAct->setShortcut(QKeySequence::Italic);
  italicAct->setCheckable(true);
  connect(italicAct, SIGNAL(triggered(bool)), this, SLOT(italic(bool)));

  underlineAct = new QAction(QIcon::fromTheme("text_under"),
                             tr("&Underline"), this);
  underlineAct->setShortcut(QKeySequence::Underline);
  underlineAct->setCheckable(true);
  connect(underlineAct, SIGNAL(triggered(bool)), this, SLOT(underline(bool)));

  zoomInAct = new QAction(QIcon::fromTheme("zoom-in"),
                          tr("Zoom &in"), this);
  zoomInAct->setShortcut(QKeySequence::ZoomIn);
  connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomOutAct = new QAction(QIcon::fromTheme("zoom-out"),
                           tr("Zoom &out"), this);
  zoomOutAct->setShortcut(QKeySequence::ZoomOut);
  connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

  zoomOriginalAct = new QAction(QIcon::fromTheme("zoom-original"),
                                tr("Zoom &1:1"), this);
  zoomOriginalAct->setShortcut(tr("Ctrl+*"));
  connect(zoomOriginalAct, SIGNAL(triggered()), this, SLOT(zoomOriginal()));

  zoomToFitAct = new QAction(QIcon::fromTheme("zoom-fit"),
                             tr("Zoom to fit"), this);
  zoomToFitAct->setShortcut(tr("Ctrl+."));
  connect(zoomToFitAct, SIGNAL(triggered()), this, SLOT(zoomToFit()));

  zoomToHeightAct = new QAction(QIcon::fromTheme("zoom-height"),
                                tr("Zoom to height"), this);
  zoomToHeightAct->setShortcut(tr("Ctrl+>"));
  connect(zoomToHeightAct, SIGNAL(triggered()), this, SLOT(zoomToHeight()));

  zoomToWidthAct = new QAction(QIcon::fromTheme("zoom-width"),
                               tr("Zoom to width"), this);
  zoomToWidthAct->setShortcut(tr("Ctrl+<"));
  connect(zoomToWidthAct, SIGNAL(triggered()), this, SLOT(zoomToWidth()));

  zoomToSelectionAct = new QAction(QIcon::fromTheme("zoom-selection"),
                                   tr("Zoom to selected box"), this);
  zoomToSelectionAct->setShortcut(tr("Ctrl+/"));
  connect(zoomToSelectionAct, SIGNAL(triggered()), this,
          SLOT(zoomToSelection()));

  showSymbolAct = new QAction(QIcon::fromTheme("showSymbol"),
                              tr("S&how symbol"), this);
  showSymbolAct->setCheckable(true);
  showSymbolAct->setShortcut(tr("Ctrl+L"));
  showSymbolAct->setToolTip(tr("Show/hide symbol over selection rectangle"));
  showSymbolAct->setStatusTip(tr("Show/hide symbol over selection rectangle"));
  connect(showSymbolAct, SIGNAL(triggered()), this, SLOT(showSymbol()));

  DirectTypingAct = new QAction(QIcon::fromTheme("key_bindings"),
                                tr("&Direct type mode"), this);
  DirectTypingAct->setCheckable(true);
  DirectTypingAct->setShortcut(tr("Ctrl+D"));
  connect(DirectTypingAct, SIGNAL(triggered(bool)), this,
          SLOT(directTypingMode(bool)));

  showFontColumnsAct = new QAction(QIcon::fromTheme("fonts"),
                                   tr("Show Font Columns"), this);
  showFontColumnsAct->setCheckable(true);
  connect(showFontColumnsAct, SIGNAL(triggered(bool)), this,
          SLOT(showFontColumns(bool)));

  drawBoxesAct = new QAction(QIcon::fromTheme("drawRect"),
                             tr("S&how boxes"), this);
  drawBoxesAct->setCheckable(true);
  drawBoxesAct->setShortcut(tr("Ctrl+H"));
  drawBoxesAct->setToolTip(tr("Show/hide rectangles for all boxes"));
  drawBoxesAct->setStatusTip(tr("Show/hide rectangles for all boxes"));
  connect(drawBoxesAct, SIGNAL(triggered()), this, SLOT(drawBoxes()));

  nextAct = new QAction(QIcon::fromTheme("next"), tr("Ne&xt"), this);
  nextAct->setShortcuts(QKeySequence::NextChild);
  nextAct->setToolTip(tr("Move the focus to the next window"));
  nextAct->setStatusTip(tr("Move the focus to the next window"));
  connect(nextAct, SIGNAL(triggered()), this, SLOT(nextTab()));

  previousAct = new QAction(QIcon::fromTheme("previous"),
                            tr("Pre&vious"), this);
  previousAct->setShortcuts(QKeySequence::PreviousChild);
  previousAct->setToolTip(tr("Move the focus to the previous window"));
  previousAct->setStatusTip(tr("Move the focus to the previous window"));
  connect(previousAct, SIGNAL(triggered()), this, SLOT(previousTab()));

  insertAct = new QAction(QIcon::fromTheme("insertRow"),
                          tr("&Insert symbol"), this);
  insertAct->setShortcut(Qt::Key_Insert);
  connect(insertAct, SIGNAL(triggered()), this, SLOT(insertSymbol()));

  splitAct = new QAction(QIcon::fromTheme("splitRow"),
                         tr("&Split symbol"), this);
  splitAct->setShortcut(tr("Ctrl+2"));
  connect(splitAct, SIGNAL(triggered()), this, SLOT(splitSymbol()));

  joinAct = new QAction(QIcon::fromTheme("joinRow"),
                        tr("&Join with Next Symbol"), this);
  joinAct->setShortcut(tr("Ctrl+1"));
  connect(joinAct, SIGNAL(triggered()), this, SLOT(joinSymbol()));

  deleteAct = new QAction(QIcon::fromTheme("deleteRow"),
                          tr("&Delete symbol"), this);
  deleteAct->setShortcut(QKeySequence::Delete);
  connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteSymbol()));

  moveUpAct = new QAction(QIcon::fromTheme("up"),
                          tr("Move row &up"), this);
  moveUpAct->setShortcut(Qt::ALT | Qt::Key_Up);
  connect(moveUpAct, SIGNAL(triggered()), this, SLOT(moveUp()));

  moveDownAct = new QAction(QIcon::fromTheme("down"),
                            tr("Move row &down"), this);
  moveDownAct->setShortcut(Qt::ALT | Qt::Key_Down);
  connect(moveDownAct, SIGNAL(triggered()), this, SLOT(moveDown()));

  moveToAct = new QAction(QIcon::fromTheme("moveTo"),
                          tr("&Move row to…"), this);
  moveToAct->setShortcut(Qt::CTRL | Qt::Key_M);
  connect(moveToAct, SIGNAL(triggered()), this, SLOT(moveTo()));

  goToRowAct = new QAction(QIcon::fromTheme("jump"),
                           tr("&Go to row…"), this);
  goToRowAct->setShortcut(tr("Ctrl+G"));
  connect(goToRowAct, SIGNAL(triggered()), this, SLOT(goToRow()));

  findAct = new QAction(QIcon::fromTheme("find"),
                        tr("&Find…"), this);
  findAct->setShortcut(tr("Ctrl+F"));
  connect(findAct, SIGNAL(triggered()), this, SLOT(find()));

  drawRectAct = new QAction(QIcon::fromTheme("rectangle"),
                            tr("Draw/Hide &Rectangle…"), this);
  drawRectAct->setCheckable(true);
  drawRectAct->setShortcut(tr("Ctrl+R"));
  connect(drawRectAct, SIGNAL(triggered(bool)), this, SLOT(drawRect(bool)));

  undoAct = new QAction(QIcon::fromTheme("undo"),
                        tr("&Undo"), this);
  undoAct->setShortcut(tr("Ctrl+Z"));
  connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

  redoAct = new QAction(QIcon::fromTheme("redo"),
                        tr("&Redo"), this);
  redoAct->setShortcut(tr("Ctrl+Y"));
  connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

  settingsAct = new QAction(QIcon::fromTheme("settings"),
                            tr("&Settings..."), this);
  settingsAct->setShortcut(tr("Ctrl+T"));
  settingsAct->setToolTip(tr("Programm settings"));
  settingsAct->setStatusTip(tr("Programm settings"));
  connect(settingsAct, SIGNAL(triggered()), this, SLOT(slotSettings()));

  genBoxAct = new QAction(tr("Generate boxes for page"), this);
  genBoxAct->setShortcut(tr("Ctrl+Alt+G"));
  genBoxAct->setToolTip(tr("Re-generate boxes for current page."));
  genBoxAct->setStatusTip(tr("Re-generate boxes for current page."));
  connect(genBoxAct, SIGNAL(triggered()), this, SLOT(genBoxFile()));

  getBinAct = new QAction(tr("Convert to binary"), this);
  getBinAct->setToolTip(tr("Convert current image page to binary - used for " \
                           "tesseract-ocr training."));
  getBinAct->setStatusTip(tr("Convert current image page to binary - used for " \
                           "tesseract-ocr training."));
  connect(getBinAct, SIGNAL(triggered()), this, SLOT(getBinImage()));

  checkForUpdateAct = new QAction(tr("&Check for update"), this);
  checkForUpdateAct->setToolTip(tr("Check whether a newer version exits."));
  checkForUpdateAct->setStatusTip(tr("Check whether a newer version exits."));
  connect(checkForUpdateAct, SIGNAL(triggered()), this, SLOT(checkForUpdate()));

  aboutAct = new QAction(QIcon::fromTheme("help-about"),
                         tr("&About"), this);
  aboutAct->setToolTip(tr("Show the application's About box"));
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  connect(aboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));

  shortCutListAct = new QAction(tr("&Shortcut List"), this);
  shortCutListAct -> setShortcut(tr("F1"));
  connect(shortCutListAct, SIGNAL(triggered()), this, SLOT(shortCutList()));
}

void MainWindow::createMenus() {
  for (int i = 0; i < MaxRecentFiles; ++i) {
    recentFileActs[i] = new QAction(this);
    recentFileActs[i]->setVisible(false);
    connect(recentFileActs[i], SIGNAL(triggered()),
            this, SLOT(openRecentFile()));
  }

  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);
  fileMenu->addAction(saveAsAct);
  fileMenu->addAction(reLoadAct);
  fileMenu->addAction(reLoadImgAct);
  fileMenu->addSeparator();
  fileMenu->addAction(splitToFeatureBFAct);
  importMenu = fileMenu->addMenu(tr("&Import..."));
  importMenu->addAction(importPLSymAct);
  importMenu->addAction(importTextSymAct);
  exportMenu = fileMenu->addMenu(tr("&Export..."));
  exportMenu->addAction(symbolPerLineAct);
  exportMenu->addAction(rowPerLineAct);
  exportMenu->addAction(paragraphPerLineAct);
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
  editMenu->addAction(undoAct);
  editMenu->addAction(redoAct);
  editMenu->addSeparator();
  editMenu->addAction(boldAct);
  editMenu->addAction(italicAct);
  editMenu->addAction(underlineAct);
  editMenu->addSeparator();
  editMenu->addAction(insertAct);
  editMenu->addAction(joinAct);
  editMenu->addAction(splitAct);
  editMenu->addAction(deleteAct);
  editMenu->addSeparator();
  editMenu->addAction(moveUpAct);
  editMenu->addAction(moveDownAct);
  editMenu->addAction(moveToAct);
  editMenu->addAction(goToRowAct);
  editMenu->addAction(findAct);
  editMenu->addSeparator();
  editMenu->addAction(DirectTypingAct);
  editMenu->addAction(drawRectAct);
  editMenu->addSeparator();
  editMenu->addAction(settingsAct);

  viewMenu = menuBar()->addMenu(tr("&View"));
  updateViewMenu();
  connect(viewMenu, SIGNAL(aboutToShow()), this, SLOT(updateViewMenu()));

  tessMenu = menuBar()->addMenu(tr("&Tesseract"));
  tessMenu->addAction(genBoxAct);
  tessMenu->addAction(getBinAct);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(checkForUpdateAct);
  helpMenu->addSeparator();
  helpMenu->addAction(shortCutListAct);
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars() {
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->setObjectName("fileToolBar");
  fileToolBar->addAction(exitAct);
  fileToolBar->addAction(openAct);
  fileToolBar->addAction(saveAct);
  fileToolBar->addAction(importTextSymAct);

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
  viewToolBar->addAction(showSymbolAct);
  viewToolBar->addAction(drawBoxesAct);
  viewToolBar->addAction(DirectTypingAct);
  viewToolBar->addAction(showFontColumnsAct);
  viewToolBar->addAction(drawRectAct);

  editToolBar = addToolBar(tr("Edit"));
  editToolBar->setObjectName("editToolBar");
  editToolBar->addAction(undoAct);
  editToolBar->addAction(redoAct);
  editToolBar->addSeparator();
  editToolBar->addAction(boldAct);
  editToolBar->addAction(italicAct);
  editToolBar->addAction(underlineAct);
  editToolBar->addSeparator();
  editToolBar->addAction(findAct);
}

void MainWindow::createStatusBar() {
  _utfCodeLabel = new QLabel();
  _utfCodeLabel->setToolTip(QString("UTF-8 codes of symbols"));
  _utfCodeLabel->setText("");
  _utfCodeLabel->setIndent(5);

  _boxsize = new QLabel();
  _boxsize->setToolTip(QString("Width&Height of box"));
  _boxsize->setFrameStyle(QFrame::Sunken);
  _boxsize->setAlignment(Qt::AlignHCenter);
  _boxsize->setMaximumWidth(60);

  _zoom = new QLabel();
  _zoom->setToolTip(QString("Zoom factor"));
  _zoom->setFrameStyle(QFrame::Sunken);
  _zoom->setAlignment(Qt::AlignHCenter);
  _zoom->setMaximumWidth(50);

  statusBar()->addWidget(_utfCodeLabel, 3);
  statusBar()->addWidget(_boxsize, 1);
  statusBar()->addWidget(_zoom, 1);
}

void MainWindow::readSettings() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);

  settings.beginGroup("mainWindow");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("state").toByteArray());
  settings.endGroup();
}

void MainWindow::writeSettings() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);

  settings.beginGroup("mainWindow");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("state", saveState());
  settings.endGroup();
}

void MainWindow::zoomRatioChanged(qreal ratio) {
  _zoom->setText(QString("%1%").arg(qRound(ratio * 100)));
}

void MainWindow::statusBarMessage(QString message) {
  statusBar()->showMessage(message, 2000);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void MainWindow::dropEvent(QDropEvent *event) {
  QList<QUrl> urls = event->mimeData()->urls();
  if (urls.count()) {
    QString fname = urls[0].toLocalFile();
    addChild(fname);
    event->acceptProposedAction();
  }
}
