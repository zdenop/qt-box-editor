/**********************************************************************
* File:        ChildWidget.cpp
* Description: ChildWidget functions
* Author:      Marcel Kolodziejczyk
* Created:     2010-01-06
*
* (C) Copyright 2010, Marcel Kolodziejczyk
* (C) Copyright 2011-2012, Zdenko Podobny
* (C) Copyright 2012, Zohar Gofer (Undo action)
* (C) Copyright 2012, Dmitri Silaev (Hall text effect)
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

#include <string>
#include <algorithm>

#include "include/ChildWidget.h"
#include "include/Settings.h"
#include "include/SettingsDialog.h"
#include "dialogs/GetRowIDDialog.h"
#include "dialogs/FindDialog.h"
#include "dialogs/DrawRectangle.h"
#include "include/DelegateEditors.h"
#include "include/TessTools.h"

int my_min(int arg1, int arg2) {
  return((arg1 < arg2) ? arg1 : arg2);
}

int my_max(int arg1, int arg2) {
  return((arg1 > arg2) ? arg1 : arg2);
}

ChildWidget::ChildWidget(QWidget* parent)
  : QSplitter(Qt::Horizontal, parent) {
  model = new QStandardItemModel(0, 9, this);
  //  model->setHeaderData(-1, Qt::Horizontal, tr("Row"));
  model->setHeaderData(0, Qt::Horizontal, tr("Letter"));
  model->setHeaderData(1, Qt::Horizontal, tr("Left"));
  model->setHeaderData(2, Qt::Horizontal, tr("Bottom"));
  model->setHeaderData(3, Qt::Horizontal, tr("Right"));
  model->setHeaderData(4, Qt::Horizontal, tr("Top"));
  model->setHeaderData(5, Qt::Horizontal, tr("Page"));
  model->setHeaderData(6, Qt::Horizontal, tr("Italic"));
  model->setHeaderData(7, Qt::Horizontal, tr("Bold"));
  model->setHeaderData(8, Qt::Horizontal, tr("Underline"));

  table = new QTableView;
  table->setModel(model);
  table->resize(1, 1);
  table->setAlternatingRowColors(true);
  selectionModel = new QItemSelectionModel(model);
  connect(
    selectionModel,
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this, SLOT(emitBoxChanged()));
  table->setSelectionModel(selectionModel);
  table->setSelectionBehavior(QAbstractItemView::SelectItems);
  table->setSelectionMode(QAbstractItemView::ExtendedSelection);
  table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

  table->hideColumn(5);
  table->hideColumn(6);
  table->hideColumn(7);
  table->hideColumn(8);
  table->installEventFilter(this);  // installs event filter

  SpinBoxDelegate* sbDelegate = new SpinBoxDelegate;
  // TODO(zdenop): setMaximum for delegates after changing box
  table->setItemDelegateForColumn(1, sbDelegate);
  table->setItemDelegateForColumn(2, sbDelegate);
  table->setItemDelegateForColumn(3, sbDelegate);
  table->setItemDelegateForColumn(4, sbDelegate);
  connect(sbDelegate, SIGNAL(sbd_valueChanged(int)), this,
          SLOT(sbValueChanged(int)));
  connect(sbDelegate, SIGNAL(sbd_editingFinished()), this,
          SLOT(drawSelectionRects()));

  CheckboxDelegate* cbDelegate = new CheckboxDelegate;
  table->setItemDelegateForColumn(6, cbDelegate);
  table->setItemDelegateForColumn(7, cbDelegate);
  table->setItemDelegateForColumn(8, cbDelegate);
  connect(cbDelegate, SIGNAL(toggled(bool)), this,
          SLOT(cbFontToggleProxy(bool)));

  // Font for table
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  QFont tableFont = settings.value("GUI/Font").value<QFont>();

  if (tableFont.family().isEmpty()) {
    tableFont.setFamily(TABLE_FONT);
    tableFont.setPointSize(TABLE_FONT_SIZE);
  }
  table->setFont(tableFont);

  if (settings.contains("GUI/Rectagle")) {
    rectColor = settings.value("GUI/Rectagle").value<QColor>();
  } else {
    rectColor = Qt::red;
  }

  if (settings.contains("GUI/Rectagle_fill")) {
    rectFillColor = settings.value("GUI/Rectagle_fill").value<QColor>();
  } else {
    rectFillColor = Qt::red;
    rectFillColor.setAlpha(127);
  }

  if (settings.contains("GUI/Box")) {
    boxColor = settings.value("GUI/Box").value<QColor>();
  } else {
    boxColor = Qt::green;
  }

  if (settings.contains("GUI/BackgroundColor")) {
    backgroundColor = settings.value("GUI/BackgroundColor").value<QColor>();
  } else {
    backgroundColor = (Qt::gray);
  }

  // Make graphics Scene and View
  imageScene = new QGraphicsScene;
  imageView = new QGraphicsView(imageScene);
  imageView->setRenderHints(QPainter::Antialiasing |
                            QPainter::SmoothPixmapTransform);
  imageView->setAttribute(Qt::WA_TranslucentBackground, true);
  imageView->setAutoFillBackground(true);
  imageView->setBackgroundBrush(backgroundColor);

  // Table toolbar
  QPushButton* upButton = new QPushButton();
  upButton->setIcon(QIcon(":/images/up.svg"));
  upButton->setToolTip(tr("Move row up"));
  upButton->setMinimumSize(QSize(24, 24));
  upButton->setMaximumSize(QSize(24, 24));
  connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));

  QPushButton* downButton = new QPushButton();
  downButton->setIcon(QIcon(":/images/down.svg"));
  downButton->setToolTip(tr("Move row down"));
  downButton->setMinimumSize(QSize(24, 24));
  downButton->setMaximumSize(QSize(24, 24));
  connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

  QPushButton* moveToButton = new QPushButton();
  moveToButton->setIcon(QIcon(":/images/moveTo.svg"));
  moveToButton->setToolTip(tr("Move row to position…"));
  moveToButton->setMinimumSize(QSize(24, 24));
  moveToButton->setMaximumSize(QSize(24, 24));
  connect(moveToButton, SIGNAL(clicked()), this, SLOT(moveTo()));

  QPushButton* goToButton = new QPushButton();
  goToButton->setIcon(QIcon(":/images/gtk-jump-to-ltr.png"));
  goToButton->setToolTip(tr("Go to row…"));
  goToButton->setMinimumSize(QSize(24, 24));
  goToButton->setMaximumSize(QSize(24, 24));
  connect(goToButton, SIGNAL(clicked()), this, SLOT(goToRow()));

  QPushButton* insertButton = new QPushButton();
  insertButton->setIcon(QIcon(":/images/insertRow.svg"));
  insertButton->setToolTip(tr("Insert symbol"));
  insertButton->setMaximumSize(QSize(24, 24));
  connect(insertButton, SIGNAL(clicked()), this, SLOT(insertSymbol()));

  QPushButton* joinButton = new QPushButton();
  joinButton->setIcon(QIcon(":/images/joinRow.svg"));
  joinButton->setToolTip(tr("Join symbols"));
  joinButton->setMaximumSize(QSize(24, 24));
  connect(joinButton, SIGNAL(clicked()), this, SLOT(joinSymbol()));

  QPushButton* splitButton = new QPushButton();
  splitButton->setIcon(QIcon(":/images/splitRow.svg"));
  splitButton->setToolTip(tr("Split symbol"));
  splitButton->setMaximumSize(QSize(24, 24));
  connect(splitButton, SIGNAL(clicked()), this, SLOT(splitSymbol()));

  QPushButton* removeButton = new QPushButton();
  removeButton->setIcon(QIcon(":/images/deleteRow.png"));
  removeButton->setToolTip(tr("Remove symbol"));
  removeButton->setMaximumSize(QSize(24, 24));
  connect(removeButton, SIGNAL(clicked()), this, SLOT(deleteSymbol()));

  QHBoxLayout* movementLayout = new QHBoxLayout();
  movementLayout->setSpacing(2);
  movementLayout->addWidget(upButton);
  movementLayout->addWidget(downButton);
  movementLayout->addWidget(moveToButton);
  movementLayout->addWidget(goToButton);

  QHBoxLayout* actionLayout = new QHBoxLayout();
  actionLayout->setSpacing(2);
  actionLayout->addWidget(insertButton);
  actionLayout->addWidget(joinButton);
  actionLayout->addWidget(splitButton);
  actionLayout->addWidget(removeButton);

  QSpacerItem* horizontalSpacer = new QSpacerItem(73, 20,
      QSizePolicy::Expanding,
      QSizePolicy::Minimum);

  QGridLayout* gridLayout = new QGridLayout();
  gridLayout->setContentsMargins(3, 3, 3, 3);
  gridLayout->addLayout(movementLayout, 0, 0, 1, 1);
  gridLayout->addItem(horizontalSpacer, 0, 1, 1, 1);
  gridLayout->addLayout(actionLayout, 0, 2, 1, 1);

  QWidget* tableWidget = new QWidget(this);
  QVBoxLayout* verticalLayout = new QVBoxLayout(tableWidget);
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  verticalLayout->addWidget(table);
  verticalLayout->addLayout(gridLayout);

  // splitter
  addWidget(tableWidget);
  addWidget(imageView);
  setStretchFactor(indexOf(table), 0);
  setStretchFactor(indexOf(imageView), 1);
  connect(this, SIGNAL(splitterMoved(int,int)), this, SLOT(updateColWidthsOnSplitter(int,int)));

  setSelectionRect();
  widgetWidth = parent->size().width();
  modified = false;
  boxesVisible = false;
  drawnRectangle = false;
  symbolShown = true;
  directTypingMode = false;
  f_dialog = 0;
  m_DrawRectangle = 0;
  rectangle = 0;

  rubberBand = new QRubberBand(QRubberBand::Rectangle, imageView);
}

void ChildWidget::updateColWidthsOnSplitter(int /*pos*/, int /*index*/) {
  table->horizontalHeader()->resizeSections(QHeaderView::Stretch);
}

