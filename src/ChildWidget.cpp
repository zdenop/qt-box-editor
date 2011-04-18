/**********************************************************************
* File:        ChildWidget.cpp
* Description: ChildWidget functions
* Author:      Marcel Kolodziejczyk
* Created:     2010-01-06
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

#include "ChildWidget.h"
#include "GetRowIDDialog.h"
#include "Settings.h"
#include "SettingsDialog.h"

ChildWidget::ChildWidget(QWidget* parent) :
  QSplitter(Qt::Horizontal, parent)
{
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
  connect(
    selectionModel,
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this, SLOT(drawSelectionRects()));
  table->setSelectionModel(selectionModel);
  //table->verticalHeader()->hide();
  table->setSelectionBehavior(QAbstractItemView::SelectItems);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->hideColumn(5);
  table->hideColumn(6);
  table->hideColumn(7);
  table->hideColumn(8);
  table->installEventFilter(this); //(this);   // installs event filter

  // Font for table
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);
  QFont tableFont = settings.value("GUI/Font").value<QFont>();

  if (tableFont.family().isEmpty())
    {
      tableFont.setFamily(TABLE_FONT);
      tableFont.setPointSize(TABLE_FONT_SIZE);
    }
  table->setFont(tableFont);

  if (settings.contains("GUI/Rectagle"))
    {
      rectColor = settings.value("GUI/Rectagle").value<QColor>();
    }
  else
    {
      rectColor = Qt::red;
    }

  if (settings.contains("GUI/Rectagle_fill"))
    rectFillColor = settings.value("GUI/Rectagle_fill").value<QColor>();
  else
    {
      rectFillColor = Qt::red;
      rectFillColor.setAlpha(127);
    }

  if (settings.contains("GUI/Box"))
    {
      boxColor = settings.value("GUI/Box").value<QColor>();
    }
  else
    {
      boxColor = Qt::green;
    }

  if (settings.contains("GUI/BackgroundColor"))
    {
      backgroundColor = settings.value("GUI/BackgroundColor").value<QColor>();
    }
  else
    {
      backgroundColor = (Qt::gray);
    }

  // Make graphics Scene and View
  imageScene = new QGraphicsScene;
  imageView = new QGraphicsView(imageScene);
  imageView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
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

  QPushButton* goToButton = new QPushButton();
  goToButton->setIcon(QIcon(":/images/gtk-jump-to-ltr.png"));
  goToButton->setToolTip(tr("Go to row…"));
  goToButton->setMinimumSize(QSize(24, 24));
  goToButton->setMaximumSize(QSize(24, 24));
  connect(goToButton, SIGNAL(clicked()), this, SLOT(goToRow()));

  QPushButton* insertButton = new QPushButton();
  insertButton->setIcon(QIcon(":/images/insertRow.svg"));
  insertButton->setToolTip(tr("Insert row"));
  insertButton->setMaximumSize(QSize(24, 24));
  connect(insertButton, SIGNAL(clicked()), this, SLOT(insertSymbol()));

  QPushButton* joinButton = new QPushButton();
  joinButton->setIcon(QIcon(":/images/joinRow.svg"));
  joinButton->setToolTip(tr("Join rows"));
  joinButton->setMaximumSize(QSize(24, 24));
  connect(joinButton, SIGNAL(clicked()), this, SLOT(joinSymbol()));

  QPushButton* splitButton = new QPushButton();
  splitButton->setIcon(QIcon(":/images/splitRow.svg"));
  splitButton->setToolTip(tr("Split row"));
  splitButton->setMaximumSize(QSize(24, 24));
  connect(splitButton, SIGNAL(clicked()), this, SLOT(splitSymbol()));

  QPushButton* removeButton = new QPushButton();
  removeButton->setIcon(QIcon(":/images/deleteRow.png"));
  removeButton->setToolTip(tr("Remove row"));
  removeButton->setMaximumSize(QSize(24, 24));
  connect(removeButton, SIGNAL(clicked()), this, SLOT(deleteSymbol()));

  QHBoxLayout* movementLayout = new QHBoxLayout();
  movementLayout->setSpacing(2);
  movementLayout->addWidget(upButton);
  movementLayout->addWidget(downButton);
  movementLayout->addWidget(goToButton);

  QHBoxLayout* actionLayout = new QHBoxLayout();
  actionLayout->setSpacing(2);
  actionLayout->addWidget(insertButton);
  actionLayout->addWidget(joinButton);
  actionLayout->addWidget(splitButton);
  actionLayout->addWidget(removeButton);

  QSpacerItem* horizontalSpacer = new QSpacerItem(73, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  QGridLayout* gridLayout = new QGridLayout();
  gridLayout->setContentsMargins(3, 3, 30, 3);
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

  setSelectionRect();
  widgetWidth = parent->size().width();
  modified = false;
  boxesVisible = false;
  symbolShown = true;
  directTypingMode = false;
}

bool ChildWidget::loadImage(const QString& fileName)
{
  QImage image(fileName);

  if (image.isNull())
    {
      QMessageBox::information(this, tr("Wrong file"), tr("Cannot load %1.").arg(fileName));
      return false;
    }
  QString boxFileName = QFileInfo(fileName).path() + QDir::separator()
                        + QFileInfo(fileName).completeBaseName() + ".box";

  if (!QFile::exists(boxFileName))
    {
      QMessageBox::warning(this, tr("Missing file"), tr(
                             "Cannot load image, because there is no corresponding box file"));
      return false;
    }
  imageHeight = image.height();
  imageWidth = image.width();

  if (loadBoxes(boxFileName))
    {
      setCurrentImageFile(fileName);
      imageItem = imageScene->addPixmap(QPixmap::fromImage(image));
      imageSelectionRect->setParentItem(imageItem);
      modified = false;
      emit modifiedChanged();
      connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(emitBoxChanged()));
      connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(documentWasModified()));
    }
  else
    return false;

  return true;
}

bool ChildWidget::loadBoxes(const QString& fileName)
{
  QFile file(fileName);

  if (!file.open(QFile::ReadOnly | QFile::Text))
    {
      QMessageBox::warning(this, SETTING_APPLICATION, tr("Cannot read file %1:\n%2.").arg(fileName).arg(
                             file.errorString()));
      return false;
    }

  QTextStream in(&file);
  in.setCodec("UTF-8");
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString line;
  int row = 0;
  int firstPage = -1;
  do
    {
      line = in.readLine();
      if (!line.isEmpty())
        {
          QFont letterFont;
          QStringList pieces = line.split(" ", QString::SkipEmptyParts);
          QString letter = pieces.value(0);
          bool bold = false, italic = false, underline = false;
          // formating is present only in case there are more than 2 letters
          if (letter.at(0) == '@' && letter.size() > 1)
            {
              bold = true;
              letterFont.setBold(true);
              letter.remove(0, 1);
            }
          if (letter.at(0) == '$' && letter.size() > 1)
            {
              italic = true;
              letterFont.setItalic(true);
              letter.remove(0, 1);
            }
          if (letter.at(0) == '\'' && letter.size() > 1)
            {
              underline = true;
              letterFont.setUnderline(true);
              letter.remove(0, 1);
            }
          int left = pieces.value(1).toInt();
          int bottom = imageHeight - pieces.value(2).toInt();
          int right = pieces.value(3).toInt();
          int top = imageHeight - pieces.value(4).toInt();
          int page = pieces.value(5).toInt();

          // TODO: implement support for multipage tif
          if (firstPage < 0)
            firstPage = page;  // first page can have number 5 ;-)
          if (firstPage == page)   // ignore other pages than first page
            {
              model->insertRow(row);
              model->setData(model->index(row, 0, QModelIndex()), letterFont, Qt::FontRole);
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
    }
  while (!line.isEmpty());

  file.close();
  QApplication::restoreOverrideCursor();

  setCurrentBoxFile(fileName);

  table->resizeColumnsToContents();
  table->resizeRowsToContents();
  table->setCornerButtonEnabled(true);
  table->setWordWrap(true);
  table->horizontalHeader()->setStretchLastSection(true);

  // set size of table
  int tableVisibleWidth = 2 * table->frameWidth();
  if (!table->verticalHeader()->isHidden())
    {
      tableVisibleWidth += 35; // TODO: table->verticalHeader()/horizontalHeader()->width() is 0
    }

  tableVisibleWidth += 25; // scrollbar

  for (int col = 0; col < table->horizontalHeader()->count(); col++)
    {
      tableVisibleWidth += table->columnWidth(col);
    }
  QList<int> splitterSizes;
  splitterSizes << tableVisibleWidth;
  splitterSizes << widgetWidth - tableVisibleWidth - this->handleWidth();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setSizes(splitterSizes);

  return true;
}

bool ChildWidget::save(const QString& fileName)
{
  QFile file(fileName);

  if (!file.open(QFile::WriteOnly | QFile::Text))
    {
      QMessageBox::warning(
        this,
        SETTING_APPLICATION,
        tr("Cannot write file %1:\n%2.").arg(boxFile).arg(file.errorString()));
      return false;
    }

  QTextStream out(&file);
  out.setCodec("UTF-8");
  QApplication::setOverrideCursor(Qt::WaitCursor);

  for (int row = 0; row < model->rowCount(); ++row)
    {
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
      out << letter << " " << left << " " << imageHeight - bottom << " " << right << " "
          << imageHeight - top << " " << page << "\n";
    }

  QApplication::restoreOverrideCursor();

  modified = false;
  emit modifiedChanged();

  return true;
}

bool ChildWidget::isBoxSelected()
{
  return selectionModel->hasSelection();
}

bool ChildWidget::isItalic()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      return model->index(index.row(), 6).data().toBool();
    }
  return false;
}

bool ChildWidget::isBold()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      return model->index(index.row(), 7).data().toBool();
    }
  return false;
}

bool ChildWidget::isUnderLine()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      return model->index(index.row(), 8).data().toBool();
    }
  return false;
}

bool ChildWidget::isShowSymbol()
{
  return symbolShown;
}

bool ChildWidget::isDirectTypingMode()
{
  return directTypingMode;
}

void ChildWidget::setDirectTypingMode(bool v)
{
  directTypingMode = v;
}

bool ChildWidget::isDrawBoxes()
{
  return boxesVisible;
}

void ChildWidget::setItalic(bool v)
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      QFont letterFont;
      letterFont.setItalic(v);
      model->setData(model->index(index.row(), 0, QModelIndex()), letterFont, Qt::FontRole);
      model->setData(model->index(index.row(), 6, QModelIndex()), v);
    }
}

void ChildWidget::setBolded(bool v)
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      QFont letterFont;
      /* TODO: switching between italic bold <-> bold/italics is strange
      if (isItalic() && !isBold())
        letterFont.setStyle(QFont::StyleOblique);
      else
      */
      letterFont.setBold(v);
      model->setData(model->index(index.row(), 0, QModelIndex()), letterFont, Qt::FontRole);
      model->setData(model->index(index.row(), 7, QModelIndex()), v);
    }
}

