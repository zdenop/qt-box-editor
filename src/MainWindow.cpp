/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <mk219533 (a) students.mimuw.edu.pl> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 *
 * MainWindow.cpp
 *
 *  Created on: 2010-01-04
 *      Author: Marcel Kolodziejczyk
 */

#include <QtGui>

#include "MainWindow.h"
#include "ChildWidget.h"

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
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (closeAllTabs()) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}

ChildWidget *MainWindow::activeChild()
{
  if (QWidget *currentWidget = tabWidget->currentWidget())
    return qobject_cast<ChildWidget *> (currentWidget);
  return 0;
}

void MainWindow::open()
{
  QString imageFile = QFileDialog::getOpenFileName(
      this,
      QString(),
      QString(),
      "TIFF Images (*.tif *.tiff)");
  addChild(imageFile);
}

void MainWindow::addChild(const QString &imageFileName)
{
  if (!imageFileName.isEmpty()) {
    QString canonicalImageFileName = QFileInfo(imageFileName).canonicalFilePath();
    for (int i = 0; i < tabWidget->count(); ++i) {
      ChildWidget *child = qobject_cast<ChildWidget *> (tabWidget->widget(i));
      if (canonicalImageFileName == child->canonicalImageFileName()) {
        tabWidget->setCurrentIndex(i);
        return;
      }
    }

    ChildWidget *child = new ChildWidget;
    if (child->loadImage(imageFileName)) {
      statusBar()->showMessage(tr("File loaded"), 2000);
      tabWidget->setCurrentIndex(tabWidget->addTab(child, child->userFriendlyCurrentFile()));
      connect(child, SIGNAL(boxChanged()), this, SLOT(updateCommandActions()));
      connect(child, SIGNAL(modifiedChanged()), this, SLOT(updateTabTitle()));
      connect(child, SIGNAL(modifiedChanged()), this, SLOT(updateSaveAction()));
    } else {
      child->close();
    }
  }
}

void MainWindow::save()
{
  if (activeChild() && activeChild()->save())
      statusBar()->showMessage(tr("File saved"), 2000);
}

bool MainWindow::closeActiveTab()
{
  if (tabWidget->currentWidget() && tabWidget->currentWidget()->close()) {
    tabWidget->removeTab(tabWidget->currentIndex());
    return true;
  }
  return false;
}

bool MainWindow::closeAllTabs()
{
  while (tabWidget->currentWidget()) {
    if (!closeActiveTab())
      return false;
  }
  return true;
}

void MainWindow::nextTab()
{
  if (tabWidget->count() > 0)
    tabWidget->setCurrentIndex((tabWidget->currentIndex() + 1) % tabWidget->count());
}

void MainWindow::previousTab()
{
  if (tabWidget->count() > 0) {
    tabWidget->setCurrentIndex((tabWidget->currentIndex() + tabWidget->count() - 1) % tabWidget->count());
  }
}

void MainWindow::bold(bool checked)
{
  if (activeChild()) {
    activeChild()->setBolded(checked);
  }
}

void MainWindow::italic(bool checked)
{
  if (activeChild()) {
    activeChild()->setItalic(checked);
  }
}

void MainWindow::underline(bool checked)
{
  if (activeChild()) {
    activeChild()->setUnderline(checked);
  }
}

void MainWindow::zoomIn()
{
  if (activeChild()) {
    activeChild()->zoomIn();
    statusBar()->showMessage(tr("Zoomed in"), 2000);
  }
}

void MainWindow::zoomOut()
{
  if (activeChild()) {
    activeChild()->zoomOut();
    statusBar()->showMessage(tr("Zoomed out"), 2000);
  }
}

void MainWindow::splitSymbol()
{
  if (activeChild()) {
    activeChild()->splitSymbol();
    statusBar()->showMessage(tr("Split symbol"), 2000);
  }
}

void MainWindow::joinSymbol()
{
  if (activeChild()) {
    activeChild()->joinSymbol();
    statusBar()->showMessage(tr("Join symbol"), 2000);
  }
}

void MainWindow::deleteSymbol()
{
  if (activeChild()) {
    activeChild()->deleteSymbol();
    statusBar()->showMessage(tr("Delete symbol"), 2000);
  }
}

void MainWindow::about()
{
   QMessageBox::about(
      this,
      tr("About"),
      tr(
          "tesseract-gui\n\ntesseract box files editor\n\nCopyright 2010 Marcel Kolodziejczyk\n\n\"THE BEER-WARE LICENSE\" (Revision 42):\n"
            "<mk219533 (a) students.mimuw.edu.pl> wrote this file. As long as you retain "
            "this notice you can do whatever you want with this stuff. If we meet some day, "
            "and you think this stuff is worth it, you can buy me a beer in return."));
}

void MainWindow::handleClose(int i)
{
  if (tabWidget->widget(i) && tabWidget->widget(i)->close())
    tabWidget->removeTab(i);
}

void MainWindow::updateMenus()
{
  closeAct->setEnabled(activeChild() != 0);
  closeAllAct->setEnabled(activeChild() != 0);
  nextAct->setEnabled(activeChild() != 0);
  previousAct->setEnabled(activeChild() != 0);
  separatorAct->setVisible(activeChild() != 0);
  zoomInAct->setEnabled(activeChild() != 0);
  zoomOutAct->setEnabled(activeChild() != 0);
}