bool ChildWidget::loadImage(const QString& fileName) {
  QImage image(fileName);

  if (image.isNull()) {
    QMessageBox::information(this, tr("Wrong file"),
                             tr("Cannot load %1.").arg(fileName));
    return false;
  }
  imageHeight = image.height();
  imageWidth = image.width();
  setCurrentImageFile(fileName);
  QString boxFileName = QFileInfo(fileName).path() + QDir::separator()
                        + QFileInfo(fileName).completeBaseName() + ".box";

  if (!QFile::exists(boxFileName)) {
    if (!qCreateBoxes(boxFileName, image)) return false;
  } else {
    if (!loadBoxes(boxFileName)) return false;
  }

  setCurrentBoxFile(boxFileName);
  imageItem = imageScene->addPixmap(QPixmap::fromImage(image));
  modified = false;
  emit modifiedChanged();
  connect(model, SIGNAL(itemChanged(QStandardItem*)), this,
          SLOT(emitBoxChanged()));
  connect(model, SIGNAL(itemChanged(QStandardItem*)), this,
          SLOT(documentWasModified()));
  return true;
}

bool ChildWidget::qCreateBoxes(const QString &boxFileName, const QImage& img) {
  switch (QMessageBox::question(
            this,
            tr("Missing file"),
            tr("Cannot load image, because there is no corresponding box "\
               "file.\nCreate new?"),
            QMessageBox::Yes |
            QMessageBox::No |
            QMessageBox::Cancel,
            QMessageBox::Cancel)) {
  case QMessageBox::Yes: {
    TessTools tt;
    QString str = tt.makeBoxes(img);
    if (str == "")
      return false;
    QTextStream boxdata(&str);
    if (!fillTableData(boxdata))
      return false;
    save(boxFileName);
    break;
  }
  case QMessageBox::No:
  case QMessageBox::Cancel:
  default:
    return false;
  }
  return true;
}

bool ChildWidget::fillTableData(QTextStream &boxdata) {
  boxdata.setCodec("UTF-8");
  QString line;
  int row = 0;
  int firstPage = -1;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  do {
    line = boxdata.readLine();
    if (!line.isEmpty()) {
      QFont letterFont;
      QStringList pieces = line.split(" ", QString::SkipEmptyParts);
      QString letter = pieces.value(0);
      bool bold = false, italic = false, underline = false;
      // formating is present only in case there are more than 2 letters
      if (letter.at(0) == '@' && letter.size() > 1) {
        bold = true;
        letterFont.setBold(true);
        letter.remove(0, 1);
      }
      if (letter.at(0) == '$' && letter.size() > 1) {
        italic = true;
        letterFont.setItalic(true);
        letter.remove(0, 1);
      }
      if (letter.at(0) == '\'' && letter.size() > 1) {
        underline = true;
        letterFont.setUnderline(true);
        letter.remove(0, 1);
      }
      int left = pieces.value(1).toInt();
      int bottom = imageHeight - pieces.value(2).toInt();
      int right = pieces.value(3).toInt();
      int top = imageHeight - pieces.value(4).toInt();
      int page = pieces.value(5).toInt();

      // TODO(zdenop): implement support for multipage tif
      if (firstPage < 0)
        firstPage = page;  // first page can have number 5 ;-)
      if (firstPage == page) {  // ignore other pages than first page
        model->insertRow(row);
        model->setData(model->index(row, 0, QModelIndex()), letterFont,
                       Qt::FontRole);
        model->setData(model->index(row, 0, QModelIndex()), letter);
        model->setData(model->index(row, 1, QModelIndex()), left);
        model->setData(model->index(row, 2, QModelIndex()), bottom);
        model->setData(model->index(row, 3, QModelIndex()), right);
        model->setData(model->index(row, 4, QModelIndex()), top);
        model->setData(model->index(row, 5, QModelIndex()), page);
        model->setData(model->index(row, 6, QModelIndex()), italic);
        model->setData(model->index(row, 7, QModelIndex()), bold);
        model->setData(model->index(row, 8, QModelIndex()), underline);
        row++;
      }
    }
  } while (!line.isEmpty());

  table->resizeRowsToContents();
  table->setCornerButtonEnabled(true);
  table->setWordWrap(true);

  // set optimum size of table
  int tableVisibleWidth = 0;
  tableVisibleWidth += table->verticalHeader()->sizeHint().width();

  for (int col = 0; col < table->horizontalHeader()->count(); col++) {
    if (table->columnWidth(col) > 0)
      tableVisibleWidth += table->columnWidth(col) + 1;  // 1 px for table grid
  }
  // scrollbar
  tableVisibleWidth += table->verticalScrollBar()->sizeHint().width();
  tableVisibleWidth += table->frameWidth()*2;

  QList<int> splitterSizes;
  splitterSizes << tableVisibleWidth;
  splitterSizes << widgetWidth - tableVisibleWidth - this->handleWidth();

  setSizes(splitterSizes);

  QApplication::restoreOverrideCursor();
  return true;
}