void ChildWidget::setUnderline(bool v)
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      QFont letterFont;
      letterFont.setUnderline(v);
      model->setData(model->index(index.row(), 0, QModelIndex()), letterFont, Qt::FontRole);
      model->setData(model->index(index.row(), 8, QModelIndex()), v);
    }
}

void ChildWidget::setSelectionRect()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);
  QFont tableFont = settings.value("GUI/Font").value<QFont>();

  if (tableFont.family().isEmpty())
    {
      tableFont.setFamily(TABLE_FONT);
      tableFont.setPointSize(TABLE_FONT_SIZE);
    }

  tableFont.setPointSize((tableFont.pointSize() * 2));

  text2 = imageScene->addText(QString(""), tableFont);
  text2->setDefaultTextColor(Qt::red);
  text2->setZValue(1);
  text2->setPos(QPoint(0, 0));

  imageSelectionRect = imageScene->addRect(0, 0, 0, 0, QPen(rectColor), rectFillColor);
  imageSelectionRect->setZValue(1);
}

void ChildWidget::setZoom(float scale)
{
  QTransform transform;

  transform.scale(scale, scale);
  imageView->setTransform(transform);
}

void ChildWidget::zoomIn()
{
  imageView->scale(1.2, 1.2);
  imageView->ensureVisible(imageSelectionRect);
}

