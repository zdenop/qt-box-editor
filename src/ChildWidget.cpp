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
#include "Settings.h"
#include "SettingsDialog.h"

ChildWidget::ChildWidget(QWidget * parent) :
  QSplitter(Qt::Horizontal, parent)
{
  model = new QStandardItemModel(0, 8, this);
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
  selectionModel = new QItemSelectionModel(model);
  connect(
    selectionModel,
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this, SLOT(emitBoxChanged()));
  connect(
    selectionModel,
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this, SLOT(drawSelectionRects()));
  table->setSelectionModel(selectionModel);
  table->verticalHeader()->hide();
  table->setSelectionBehavior(QAbstractItemView::SelectItems);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->hideColumn(5);
  table->hideColumn(6);
  table->hideColumn(7);
  table->hideColumn(8);

  // Font for table
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);
  tableFont = settings.value("GUI/Font").value<QFont>();
  if (tableFont.rawName().isEmpty())
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

  // Make graphics Scene and View
  imageScene = new QGraphicsScene;
  imageView = new QGraphicsView(imageScene);
  imageView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  // splitter
  addWidget(table);
  addWidget(imageView);
  setStretchFactor(0, 0);
  setStretchFactor(1, 1);

  setSelectionRect();
  modified = false;
  boxesVisible = false;
  ToSelection = false;
}

bool ChildWidget::loadImage(const QString &fileName)
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
      connect(model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(emitBoxChanged()));
      connect(model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(documentWasModified()));
    }
  else
    return false;

  return true;
}

bool ChildWidget::loadBoxes(const QString &fileName)
{
  QFile file(fileName);

  if (!file.open(QFile::ReadOnly | QFile::Text))
    {
      QMessageBox::warning(this, tr("MDI"), tr("Cannot read file %1:\n%2.").arg(fileName).arg(
                             file.errorString()));
      return false;
    }

  QTextStream in(&file);
  in.setCodec("UTF-8");
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString line;
  int row = 0;
  do
    {
      line = in.readLine();
      if (!line.isEmpty())
        {
          model->insertRow(row);
          QStringList pieces = line.split(" ", QString::SkipEmptyParts);
          QString letter = pieces.value(0);
          bool bold = false, italic = false, underline = false;
          if (letter.at(0) == '@')
            {
              bold = true;
              letter.remove(0, 1);
            }
          if (letter.at(0) == '$')
            {
              italic = true;
              letter.remove(0, 1);
            }
          if (letter.at(0) == '\'')
            {
              underline = true;
              letter.remove(0, 1);
            }
          int left = pieces.value(1).toInt();
          int bottom = imageHeight - pieces.value(2).toInt();
          int right = pieces.value(3).toInt();
          int top = imageHeight - pieces.value(4).toInt();
          int page = pieces.value(5).toInt();
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
    } while (!line.isEmpty());
  table->resizeColumnsToContents();
  file.close();
  QApplication::restoreOverrideCursor();

  setCurrentBoxFile(fileName);

  return true;
}

bool ChildWidget::save(const QString &fileName)
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

bool ChildWidget::isBold()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      return model->index(index.row(), 6).data().toBool();
    }
  return false;
}

bool ChildWidget::isItalic()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      return model->index(index.row(), 5).data().toBool();
    }
  return false;
}

bool ChildWidget::isUnderLine()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      return model->index(index.row(), 7).data().toBool();
    }
  return false;
}

void ChildWidget::setBolded(bool v)
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      model->setData(model->index(index.row(), 6, QModelIndex()), v);
    }
}

void ChildWidget::setSelectionRect()
{
  imageSelectionRect = imageScene->addRect(0, 0, 0, 0, QPen(rectColor), rectFillColor);
  imageSelectionRect->setZValue(1);
}

void ChildWidget::setItalic(bool v)
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      model->setData(model->index(index.row(), 5, QModelIndex()), v);
    }
}