bool ChildWidget::loadBoxes(const QString& fileName) {
  QFile file(fileName);

  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, SETTING_APPLICATION,
                         tr("Cannot read file %1:\n%2.").arg(fileName).arg(
                           file.errorString()));
    return false;
  }

  QTextStream in(&file);
  fillTableData(in);
  file.close();
  return true;
}

bool ChildWidget::save(const QString& fileName) {
  QFile file(fileName);

  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(
      this,
      SETTING_APPLICATION,
      tr("Cannot write file %1:\n%2.").arg(boxFile).arg(file.errorString()));
    return false;
  }

  QTextStream out(&file);
  out.setCodec("UTF-8");
  QApplication::setOverrideCursor(Qt::WaitCursor);

  for (int row = 0; row < model->rowCount(); ++row) {
    QString letter = model->index(row, 0).data().toString();
    int left = model->index(row, 1).data().toInt();
    int bottom = model->index(row, 2).data().toInt();
    int right = model->index(row, 3).data().toInt();
    int top = model->index(row, 4).data().toInt();
    int page = model->index(row, 5).data().toInt();
    bool italic = model->index(row, 6).data().toBool();
    bool bold = model->index(row, 7).data().toBool();
    bool underline = model->index(row, 8).data().toBool();
    if (bold)
      out << "@";
    if (italic)
      out << "$";
    if (underline)
      out << "\'";
    out << letter << " " << left << " " << imageHeight - bottom << " "
        << right << " " << imageHeight - top << " " << page << "\n";
  }

  QApplication::restoreOverrideCursor();

  modified = false;
  emit modifiedChanged();

  return true;
}

bool ChildWidget::splitToFeatureBF(const QString& fileName) {
  // Todo(zdenop): there must be a smarter way how to do this

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString normBoxes = "", boldBoxes = "", italicBoxes = "", boldItaBoxes = "";
  QString underBoxes = "";
  for (int row = 0; row < model->rowCount(); ++row) {
    QString letter = model->index(row, 0).data().toString();
    int left = model->index(row, 1).data().toInt();
    int bottom = model->index(row, 2).data().toInt();
    int right = model->index(row, 3).data().toInt();
    int top = model->index(row, 4).data().toInt();
    int page = model->index(row, 5).data().toInt();
    bool italic = model->index(row, 6).data().toBool();
    bool bold = model->index(row, 7).data().toBool();
    bool underline = model->index(row, 8).data().toBool();

    if (bold && !italic) {
      boldBoxes += QString("%1 %2 %3 %4 %5 %6\n").arg(letter).arg(left)
                   .arg(imageHeight - bottom).arg(right).arg(imageHeight - top)
                   .arg(page);
    } else if (italic && !bold) {
      italicBoxes += QString("%1 %2 %3 %4 %5 %6\n").arg(letter).arg(left)
                     .arg(imageHeight - bottom).arg(right)
                     .arg(imageHeight - top).arg(page);
    } else if (italic && bold) {
      boldItaBoxes += QString("%1 %2 %3 %4 %5 %6\n").arg(letter).arg(left)
                      .arg(imageHeight - bottom).arg(right)
                      .arg(imageHeight - top).arg(page);
    } else if (underline) {
      underBoxes += QString("%1 %2 %3 %4 %5 %6\n").arg(letter).arg(left)
                    .arg(imageHeight - bottom).arg(right).arg(imageHeight - top)
                    .arg(page);
    } else {
      normBoxes += QString("%1 %2 %3 %4 %5 %6\n").arg(letter).arg(left)
                   .arg(imageHeight - bottom).arg(right).arg(imageHeight - top)
                   .arg(page);
    }
  }

  // find path + name + ext:
  int dotCount = QFileInfo(fileName).fileName().count(".");
  QStringList results = QFileInfo(fileName).fileName().split(".");
  QString path, base, ext;
  path = QFileInfo(fileName).path() + QDir::separator();

  if (dotCount < 3) {
    base = QFileInfo(fileName).baseName();
    ext = QFileInfo(fileName).completeSuffix();
  } else  {
    for (int dot = 0; dot < (dotCount - 1); ++dot) {
      base += results[dot];
      if (dot < (dotCount - 2))
        base += ".";
    }
    ext = results[(dotCount - 1)] + "." + results[dotCount];
  }

  if (normBoxes.size()) {
    saveString(path + base + "normal." + ext, normBoxes);
  }

  if (boldBoxes.size()) {
    saveString(path + base + "bold." + ext, boldBoxes);
  }

  if (italicBoxes.size()) {
    saveString(path + base + "italic." + ext, italicBoxes);
  }

  if (boldItaBoxes.size()) {
    saveString(path + base + "bolditalic." + ext, boldItaBoxes);
  }

  if (underBoxes.size()) {
    saveString(path + base + "underline." + ext, underBoxes);
  }

  QApplication::restoreOverrideCursor();
  return true;
}

bool ChildWidget::saveString(const QString& fileName, const QString& qData) {
  QFile file(fileName);

  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(
      this,
      SETTING_APPLICATION,
      tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
    return false;
  }

  QTextStream out(&file);
  out.setCodec("UTF-8");
  out << qData;
  file.close();
  return true;
}

bool ChildWidget::importSPLToChild(const QString& fileName) {
  QFile file(fileName);

  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, SETTING_APPLICATION,
                         tr("Cannot read file %1:\n%2.").arg(fileName).arg(
                           file.errorString()));
    return false;
  }

  QTextStream in(&file);
  in.setCodec("UTF-8");
  // TODO(zdenop): ask for format of input file
  // format_1: 1 line = 1 symbol
  // format_2: 1 letter = 1 symbol

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString line;
  int row = 0;
  do {
    line = in.readLine();
    if (!line.isEmpty()) {
      if (row > model->rowCount()) {
        QMessageBox::warning(this, SETTING_APPLICATION,
                             tr("There are more symbols in import file than " \
                                "boxes!\nRest of symbols are ignored."));
        file.close();
        QApplication::restoreOverrideCursor();
        return true;
      }
      model->setData(model->index(row, 0, QModelIndex()), line);
      row++;
    }
  } while (!line.isEmpty());

  if (row < model->rowCount()) {
    QMessageBox::warning(this, SETTING_APPLICATION,
                         tr("There are less symbols in import file than boxes!")
                         .arg(fileName).arg(file.errorString()));
  }

  file.close();
  QApplication::restoreOverrideCursor();

  modified = true;
  emit modifiedChanged();

  return true;
}