void ChildWidget::zoomOut()
{
  imageView->scale(1 / 1.2, 1 / 1.2);
  imageView->ensureVisible(imageSelectionRect);
}

void ChildWidget::zoomToFit()
{
  float viewWidth = imageView->viewport()->width();
  float viewHeight = imageView->viewport()->height();
  float zoomFactor;
  float ratio = viewWidth / viewHeight;
  float aspectRatio = imageWidth / imageHeight;

  if (ratio > aspectRatio)
    {
      zoomFactor = viewHeight / imageHeight;
    }
  else
    {
      zoomFactor = viewWidth / imageWidth;
    }

  setZoom(zoomFactor);
}

void ChildWidget::zoomToHeight()
{
  float viewHeight = imageView->viewport()->height();
  float zoomFactor = viewHeight / imageHeight;

  setZoom(zoomFactor);
  imageView->ensureVisible(imageSelectionRect);
}

void ChildWidget::zoomToWidth()
{
  float viewWidth = imageView->viewport()->width();
  float zoomFactor = viewWidth / imageWidth;

  setZoom(zoomFactor);
  imageView->ensureVisible(imageSelectionRect);
}

void ChildWidget::zoomOriginal()
{
  setZoom(1);
  imageView->ensureVisible(imageSelectionRect);
}