void ChildWidget::setUnderline(bool v)
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      model->setData(model->index(index.row(), 7, QModelIndex()), v);
    }
}

void ChildWidget::setZoom(float scale)
{
  QTransform transform;

  transform.scale(scale, scale);
  imageView->setTransform(transform);
  // qreal scaleFactor = transform.m11();
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
  if (ToSelection == false)
    {
      imageView->fitInView(imageSelectionRect, Qt::KeepAspectRatio);
      imageView->scale(1 / 1.1, 1 / 1.1);    // make small border
      imageView->ensureVisible(imageSelectionRect);
      imageView->centerOn(imageSelectionRect);
      ToSelection = true;
    }
  else
    {
      // Lets keep zoom factor
      ToSelection = false;
    }
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

void ChildWidget::deleteBoxes(const QList<QGraphicsItem*> &items)
{
  foreach (QGraphicsItem *item, items)
    {
      qint32 type = static_cast<qint32>(item->type());
      if (type == 3)   // delete only rectagles
        imageScene->removeItem(item);
    }
  setSelectionRect();   //initialize removed selection rectangle
  drawSelectionRects();
}

void ChildWidget::splitSymbol()
{
  QModelIndex index = selectionModel->currentIndex();

  if (index.isValid())
    {
      QModelIndex left = model->index(index.row(), 1);
      QModelIndex right = model->index(index.row(), 3);
      QModelIndex page = model->index(index.row(), 5);
      int width = right.data().toInt() - left.data().toInt();
      model->insertRow(index.row() + 1);
      model->setData(model->index(index.row() + 1, 0), "*");
      model->setData(model->index(index.row() + 1, 1), right.data().toInt() - width / 2);
      model->setData(model->index(index.row() + 1, 2), model->index(index.row(), 2).data().toInt());
      model->setData(model->index(index.row() + 1, 3), right.data().toInt());
      model->setData(model->index(index.row() + 1, 4), model->index(index.row(), 4).data().toInt());
      model->setData(model->index(index.row() + 1, 5), page.data().toInt());
      model->setData(model->index(index.row() + 1, 6), model->index(index.row(), 5).data().toBool());
      model->setData(model->index(index.row() + 1, 7), model->index(index.row(), 6).data().toBool());
      model->setData(model->index(index.row() + 1, 8), model->index(index.row(), 7).data().toBool());
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
      int left = model->index(index.row(), 1).data().toInt();
      int bottom = model->index(index.row(), 2).data().toInt();
      int right = model->index(index.row(), 3).data().toInt();
      int top = model->index(index.row(), 4).data().toInt();
      //int page = model->index(index.row(), 5).data().toInt(); //not used for the moment
      imageSelectionRect->setRect(QRectF(QPoint(left, top), QPointF(right, bottom)));
      imageSelectionRect->setVisible(true);
      imageView->ensureVisible(imageSelectionRect);
    }
  else
    {
      imageSelectionRect->setVisible(false);
    }
  if (ToSelection == true)   //if zoomToSelection is enabled, than keep zooming to selection
    {
      ToSelection = false;
      zoomToSelection();
    }
}

void ChildWidget::closeEvent(QCloseEvent *event)
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
      ret = QMessageBox::warning(this, tr("MDI"), tr("'%1' has been modified.\n"
                                                     "Do you want to save your changes?").arg(userFriendlyCurrentFile()), QMessageBox::Save
                                 | QMessageBox::Discard | QMessageBox::Cancel);
      if (ret == QMessageBox::Save)
        return save(boxFile);
      else if (ret == QMessageBox::Cancel)
        return false;
    }
  return true;
}

void ChildWidget::setCurrentImageFile(const QString &fileName)
{
  imageFile = QFileInfo(fileName).canonicalFilePath();
}

void ChildWidget::setCurrentBoxFile(const QString &fileName)
{
  boxFile = QFileInfo(fileName).canonicalFilePath();
}

QString ChildWidget::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}