bool ChildWidget::importTextToChild(const QString& fileName) {
  // TODO(zdenop): code clean up, and join with importSPLToChild

  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);

  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, SETTING_APPLICATION,
                         tr("Cannot read file %1:\n%2.").arg(fileName)
                         .arg(file.errorString()));
    return false;
  }

  if (settings.contains("Text/OpenDialog") != true ||
      (settings.value("Text/OpenDialog").toBool())) {
    SettingsDialog *runSettingsDialog = new SettingsDialog(this, 1);
    runSettingsDialog->exec();
  }

  QList<QString> ligatures = settings.value("Text/Ligatures").toString()
                             .split("\n");

  QTextStream in(&file);
  in.setCodec("UTF-8");

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString line;
  QString symbolText;

  do {
    line = in.readLine();
    if (!line.isEmpty()) {
      symbolText += line;
    }
  } while (!line.isEmpty());
  file.close();

  // check if there are ligatures in line
  for (int i = 0; i < ligatures.size(); ++i) {
    symbolText.replace(ligatures[i], QString("\n%1\n").arg(ligatures[i]));
  }

  QStringList lines;
  QString oneLetter, letters;
  bool ligature;
  QList<QString> symbols;

  lines = symbolText.split("\n");  // split string to lines
  for (int i = 0; i < lines.size(); ++i) {
    letters = lines.at(i);
    ligature = false;
    for (int m = 0; m < ligatures.size(); ++m) {
      if (ligatures.at(m) == letters) {  // this is ligature
        symbols.append(letters);
        ligature = true;
      }
    }
    if (ligature == false) {
      letters = letters.remove(QRegExp("\\s+"));  // remove whitespace
      for (int j = 0; j < letters.size(); ++j) {  // split lines to letters
        oneLetter = letters[j];
        symbols.append(oneLetter);
      }
    }
  }
  if (symbols.size() != model->rowCount()) {
    QMessageBox::warning(this, SETTING_APPLICATION,
                         tr("Number of symbols in import file differ with " \
                            "number of boxes!"));
  }

  for (int i = 0; i < symbols.size(); ++i) {
    model->setData(model->index(i, 0, QModelIndex()), symbols.at(i));
  }

  QApplication::restoreOverrideCursor();

  modified = true;
  emit modifiedChanged();

  return true;
}

/**
   * Export symbols from table to text file. eType identify export format:
   * 1 - one symbol per line
   * 2 - one row per line
   * 3 - one paragraph per line
   * Returns false if export was not possible for some reason.
   *
   * Export will work only on one column text correctly.
   * Words are identified if space between boxes is bigger than 6.
   * Lines are identified if space between boxes is bigger than -6.
   * Paragraph is identified based on left indentation (15) of from last left
   * margin.
*/
bool ChildWidget::exportTxt(const int& eType, const QString& fileName) {
  QFile file(fileName);

  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(
      this,
      SETTING_APPLICATION,
      tr("Cannot write file %1:\n%2.").arg(boxFile).arg(file.errorString()));
    return false;
  }

  QTextStream out(&file);
  out.setCodec("UTF-8");
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString letter;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  int left, bottom, right, top;
  int line_start_prev = 0;
  int line_end_prev = 0;
  int right_prev = -1;
  int last_bottom = -1;
  int wordSpace = settings.value("Text/WordSpace").toInt();
  int paraIndent = settings.value("Text/ParagraphIndent").toInt();

  for (int row = 0; row < model->rowCount(); ++row) {
    letter = model->index(row, 0).data().toString();
    left = model->index(row, 1).data().toInt();
    bottom = model->index(row, 2).data().toInt();
    right = model->index(row, 3).data().toInt();
    top = model->index(row, 4).data().toInt();

    if (last_bottom == -1)
      last_bottom = top;

    if (eType == 1 && (right_prev != -1)) {
      out << "\n";
    }

    // line by line
    if (eType == 2) {
      if (((left - right_prev) >= wordSpace) && (right_prev != -1)) {
        // new word
        out << " ";
      } else if (((left - right_prev) <= (2 * (left - right))) &&
                 (right_prev != -1)) {
        // new line -> if negative difference is bigger than double of letter
        // width
        out << "\n";
      }
    }

    if (eType == 3) {
      if (((left - right_prev) >= wordSpace) && (right_prev != -1))
        // new word
        out << " ";

      if (((left - right_prev) <= (2 * (left - right))) && (right_prev != -1)) {
        // new line -> if negative difference is bigger than double of letter
        // width
        if (line_end_prev == 0)  // first line
          line_end_prev = right;
        if ((left - line_start_prev >= paraIndent) ||  // distance from left
            (top - last_bottom >= paraIndent) ||   // distance between lines
            (abs(right - line_end_prev) >= (paraIndent / 2)))
          // distance from right
          out << "\n";
        else
          out << " ";
        line_start_prev = left;
        line_end_prev = right_prev;
      }
    }

    out << letter;
    right_prev = right;
    last_bottom = bottom;
  }

  out << "\n";
  QApplication::restoreOverrideCursor();
  return true;
}

bool ChildWidget::isBoxSelected() {
  return selectionModel->hasSelection();
}

bool ChildWidget::isItalic() {
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid()) {
    return model->index(index.row(), 6).data().toBool();
  }
  return false;
}

bool ChildWidget::isBold() {
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid()) {
    return model->index(index.row(), 7).data().toBool();
  }
  return false;
}

bool ChildWidget::isUnderLine() {
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid()) {
    return model->index(index.row(), 8).data().toBool();
  }
  return false;
}

bool ChildWidget::isShowSymbol() {
  return symbolShown;
}

bool ChildWidget::isDirectTypingMode() {
  return directTypingMode;
}

bool ChildWidget::isFontColumnsShown() {
  return (!table->isColumnHidden(6));
}

void ChildWidget::setDirectTypingMode(bool v) {
  directTypingMode = v;
}

void ChildWidget::setShowFontColumns(bool v) {
  table->setColumnHidden(6, !v);
  table->setColumnHidden(7, !v);
  table->setColumnHidden(8, !v);
  table->horizontalHeader()->resizeSections(QHeaderView::Stretch);
}

bool ChildWidget::isDrawBoxes() {
  return boxesVisible;
}

bool ChildWidget::isDrawRect() {
  return drawnRectangle;
}

void ChildWidget::setItalic(bool v) {
  QModelIndexList indexes = table->selectionModel()->selection().indexes();
  QModelIndex index;
  QFont letterFont;

  foreach(index, indexes) {
    letterFont = model->data(model->index(index.row(), 0, QModelIndex()),
                             Qt::FontRole).value<QFont>();
    letterFont.setItalic(v);
    model->setData(model->index(index.row(), 0, QModelIndex()), letterFont,
                   Qt::FontRole);
    model->setData(model->index(index.row(), 6, QModelIndex()), v);
  }
}

void ChildWidget::setBolded(bool v) {
  QModelIndexList indexes = table->selectionModel()->selection().indexes();
  QModelIndex index;
  QFont letterFont;

  foreach(index, indexes) {
    letterFont = model->data(model->index(index.row(), 0, QModelIndex()),
                             Qt::FontRole).value<QFont>();
    letterFont.setBold(v);
    model->setData(model->index(index.row(), 0, QModelIndex()), letterFont,
                   Qt::FontRole);
    model->setData(model->index(index.row(), 7, QModelIndex()), v);
  }
}

void ChildWidget::setUnderline(bool v) {
  QModelIndexList indexes = table->selectionModel()->selection().indexes();
  QModelIndex index;
  QFont letterFont;

  foreach(index, indexes) {
    letterFont = model->data(model->index(index.row(), 0, QModelIndex()),
                             Qt::FontRole).value<QFont>();
    letterFont.setUnderline(v);
    model->setData(model->index(index.row(), 0, QModelIndex()), letterFont,
                   Qt::FontRole);
    model->setData(model->index(index.row(), 8, QModelIndex()), v);
  }
}

void ChildWidget::setSelectionRect() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  QFont tableFont = settings.value("GUI/Font").value<QFont>();

  if (tableFont.family().isEmpty()) {
    tableFont.setFamily(TABLE_FONT);
    tableFont.setPointSize(TABLE_FONT_SIZE);
  }

  tableFont.setPointSize((tableFont.pointSize() * 2));
}

void ChildWidget::getZoom() {
  qreal _zoomratio = imageView->transform().m11();
  emit zoomRatioChanged(_zoomratio);
}

void ChildWidget::setZoom(float scale) {
  QTransform transform;

  transform.scale(scale, scale);
  imageView->setTransform(transform);
  qreal _zoomratio = scale;
  emit zoomRatioChanged(_zoomratio);
}