void ChildWidget::zoomToSelection()
{
  imageView->fitInView(imageSelectionRect, Qt::KeepAspectRatio);
  imageView->scale(1 / 1.1, 1 / 1.1);    // make small border
  imageView->ensureVisible(imageSelectionRect);
  imageView->centerOn(imageSelectionRect);
}

void ChildWidget::showSymbol()
{
  if (symbolShown == false)
    symbolShown = true;
  else
    symbolShown = false;
  drawSelectionRects();
}

void ChildWidget::drawBoxes()
{
  if (boxesVisible == false)
    {
      for (int row = 0; row < model->rowCount(); ++row)
        {
          int left = model->index(row, 1).data().toInt();
          int top = model->index(row, 4).data().toInt();
          int width = model->index(row, 3).data().toInt() - left;
          int height = model->index(row, 2).data().toInt() - top;
          imageScene->addRect(left, top, width, height, QPen(boxColor));
          boxesVisible = true;
        }
    }
  else
    {
      deleteBoxes(imageScene->items());
      boxesVisible = false;
    }
}

void ChildWidget::mousePressEvent(QMouseEvent* event)
{
  qreal zoomFactor = imageView->transform().m22();  // zoom should be proportional => m11=m22
  QPointF mouseCoordinates = imageView->mapToScene(event->pos());
  int offset = this->sizes().first() + 6;  // 6 is estimated width  of splitter
  int zoomedOffset = offset / zoomFactor;

  if (event->button() == Qt::LeftButton)
    {
      for (int row = 0; row < model->rowCount(); ++row)
        {
          QString letter = model->index(row, 0).data().toString();
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

bool ChildWidget::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::KeyRelease)
    {
      // transforms QEvent into QKeyEvent
      QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(event);
      if (pKeyEvent->modifiers() & Qt::ControlModifier)
        {
          switch (pKeyEvent->key())
            {
              case Qt::Key_Up:
                {
                  moveSymbolRow(-1);
                  break;
                }
              case Qt::Key_Down:
                {
                  moveSymbolRow(2);
                  break;
                }
              case Qt::Key_C:
                {
                  copyFromCell();
                  break;
                }
              case Qt::Key_V:
                {
                  pasteToCell();
                  break;
                }
            }
          pKeyEvent->accept();
          return true;
        }
      else if (directTypingMode)
        {
          directType(pKeyEvent);
        }
      else
        {
          return QWidget::eventFilter(object, pKeyEvent);
        }
    }
  return false;
}

void ChildWidget::moveSymbolRow(int direction)
{
  QModelIndex index = selectionModel->currentIndex();

  // check if any row is selected
  if (index.row() < 0)
    return;

  //check where if we are if move is possible top/bottom
  if (direction < 0 && index.row() == 0)
    {
      return;
    }
  else if (direction > 0 && index.row() == (model->rowCount() - 1))
    {
      return;
    }
  else
    {
      // add new row
      model->insertRow(index.row() + direction);
      int newRow = index.row() + direction;
      //insertRow change row id!
      int currentRow = table->currentIndex().row();
      for (int i = 0; i < (model->columnCount() - 1); ++i)
        {
          model->setData(model->index(newRow, i), model->index(currentRow, i).data());
        }

      // activate new row
      table->setCurrentIndex(model->index(newRow, 0));
      //delete original row
      model->removeRow(currentRow);
      drawSelectionRects();
    }
}

void ChildWidget::copyFromCell()
{
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(table->currentIndex().data().toString());
}

void ChildWidget::pasteToCell()
{
  const QClipboard* clipboard = QApplication::clipboard();
  QModelIndex index = selectionModel->currentIndex();

  // do not paste string to int fields
  if ((index.column() > 0 && index.column() < 5) && (clipboard->text().toInt() > 0))
    model->setData(table->currentIndex(), clipboard->text().toInt());

  // paste string only to string field
  if (index.column() == 0)
    model->setData(table->currentIndex(), clipboard->text());
  drawSelectionRects();
}

void ChildWidget::directType(QKeyEvent* event)
{
  QModelIndex index = selectionModel->currentIndex();

  if (!event->text().isNull())
    // enter only text
    {
      if ((event->key() ==  Qt::Key_Enter) || (event->key() ==  Qt::Key_Return))
        {
          // enter/return just move to next row
          table->setCurrentIndex(model->index(index.row() + 1, 0));
        }
      else
        {
          model->setData(model->index(index.row(), 0, QModelIndex()), event->text());
          table->setCurrentIndex(model->index(index.row() + 1, 0));
        }
    }
  event->accept();
  drawSelectionRects();
}

void ChildWidget::deleteBoxes(const QList<QGraphicsItem*> &items)
{
  foreach(QGraphicsItem * item, items)
  {
    qint32 type = static_cast<qint32>(item->type());
    if (type == 3)   // delete only rectagles
      imageScene->removeItem(item);
  }
  setSelectionRect();   //initialize removed selection rectangle
  drawSelectionRects();
}

void ChildWidget::insertSymbol()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      int leftBorder = model->index(index.row(), 3).data().toInt() + 1;
      int rightBorder = model->index(index.row() + 1, 1).data().toInt() - 1;

      if (leftBorder > rightBorder)  //end of line or overlapping boxes
        rightBorder = leftBorder + (leftBorder - model->index(index.row(), 1).data().toInt());
      model->insertRow(index.row() + 1);
      model->setData(model->index(index.row() + 1, 0), "*");
      model->setData(model->index(index.row() + 1, 1), leftBorder);
      model->setData(model->index(index.row() + 1, 2), model->index(index.row(), 2).data().toInt());
      model->setData(model->index(index.row() + 1, 3), rightBorder);
      model->setData(model->index(index.row() + 1, 4), model->index(index.row(), 4).data().toInt());
      model->setData(model->index(index.row() + 1, 5), model->index(index.row(), 5).data().toInt());
      model->setData(model->index(index.row() + 1, 6), model->index(index.row(), 6).data().toBool());
      model->setData(model->index(index.row() + 1, 7), model->index(index.row(), 7).data().toBool());
      model->setData(model->index(index.row() + 1, 8), model->index(index.row(), 8).data().toBool());

      table->setCurrentIndex(model->index(index.row() + 1, 0));
      drawSelectionRects();
    }
}