void MainWindow::updateCommandActions()
{
  bool enable = (activeChild()) ? activeChild()->isBoxSelected() : false;
  boldAct->setEnabled(enable);
  boldAct->setChecked((activeChild()) ? activeChild()->isBold() : false);
  italicAct->setEnabled(enable);
  italicAct->setChecked((activeChild()) ? activeChild()->isItalic() : false);
  underlineAct->setEnabled(enable);
  underlineAct->setChecked((activeChild()) ? activeChild()->isUnderLine() : false);
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
  if (activeChild()) {
    QString title = activeChild()->userFriendlyCurrentFile();
    if (activeChild()->isModified())
      title += " *";
    tabWidget->setTabText(tabWidget->currentIndex(), title);
  }
}


void MainWindow::updateFileMenu()
{
  fileMenu->clear();
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);
  fileMenu->addSeparator();
  fileMenu->addAction(closeAct);
  fileMenu->addAction(closeAllAct);
  fileMenu->addAction(separatorAct);

  separatorAct->setVisible(tabWidget->count() > 0);

  for (int i = 0; i < tabWidget->count(); ++i) {
    ChildWidget *child = qobject_cast<ChildWidget *> (tabWidget->widget(i));

    QString text;
    if (i < 9) {
      text = tr("&%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
    } else {
      text = tr("%1 %2").arg(i + 1) .arg(child->userFriendlyCurrentFile());
    }
    QAction *action = fileMenu->addAction(text);
    action->setCheckable(true);
    action ->setChecked(child == activeChild());
    connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
    windowMapper->setMapping(action, i);
  }

  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

}

void MainWindow::createActions()
{
  openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the document to disk"));
  saveAct->setEnabled(false);
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  closeAct = new QAction(QIcon(":/images/close.png"), tr("Cl&ose"), this);
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

  boldAct = new QAction(QIcon(":/images/textbold.png"), tr("&Bold"), this);
  boldAct->setShortcut(QKeySequence::Bold);
  boldAct->setCheckable(true);
  connect(boldAct, SIGNAL(triggered(bool)), this, SLOT(bold(bool)));

  italicAct = new QAction(QIcon(":/images/textitalic.png"), tr("&Italic"), this);
  italicAct->setShortcut(QKeySequence::Italic);
  italicAct->setCheckable(true);
  connect(italicAct, SIGNAL(triggered(bool)), this, SLOT(italic(bool)));

  underlineAct = new QAction(QIcon(":/images/textunder.png"), tr("&Underine"), this);
  underlineAct->setShortcut(QKeySequence::Underline);
  underlineAct->setCheckable(true);
  connect(underlineAct, SIGNAL(triggered(bool)), this, SLOT(underline(bool)));

  zoomInAct = new QAction(QIcon(":/images/zoomin.png"), tr("Zoom &in"), this);
  zoomInAct->setShortcut(QKeySequence::ZoomIn);
  connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomOutAct = new QAction(QIcon(":/images/zoomout.png"), tr("Zoom &out"), this);
  zoomOutAct->setShortcut(QKeySequence::ZoomOut);
  connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

  nextAct = new QAction(QIcon(":/images/next.png"), tr("Ne&xt"), this);
  nextAct->setShortcuts(QKeySequence::NextChild);
  nextAct->setStatusTip(tr("Move the focus to the next window"));
  connect(nextAct, SIGNAL(triggered()), this, SLOT(nextTab()));

  previousAct = new QAction(QIcon(":/images/previous.png"), tr("Pre&vious"), this);
  previousAct->setShortcuts(QKeySequence::PreviousChild);
  previousAct->setStatusTip(tr("Move the focus to the previous window"));
  connect(previousAct, SIGNAL(triggered()), this, SLOT(previousTab()));

  splitAct = new QAction(tr("&Split symbol"), this);
  splitAct->setShortcut(tr("Ctrl+2"));
  connect(splitAct, SIGNAL(triggered()), this, SLOT(splitSymbol()));

  joinAct = new QAction(tr("&Join with Next Symbol"), this);
  joinAct->setShortcut(tr("Ctrl+1"));
  connect(joinAct, SIGNAL(triggered()), this, SLOT(joinSymbol()));

  deleteAct = new QAction(tr("&Delete symbol"), this);
  deleteAct->setShortcut(QKeySequence::Delete);
  connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteSymbol()));

  aboutAct = new QAction(QIcon(":/images/about.png"), tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  updateFileMenu();
  connect(fileMenu, SIGNAL(aboutToShow()), this, SLOT(updateFileMenu()));

  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(boldAct);
  editMenu->addAction(italicAct);
  editMenu->addAction(underlineAct);
  editMenu->addSeparator();
  editMenu->addAction(splitAct);
  editMenu->addAction(joinAct);
  editMenu->addAction(deleteAct);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(nextAct);
  viewMenu->addAction(previousAct);
  viewMenu->addSeparator();
  viewMenu->addAction(zoomInAct);
  viewMenu->addAction(zoomOutAct);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars()
{
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->addAction(openAct);
  fileToolBar->addAction(saveAct);
  fileToolBar->addAction(closeAct);

  viewToolBar = addToolBar(tr("View"));
  viewToolBar->addAction(previousAct);
  viewToolBar->addAction(nextAct);
  viewToolBar->addAction(zoomInAct);
  viewToolBar->addAction(zoomOutAct);

  editToolBar = addToolBar(tr("Edit"));
  editToolBar->addAction(boldAct);
  editToolBar->addAction(italicAct);
  editToolBar->addAction(underlineAct);
}

void MainWindow::readSettings()
{
  QSettings settings("", "tesseract-gui");
  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("size", QSize(400, 400)).toSize();
  move(pos);
  resize(size);
}

void MainWindow::writeSettings()
{
  QSettings settings("", "tesseract-gui");
  settings.setValue("pos", pos());
  settings.setValue("size", size());
}