void ChildWidget::zoomIn() {
  imageView->scale(1.2, 1.2);
  if (rectItem.size()) {
    imageView->ensureVisible(rectItem.last());
  }
  getZoom();
}

void ChildWidget::zoomOut() {
  imageView->scale(1 / 1.2, 1 / 1.2);
  if (rectItem.size()) {
    imageView->ensureVisible(rectItem.last());
  }
  getZoom();
}

void ChildWidget::zoomToFit() {
  float viewWidth = imageView->viewport()->width();
  float viewHeight = imageView->viewport()->height();
  float zoomFactor;
  float ratio = viewWidth / viewHeight;
  float aspectRatio = imageWidth / imageHeight;

  if (ratio > aspectRatio) {
    zoomFactor = viewHeight / imageHeight;
  } else {
    zoomFactor = viewWidth / imageWidth;
  }

  setZoom(zoomFactor);
}

void ChildWidget::zoomToHeight() {
  float viewHeight = imageView->viewport()->height();
  float zoomFactor = viewHeight / imageHeight;

  setZoom(zoomFactor);
  if (rectItem.size()) {
    imageView->ensureVisible(rectItem.last());
  }
}

void ChildWidget::zoomToWidth() {
  float viewWidth = imageView->viewport()->width();
  float zoomFactor = viewWidth / imageWidth;

  setZoom(zoomFactor);
  if (rectItem.size()) {
    imageView->ensureVisible(rectItem.last());
  }
}

void ChildWidget::zoomOriginal() {
  setZoom(1);
  if (rectItem.size()) {
    imageView->ensureVisible(rectItem.last());
  }
}

void ChildWidget::zoomToSelection() {
  imageView->fitInView(rectItem.last(), Qt::KeepAspectRatio);
  imageView->scale(1 / 1.1, 1 / 1.1);    // make small border
  if (rectItem.size()) {
    imageView->ensureVisible(rectItem.last());
  }
  imageView->centerOn(rectItem.last());
  getZoom();
}

void ChildWidget::showSymbol() {
  if (symbolShown == false)
    symbolShown = true;
  else
    symbolShown = false;
  drawSelectionRects();
}

void ChildWidget::drawRectangle(bool checked) {
  if (checked) {
    if (!m_DrawRectangle) {
      m_DrawRectangle = new DrawRectangle(this, userFriendlyCurrentFile(),
                                          imageWidth, imageHeight);
    }
    int ret = m_DrawRectangle->exec();
    if (ret) {
      QRect newCoords = m_DrawRectangle->getRectangle();
      if (rectangle) {
        imageScene->removeItem(rectangle);
      }
      rectangle = imageScene->addRect(newCoords.x(), newCoords.y(),
                                      newCoords.width(), newCoords.height(),
                                      QPen(QColor(255, 0, 0, 255)),
                                      QBrush(QColor(255, 0, 0, 100)));
      rectangle->setZValue(1);
      rectangle->setVisible(true);
      drawnRectangle = true;
    }
  } else {
    if (rectangle) {
      // I can not call rectangle->prepareGeometryChange();
      // http://www.qtcentre.org/threads/28749-Qt-4.6-GraphicsView-items-remove-problem
      imageScene->removeItem(rectangle);
      // Workaround
      QRectF isCoords = imageScene->sceneRect();
      imageScene->update(isCoords);

      drawnRectangle = false;
    }
  }
}

void ChildWidget::drawBoxes() {
  if (boxesVisible == false) {
    for (int row = 0; row < model->rowCount(); ++row) {
      int left = model->index(row, 1).data().toInt();
      int top = model->index(row, 4).data().toInt();
      int width = model->index(row, 3).data().toInt() - left;
      int height = model->index(row, 2).data().toInt() - top;
      boxesItem << imageScene->addRect(left, top, width, height,
                                       QPen(boxColor));
    }
    boxesVisible = true;
  } else {
    removeMyItems(boxesItem);
    boxesVisible = false;
  }
}

void ChildWidget::mousePressEvent(QMouseEvent* event) {
  // zoom should be proportional => m11=m22
  qreal zoomFactor = imageView->transform().m22();
  QPointF mouseCoordinates = imageView->mapToScene(event->pos());
  int offset = this->sizes().first() + 6;  // 6 is estimated width  of splitter
  int zoomedOffset = offset / zoomFactor;

  // This handler is for left click events only
  if(event->button() != Qt::LeftButton)
    return;

  // Rubber band
  if(event->modifiers() == Qt::ControlModifier)
  {
    rbOrigin = imageView->mapFromParent(event->pos());
    rubberBand->setGeometry(QRect(rbOrigin, QSize()));
    rubberBand->show();
    grabMouse();
  }
  // BB click selection
  else if(event->modifiers() == Qt::NoModifier)
  {
      for (int row = 0; row < model->rowCount(); ++row)
      {
          int left = model->index(row, 1).data().toInt();
          int bottom = model->index(row, 2).data().toInt();
          int right = model->index(row, 3).data().toInt();
          int top = model->index(row, 4).data().toInt();

          if ((left <= (mouseCoordinates.x() - zoomedOffset) &&
               (mouseCoordinates.x() - zoomedOffset) <= right) &&
              (top <= mouseCoordinates.y()) &&
              (mouseCoordinates.y() <= bottom))
          {
              table->setCurrentIndex(model->index(row, 0));
              table->setFocus();
          }
       }
       drawSelectionRects();
  }
}

void ChildWidget::mouseMoveEvent(QMouseEvent* event) {
  QPoint topleft;
  QPoint botright;
  QPoint rbCurPoint = imageView->mapFromParent(event->pos());
  if(rbOrigin.x() > rbCurPoint.x())
  {
      topleft.setX(rbCurPoint.x());
      botright.setX(rbOrigin.x());
  }
  else
  {
      topleft.setX(rbOrigin.x());
      botright.setX(rbCurPoint.x());
  }
  if(rbOrigin.y() > rbCurPoint.y())
  {
      topleft.setY(rbCurPoint.y());
      botright.setY(rbOrigin.y());
  }
  else
  {
      topleft.setY(rbOrigin.y());
      botright.setY(rbCurPoint.y());
  }
  rubberBand->setGeometry(QRect(topleft, botright));
}

void ChildWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    releaseMouse();
    rubberBand->hide();

    QRect rect(rubberBand->pos(), rubberBand->size());
    QPoint topleft = imageView->mapToScene(rect.topLeft()).toPoint();
    QPoint botright = imageView->mapToScene(rect.bottomRight()).toPoint();
    QItemSelection selection;
    for(int row = 0; row < model->rowCount(); ++row)
    {
        int cx = (model->index(row, 1).data().toInt() + model->index(row, 3).data().toInt())/2;
        int cy = (model->index(row, 2).data().toInt() + model->index(row, 4).data().toInt())/2;

        if(cx >= topleft.x() && cx <= botright.x() && cy >= topleft.y() && cy <= botright.y())
            selection.push_back(QItemSelectionRange(model->index(row, 0)));
    }

    table->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    table->setFocus();
    if(!selection.empty())
        table->setCurrentIndex(selection.back().indexes().last());
    drawSelectionRects();
}