void ChildWidget::splitSymbol()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      QModelIndex left = model->index(index.row(), 1);
      QModelIndex right = model->index(index.row(), 3);
      int width = right.data().toInt() - left.data().toInt();
      model->insertRow(index.row() + 1);
      model->setData(model->index(index.row() + 1, 0), "*");
      model->setData(model->index(index.row() + 1, 1), right.data().toInt() - width / 2);
      model->setData(model->index(index.row() + 1, 2), model->index(index.row(), 2).data().toInt());
      model->setData(model->index(index.row() + 1, 3), right.data().toInt());
      model->setData(model->index(index.row() + 1, 4), model->index(index.row(), 4).data().toInt());
      model->setData(model->index(index.row() + 1, 5), model->index(index.row(), 5).data().toInt());
      model->setData(model->index(index.row() + 1, 6), model->index(index.row(), 6).data().toBool());
      model->setData(model->index(index.row() + 1, 7), model->index(index.row(), 7).data().toBool());
      model->setData(model->index(index.row() + 1, 8), model->index(index.row(), 8).data().toBool());
      model->setData(right, right.data().toInt() - width / 2);
      drawSelectionRects();
    }
}

template<class T>
inline T min(T arg1, T arg2)
{
  return((arg1 < arg2) ? arg1 : arg2);
}

template<class T>
inline T max(T arg1, T arg2)
{
  return((arg1 > arg2) ? arg1 : arg2);
}

