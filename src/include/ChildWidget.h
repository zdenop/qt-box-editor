/**********************************************************************
 * File:        ChildWidget.h
 * Description: ChildWidget functions
 * Author:      Marcel Kolodziejczyk
 * Created:     2010-01-06
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

#ifndef CHILDWIDGET_H_
#define CHILDWIDGET_H_

#include <QStandardItemModel>
#include <QTableView>
#include <QAbstractItemView>
#include <QGraphicsScene>
#include <QHeaderView>
#include <QGraphicsView>
#include <QPixmap>
#include <QGraphicsRectItem>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QApplication>
#include <QDebug>
#include <QCloseEvent>
#include <QSplitter>
#include <QMessageBox>

class QGraphicsScene;
class QGraphicsView;
class QAbstractItemModel;
class QItemSelectionModel;
class QTableView;
class QGraphicsItem;
class QGraphicsRectItem;

class ChildWidget : public QSplitter
{
  Q_OBJECT

public:
  ChildWidget(QWidget * parent = 0);

  bool isModified() { return modified; }
  bool isBoxSelected();
  bool isBold();
  bool isItalic();
  bool isUnderLine();

  QString userFriendlyCurrentFile();
  QString currentBoxFile();
  QString canonicalImageFileName() { return imageFile; }

  bool save(const QString &fileName);
  bool loadImage(const QString &fileName);
  bool loadBoxes(const QString &fileName);
  void setBolded(bool v);
  void setItalic(bool v);
  void setUnderline(bool v);
  void zoomIn();
  void zoomOut();
  void splitSymbol();
  void joinSymbol();
  void deleteSymbol();

private slots:
  void documentWasModified();
  void emitBoxChanged();
  void drawSelectionRects();

protected:
  void closeEvent(QCloseEvent *event);
  bool maybeSave();
  void setCurrentImageFile(const QString &fileName);
  void setCurrentBoxFile(const QString &fileName);

  QString strippedName(const QString &fullFileName);

signals:
  void boxChanged();
  void modifiedChanged();

protected:
  QGraphicsScene *imageScene;
  QGraphicsView *imageView;
  QGraphicsItem * imageItem;
  QGraphicsRectItem *imageSelectionRect;

  QTableView *table;

  QAbstractItemModel *model;
  QItemSelectionModel *selectionModel;

  QString imageFile;
  QString boxFile;

  bool modified;
  int imageHeight;
};

#endif /* CHILDWIDGET_H_ */