bool ChildWidget::eventFilter(QObject* object, QEvent* event) {
  if (event->type() == QEvent::KeyRelease) {
    // transforms QEvent into QKeyEvent
    QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(event);
    if (pKeyEvent->modifiers() & Qt::ControlModifier) {
      switch (pKeyEvent->key()) {
      case Qt::Key_C: {
        copyFromCell();
        break;
      }
      case Qt::Key_V: {
        pasteToCell();
        break;
      }
      }
      pKeyEvent->accept();
      return true;
    } else if (directTypingMode) {
      directType(pKeyEvent);
    } else {
      return QWidget::eventFilter(object, pKeyEvent);
    }
  }
  else if(event->type() == QEvent::GraphicsSceneMouseMove) {
      rubberBand->setGeometry(QRect(QPoint(30, 30), QSize(10, 10)));
      rubberBand->show();
      return true;
  }

  return false;
}

void ChildWidget::moveSymbolRow(int direction) {
  QModelIndex index = selectionModel->currentIndex();

  int currentRow = index.row();
  // check if any row is selected
  if (currentRow < 0)
    return;

  // check where if we are if move is possible top/bottom
  if (direction < 0 && currentRow == 0) {
    return;
  } else if (direction > 0 && currentRow == model->rowCount()) {
    return;
  } else {
    UndoItem ui;
    ui.m_eop = euoReplace;
    ui.m_origrow = currentRow;
    ui.m_extrarow = currentRow + direction;

    for (int j = 0; j < 9; j++) {
      ui.m_vdata[j] = model->index(currentRow, j).data();
      ui.m_vextradata[j] = model->index(ui.m_extrarow, j).data();

      model->setData(model->index(ui.m_extrarow, j), ui.m_vdata[j]);
      model->setData(model->index(ui.m_origrow, j), ui.m_vextradata[j]);
    }

    m_undostack.push(ui);
    // activate new row
    table->setCurrentIndex(model->index(ui.m_extrarow, 0));
    drawSelectionRects();
  }
}

void ChildWidget::copyFromCell() {
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(table->currentIndex().data().toString());
}

void ChildWidget::pasteToCell() {
  const QClipboard* clipboard = QApplication::clipboard();
  QModelIndex index = selectionModel->currentIndex();

  UndoItem ui;
  ui.m_eop = euoChange;
  ui.m_origrow = index.row();

  for (int i = 0; i < 9; i++)
    ui.m_vdata[i] = model->index(ui.m_origrow, i).data();

  // do not paste string to int fields
  if ((index.column() > 0 && index.column() < 5) &&
      (clipboard->text().toInt() > 0)) {
    model->setData(table->currentIndex(), clipboard->text().toInt());
    m_undostack.push(ui);
  }

  // paste string only to string field
  if (index.column() == 0) {
    model->setData(table->currentIndex(), clipboard->text());
    m_undostack.push(ui);
  }

  if (directTypingMode)
    table->setCurrentIndex(model->index(index.row() + 1, 0));


  drawSelectionRects();
}

void ChildWidget::directType(QKeyEvent* event) {
  QModelIndex index = selectionModel->currentIndex();
  if (!event->text().toAscii().trimmed().isEmpty() &&
      (event->key() !=  Qt::Key_Delete))  {
    // enter only text
    if ((event->key() ==  Qt::Key_Enter) || (event->key() ==  Qt::Key_Return)) {
      // enter/return move to next row
      table->setCurrentIndex(model->index(index.row() + 1, 0));
    } else  {
      model->setData(model->index(index.row(), 0, QModelIndex()),
                     event->text());

      UndoItem ui;
      ui.m_eop = euoChange;
      ui.m_origrow = index.row();

      for (int i = 0; i < 9; i++)
        ui.m_vdata[i] = model->index(ui.m_origrow, i).data();

      m_undostack.push(ui);
      table->setCurrentIndex(model->index(index.row() + 1, 0));
    }
  }
  event->accept();
  drawSelectionRects();
}

void ChildWidget::insertSymbol() {
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid()) {
    int leftBorder = model->index(index.row(), 3).data().toInt() + 1;
    int rightBorder = model->index(index.row() + 1, 1).data().toInt() - 1;

    if (leftBorder > rightBorder)  // end of line or overlapping boxes
      rightBorder = leftBorder +
                    (leftBorder - model->index(index.row(), 1).data().toInt());
    model->insertRow(index.row() + 1);
    model->setData(model->index(index.row() + 1, 0), "*");
    model->setData(model->index(index.row() + 1, 1), leftBorder);
    model->setData(model->index(index.row() + 1, 2),
                   model->index(index.row(), 2).data().toInt());
    model->setData(model->index(index.row() + 1, 3), rightBorder);
    model->setData(model->index(index.row() + 1, 4),
                   model->index(index.row(), 4).data().toInt());
    model->setData(model->index(index.row() + 1, 5),
                   model->index(index.row(), 5).data().toInt());
    model->setData(model->index(index.row() + 1, 6),
                   model->index(index.row(), 6).data().toBool());
    model->setData(model->index(index.row() + 1, 7),
                   model->index(index.row(), 7).data().toBool());
    model->setData(model->index(index.row() + 1, 8),
                   model->index(index.row(), 8).data().toBool());

    table->setCurrentIndex(model->index(index.row() + 1, 0));
    table->setFocus();

    UndoItem ui;
    ui.m_eop = euoAdd;
    ui.m_origrow = index.row() + 1;
    m_undostack.push(ui);

    drawSelectionRects();
  }
}

void ChildWidget::splitSymbol() {
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid()) {
    UndoItem ui;
    ui.m_eop = euoSplit;
    ui.m_origrow = index.row();
    ui.m_extrarow = ui.m_origrow + 1;

    for (int i = 0; i < 9; i++)
      ui.m_vdata[i] = model->index(ui.m_origrow, i).data();

    m_undostack.push(ui);

    QModelIndex left = model->index(index.row(), 1);
    QModelIndex right = model->index(index.row(), 3);
    int width = right.data().toInt() - left.data().toInt();
    model->insertRow(index.row() + 1);
    model->setData(model->index(index.row() + 1, 0), "*");
    model->setData(model->index(index.row() + 1, 1),
                   right.data().toInt() - width / 2);
    model->setData(model->index(index.row() + 1, 2),
                   model->index(index.row(), 2).data().toInt());
    model->setData(model->index(index.row() + 1, 3), right.data().toInt());
    model->setData(model->index(index.row() + 1, 4),
                   model->index(index.row(), 4).data().toInt());
    model->setData(model->index(index.row() + 1, 5),
                   model->index(index.row(), 5).data().toInt());
    model->setData(model->index(index.row() + 1, 6),
                   model->index(index.row(), 6).data().toBool());
    model->setData(model->index(index.row() + 1, 7),
                   model->index(index.row(), 7).data().toBool());
    model->setData(model->index(index.row() + 1, 8),
                   model->index(index.row(), 8).data().toBool());
    model->setData(right, right.data().toInt() - width / 2);
    drawSelectionRects();
  }
}