void ChildWidget::joinSymbol()
{
  QModelIndex index = selectionModel->currentIndex(), next = model->index(
                        index.row() + 1,
                        index.column());

  if (index.isValid() && next.isValid())
    {
      int row = index.row();
      model->setData(model->index(row, 0), model->index(row, 0).data().toString() + model->index(row
                     + 1, 0).data().toString());
      min(2, 3);
      model->setData(model->index(row, 1), min(model->index(row, 1).data().toInt(), model->index(row
                     + 1, 1).data().toInt()));
      model->setData(model->index(row, 2), max(model->index(row, 2).data().toInt(), model->index(row
                     + 1, 2).data().toInt()));
      model->setData(model->index(row, 3), max(model->index(row, 3).data().toInt(), model->index(row
                     + 1, 3).data().toInt()));
      model->setData(model->index(row, 4), min(model->index(row, 4).data().toInt(), model->index(row
                     + 1, 4).data().toInt()));
      model->setData(model->index(row, 5), min(model->index(row, 5).data().toInt(), model->index(row
                     + 1, 5).data().toInt()));
      model->setData(model->index(row, 6), model->index(row, 6).data().toBool() || model->index(row
                     + 1, 6).data().toBool());
      model->setData(model->index(row, 7), model->index(row, 7).data().toBool() || model->index(row
                     + 1, 7).data().toBool());
      model->setData(model->index(row, 8), model->index(row, 8).data().toBool() || model->index(row
                     + 1, 8).data().toBool());
      model->removeRow(row + 1);
      table->setCurrentIndex(model->index(row, 0));
      table->setFocus();
      drawSelectionRects();
    }
}

void ChildWidget::deleteSymbol()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      model->removeRow(index.row());
    }
}

void ChildWidget::moveUp()
{
  moveSymbolRow(-1);
}

void ChildWidget::moveDown()
{
  moveSymbolRow(2);
}

void ChildWidget::goToRow()
{
  GetRowIDDialog dialog(this);
  int row;
  if (dialog.exec())
    {
      QString string = dialog.lineEdit->text();
      row = string.toInt() - 1;
      if (row > model->rowCount())
        row = model->rowCount() - 1;
      table->setCurrentIndex(model->index(row, 0));
      table->setFocus();
      drawSelectionRects();
    }
}

QString ChildWidget::userFriendlyCurrentFile()
{
  return strippedName(boxFile);
}

QString ChildWidget::currentBoxFile()
{
  return QFileInfo(boxFile).canonicalFilePath();
}

void ChildWidget::documentWasModified()
{
  modified = true;
  emit modifiedChanged();
}

void ChildWidget::emitBoxChanged()
{
  emit boxChanged();
}

void ChildWidget::drawSelectionRects()
{
  QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

  if (!selectedIndexes.empty())
    {
      QModelIndex index = selectedIndexes.first();
      QString letter = model->index(index.row(), 0).data().toString();
      int left = model->index(index.row(), 1).data().toInt();
      int bottom = model->index(index.row(), 2).data().toInt();
      int right = model->index(index.row(), 3).data().toInt();
      int top = model->index(index.row(), 4).data().toInt();
      imageSelectionRect->setRect(QRectF(QPoint(left, top), QPointF(right, bottom)));
      imageSelectionRect->setVisible(true);
      imageView->ensureVisible(imageSelectionRect);

      if (symbolShown == true)
        {
          text2->setPlainText(letter);
          // TODO: get font metrics and calculate better placement
          // (e.g. visible in case of narrow margin)
          text2->setPos(QPoint(left, top - 16 * 2 - 15));
          text2->setVisible(true);
        }
      else
        {
          text2->setVisible(false);
        }
    }
  else
    {
      imageSelectionRect->setVisible(false);
      text2->setVisible(false);
    }
}

void ChildWidget::closeEvent(QCloseEvent* event)
{
  if (!maybeSave())
    {
      event->ignore();
    }
}

bool ChildWidget::maybeSave()
{
  if (isModified())
    {
      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning(this, SETTING_APPLICATION, tr("'%1' has been modified.\n"
                                 "Do you want to save your changes?").arg(userFriendlyCurrentFile()), QMessageBox::Save
                                 | QMessageBox::Discard | QMessageBox::Cancel);
      if (ret == QMessageBox::Save)
        return save(boxFile);
      else if (ret == QMessageBox::Cancel)
        return false;
    }
  return true;
}

void ChildWidget::setCurrentImageFile(const QString& fileName)
{
  imageFile = QFileInfo(fileName).canonicalFilePath();
}

void ChildWidget::setCurrentBoxFile(const QString& fileName)
{
  boxFile = QFileInfo(fileName).canonicalFilePath();
}

QString ChildWidget::strippedName(const QString& fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}