void ChildWidget::joinSymbol() {
  QModelIndex index = selectionModel->currentIndex(), next = model->index(
                        index.row() + 1,
                        index.column());

  if (index.isValid() && next.isValid()) {
    int row = index.row();

    UndoItem ui;
    ui.m_eop = euoJoin;
    ui.m_origrow = row;
    ui.m_extrarow = row + 1;

    for (int i = 0; i < 9; i++) {
      ui.m_vdata[i] = model->index(ui.m_origrow, i).data();
      ui.m_vextradata[i] = model->index(ui.m_extrarow, i).data();
    }

    m_undostack.push(ui);

    model->setData(model->index(row, 0),
                   model->index(row, 0).data().toString() + model->index(row
                       + 1, 0).data().toString());
    my_min(2, 3);
    model->setData(model->index(row, 1),
                   my_min(model->index(row, 1).data().toInt(), model->index(row
                       + 1, 1).data().toInt()));
    model->setData(model->index(row, 2),
                   my_max(model->index(row, 2).data().toInt(), model->index(row
                       + 1, 2).data().toInt()));
    model->setData(model->index(row, 3),
                   my_max(model->index(row, 3).data().toInt(), model->index(row
                       + 1, 3).data().toInt()));
    model->setData(model->index(row, 4),
                   my_min(model->index(row, 4).data().toInt(), model->index(row
                       + 1, 4).data().toInt()));
    model->setData(model->index(row, 5),
                   my_min(model->index(row, 5).data().toInt(), model->index(row
                       + 1, 5).data().toInt()));
    model->setData(model->index(row, 6),
                   model->index(row, 6).data().toBool() || model->index(row
                       + 1, 6).data().toBool());
    model->setData(model->index(row, 7),
                   model->index(row, 7).data().toBool() || model->index(row
                       + 1, 7).data().toBool());
    model->setData(model->index(row, 8),
                   model->index(row, 8).data().toBool() || model->index(row
                       + 1, 8).data().toBool());
    model->removeRow(row + 1);
    table->setCurrentIndex(model->index(row, 0));
    table->setFocus();
    drawSelectionRects();
  }
}

void ChildWidget::deleteSymbol() {
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid()) {
    UndoItem ui;
    ui.m_eop = euoDelete;
    ui.m_origrow = index.row();

    for (int i = 0; i < 9; i++)  // TODO(zdenop): replace 9 with columns count
      ui.m_vdata[i] = model->index(ui.m_origrow, i).data();

    m_undostack.push(ui);
    model->removeRow(index.row());

    table->setCurrentIndex(model->index(ui.m_origrow, 0));
    table->setFocus();

    drawSelectionRects();
    documentWasModified();
  }
}

void ChildWidget::moveUp() {
  moveSymbolRow(-1);
}

void ChildWidget::moveDown() {
  moveSymbolRow(1);
}

void ChildWidget::moveTo() {
  if (table->currentIndex().row() < 0)
    return;

  int sourceRow = table->currentIndex().row();
  int destRow = 0;
  GetRowIDDialog dialog(this);

  if (dialog.exec()) {
    QString string = dialog.lineEdit->text();

    if (string.toInt() == 0)
      destRow = string.toInt();
    else
      destRow = string.toInt() - 1;

    if (destRow > model->rowCount())
      destRow = model->rowCount() - 1;
  }

  if ((destRow - sourceRow) > 0)
    moveSymbolRow(destRow - sourceRow);
  else
    moveSymbolRow(destRow - sourceRow);

  table->resizeRowToContents(destRow);
}

void ChildWidget::goToRow() {
  GetRowIDDialog dialog(this);
  int row;
  if (dialog.exec()) {
    QString string = dialog.lineEdit->text();

    if (string.toInt() == 0)
      row = string.toInt();
    else
      row = string.toInt() - 1;

    if (row > model->rowCount())
      row = model->rowCount() - 1;

    table->setCurrentIndex(model->index(row, 0));
    table->setFocus();
    drawSelectionRects();
  }
}

void ChildWidget::find() {
  if (!f_dialog) {
    f_dialog = new FindDialog(this, userFriendlyCurrentFile());
    connect(f_dialog, SIGNAL(findNext(const QString &,
                                      Qt::CaseSensitivity)),
            this, SLOT(findNext(const QString &,
                                Qt::CaseSensitivity)));
    connect(f_dialog, SIGNAL(findPrev(const QString &,
                                      Qt::CaseSensitivity)),
            this, SLOT(findPrev(const QString &,
                                Qt::CaseSensitivity)));
  }

  f_dialog->show();
  f_dialog->raise();
  f_dialog->activateWindow();
}

QString ChildWidget::userFriendlyCurrentFile() {
  return strippedName(boxFile);
}

/* Get symbol string and convert it to hexadecimal codes */
QString ChildWidget::getSymbolHexCode() {
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid()) {
    QString symbol = model->index(index.row(), 0).data().toString();
    QString result = "";
    for (int i = 0; i < symbol.size(); ++i) {
      QString str2 = QString::number(symbol[i].unicode(),
                                     16).toUpper().rightJustified(4, '0');
      result.append("0x" + str2 + " ");
    }
    return result;
  }
  return QString::null;
}

/* Get size of box */
QString ChildWidget::getBoxSize() {
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid()) {
    int left = model->index(index.row(), 1).data().toInt();
    int bottom = model->index(index.row(), 2).data().toInt();
    int right = model->index(index.row(), 3).data().toInt();
    int top = model->index(index.row(), 4).data().toInt();
    QString result;
    result = QString("%1x%2").arg(right - left).arg(bottom - top);
    return result;
  }
  return QString::null;
}

QString ChildWidget::currentBoxFile() {
  return QFileInfo(boxFile).canonicalFilePath();
}

void ChildWidget::documentWasModified() {
  modified = true;
  emit modifiedChanged();
}

void ChildWidget::emitBoxChanged() {
  drawSelectionRects();
  emit boxChanged();
}

void ChildWidget::removeMyItems(QVector<QGraphicsRectItem *> &graphicsItems) {
  foreach(QGraphicsItem *item, graphicsItems) {
    imageScene->removeItem(item);
  }
  graphicsItems.clear();
}

void ChildWidget::clearBalloons()
{
    for(int i = 0; i < balloons.size(); ++i)
    {
        imageScene->removeItem(balloons[i].symbol);
        balloons[i].symbol->deleteLater();
        for(int j = 0; j < BalloonSymbol::haloCompCount; ++j)
        {
            imageScene->removeItem(balloons[i].halo[j]);
            balloons[i].halo[j]->deleteLater();
        }
    }
    balloons.clear();
}

void ChildWidget::updateBalloons()
{
    int idx = table->selectionModel()->selection().indexes().last().row();
    int min_idx = my_max(idx - balloonCount/2, 0);
    int max_idx = my_min(idx + balloonCount/2, model->rowCount() - 1);
    QFont tableFont = table->font();
    tableFont.setPointSize(2*tableFont.pointSize());
    for(int i = min_idx; i <= max_idx; ++i)
    {
        balloons.push_back(BalloonSymbol());

        QString letter = model->index(i, 0).data().toString();
        int left = model->index(i, 1).data().toInt();
        int top = model->index(i, 4).data().toInt();

        balloons.back().symbol = imageScene->addText(letter, tableFont);
        QGraphicsTextItem* curSymbol = balloons.back().symbol;
        // TODO(zdenop): get font metrics and calculate better placement
        // (e.g. visible in case of narrow margin)
        curSymbol->setPos(QPoint(left, top - 16*2 - 15));
        curSymbol->setDefaultTextColor(Qt::red);
        curSymbol->setZValue(2);
        curSymbol->setVisible(true);

        for(int j = 0; j < BalloonSymbol::haloCompCount; ++j)
        {
            balloons.back().halo[j] = imageScene->addText(letter, tableFont);
            QGraphicsTextItem* curHalo = balloons.back().halo[j];
            curHalo->setDefaultTextColor(Qt::white);
            curHalo->setZValue(1);
        }
        balloons.back().halo[0]->setPos(curSymbol->pos() + QPoint( 2,  2));
        balloons.back().halo[1]->setPos(curSymbol->pos() + QPoint(-2, -2));
        balloons.back().halo[2]->setPos(curSymbol->pos() + QPoint(-2,  2));
        balloons.back().halo[3]->setPos(curSymbol->pos() + QPoint( 2, -2));
        for(int j = 0; j < BalloonSymbol::haloCompCount; ++j)
          balloons.back().halo[j]->setVisible(true);
    }   // for i (idx)

    // Make the first (OK?) balloon is visible
    imageView->ensureVisible(balloons.front().symbol);
}

void ChildWidget::drawSelectionRects() {
  QModelIndexList indexes = table->selectionModel()->selection().indexes();

  if (!indexes.empty()) {
    removeMyItems(rectItem);
    clearBalloons();

    for (int i = 0; i < indexes.size(); ++i) {
      int row = indexes[i].row();
      int left = model->index(row, 1).data().toInt();
      int bottom = model->index(row, 2).data().toInt();
      int right = model->index(row, 3).data().toInt();
      int top = model->index(row, 4).data().toInt();
      rectItem << imageScene->addRect(QRectF(QPoint(left, top),
                                             QPointF(right, bottom)),
                                      QPen(rectColor));
    }

    rectItem.last()->setZValue(1);
    imageView->ensureVisible(rectItem.last());
    if (symbolShown == true && indexes.size() == 1)
      updateBalloons();
  } else
    clearBalloons();
}

void ChildWidget::closeEvent(QCloseEvent* event) {
  if (!maybeSave()) {
    event->ignore();
  }
  if (f_dialog)
    delete f_dialog;
}

bool ChildWidget::maybeSave() {
  if (isModified()) {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, SETTING_APPLICATION,
                               tr("'%1' has been modified.\n"
                                  "Do you want to save your changes?").arg(
                                 userFriendlyCurrentFile()), QMessageBox::Save
                               | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
      return save(boxFile);
    else if (ret == QMessageBox::Cancel)
      return false;
  }
  return true;
}

void ChildWidget::setCurrentImageFile(const QString& fileName) {
  imageFile = QFileInfo(fileName).canonicalFilePath();
}

void ChildWidget::setCurrentBoxFile(const QString& fileName) {
  boxFile = QFileInfo(fileName).canonicalFilePath();
}

QString ChildWidget::strippedName(const QString& fullFileName) {
  return QFileInfo(fullFileName).fileName();
}

void ChildWidget::cbFontToggleProxy(bool checked)
{
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    QModelIndex index = selectedIndexes.first();
    int col = index.column();
    switch(col)
    {
    case 6 :
        setItalic(checked);
        break;
    case 7 :
        setBolded(checked);
        break;
    case 8 :
        setUnderline(checked);
        break;
    default :
        break;
    }
}

void ChildWidget::sbValueChanged(int sbdValue) {
  QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
  QModelIndex index = selectedIndexes.first();

  int left = model->index(index.row(), 1).data().toInt();
  int bottom = model->index(index.row(), 2).data().toInt();
  int right = model->index(index.row(), 3).data().toInt();
  int top = model->index(index.row(), 4).data().toInt();

  switch (index.column()) {
  case 1:
    left = sbdValue;
    break;
  case 2:
    bottom = sbdValue;
    break;
  case 3:
    right = sbdValue;
    break;
  case 4:
    top = sbdValue;
    break;
  default :
    break;
  }

  UndoItem ui;
  ui.m_eop = euoChange;
  ui.m_origrow = index.row();

  for (int i = 0; i < 9; i++)
    ui.m_vdata[i] = model->index(ui.m_origrow, i).data();

  m_undostack.push(ui);

  rectItem.first()->setRect(QRectF(QPoint(left, top),
                                   QPointF(right, bottom)));
  imageView->ensureVisible(rectItem.first());
}

void ChildWidget::findNext(const QString &symbol, Qt::CaseSensitivity mc) {
  int row = table->currentIndex().row() + 1;
  while (row < model->rowCount()) {
    QString letter = model->index(row, 0).data().toString();
    if (letter.contains(symbol, mc)) {
      table->setCurrentIndex(model->index(row, 0));
      table->setFocus();
      drawSelectionRects();
      return;
    }
    ++row;
  }
  QApplication::beep();
}

void ChildWidget::findPrev(const QString &symbol,
                           Qt::CaseSensitivity mc) {
  int row = table->currentIndex().row() - 1;
  while (row >= 0) {
    QString letter = model->index(row, 0).data().toString();
    if (letter.contains(symbol, mc)) {
      table->setCurrentIndex(model->index(row, 0));
      table->setFocus();
      drawSelectionRects();
      return;
    }
    --row;
  }
  QApplication::beep();
}

bool ChildWidget::isUndoAvailable() {
  return m_undostack.isEmpty() ? false : true;
}

void ChildWidget::undo() {
  if (m_undostack.isEmpty()) {
    emit boxChanged();  // update toolbar/menu to disable undo action
    // TODO(all): it is not working perfectly (are there some "ghost" undo
    // actions?). Maybe we need to implement UndoStackView similar to
    // http://doc-snapshot.qt-project.org/4.8/demos-undo.html
    return;
  }

  UndoItem ui = m_undostack.pop();

  switch (ui.m_eop) {
  case euoAdd:
    // Item was added. Reverse is remove it.
    undoDelete(ui);
    break;
  case euoDelete:
    // Item was deleted. Reverse is put it back.
    undoAdd(ui);
    break;
  case euoChange:
    // Item was edited. Put back old values.
    undoEdit(ui);
    break;
  case euoJoin:
    // Two items joined. Split back.
    undoSplit(ui);
    break;
  case euoSplit:
    // Item split in two. Join back.
    undoJoin(ui);
    break;
  case euoReplace:
    // Two item changed places. Change places back.
    undoMoveBack(ui);
    break;
  default:
    // Nothing to dofor other cases. Report error.

    QMessageBox::warning(
      this,
      SETTING_APPLICATION,
      "Invalid undo operation.");
    break;
  }
}

// Delete item as undo operation of add
void ChildWidget::undoDelete(UndoItem& ui) {
  model->removeRow(ui.m_origrow);

  int rows = model->rowCount();

  int newfocusrow = ui.m_origrow;

  if (newfocusrow > rows)
    newfocusrow = rows;

  table->setCurrentIndex(model->index(newfocusrow, 0));
  table->setFocus();
  drawSelectionRects();
}

// Add back item as undo operation of delete
void ChildWidget::undoAdd(UndoItem& ui) {
  model->insertRow(ui.m_origrow);
  undoEdit(ui);
}

// Put back edited values
void ChildWidget::undoEdit(UndoItem& ui) {
  for (int i = 0; i < 9; i++)
    model->setData(model->index(ui.m_origrow, i), ui.m_vdata[i]);

  table->setCurrentIndex(model->index(ui.m_origrow, 0));
  table->setFocus();
  drawSelectionRects();
}

// Re-join split rows
void ChildWidget::undoJoin(UndoItem& ui) {
  model->removeRow(ui.m_extrarow);
  undoEdit(ui);
}

// Split back joined lines
void ChildWidget::undoSplit(UndoItem& ui) {
  model->insertRow(ui.m_extrarow);

  for (int i = 0; i < 9; i++)
    model->setData(model->index(ui.m_extrarow, i), ui.m_vextradata[i]);

  undoEdit(ui);
}

// Put replaced rows back to original location
void ChildWidget::undoMoveBack(UndoItem& ui) {
  for (int i = 0; i < 9; i++)
    model->setData(model->index(ui.m_extrarow, i), ui.m_vextradata[i]);

  undoEdit(ui);
}
