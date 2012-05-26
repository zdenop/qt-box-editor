/**********************************************************************
* File:                ChildWidget.h
* Description: ChildWidget functions
* Author:            Marcel Kolodziejczyk
* Created:         2010-01-06
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
**        http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*
**********************************************************************/

#ifndef SRC_INCLUDE_CHILDWIDGET_H_
#define SRC_INCLUDE_CHILDWIDGET_H_

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtCore/qmath.h>
#include <QtCore/QStack>
#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QCloseEvent>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>
#include <QtGui/QSplitter>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTableView>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QTransform>

class QGraphicsScene;
class QGraphicsView;
class QAbstractItemModel;
class QItemSelectionModel;
class QTableView;
class QGraphicsItem;
class QGraphicsRectItem;
class FindDialog;
class DrawRectangle;

enum undoOperation
{
    euoAdd=1,
    euoDelete=2,
    euoChange=4,
    euoJoin=8,
    euoSplit=16,
    euoReplace=32
};

struct UndoItem
{
    undoOperation m_eop;
    int m_origrow;
    int m_extrarow;
    QVariant m_vdata[9];
    QVariant m_vextradata[9];
};

// Overhead symbol displayed in Show symbol mode
struct BalloonSymbol
{
    // Symbol itself
    QGraphicsTextItem* symbol;

    // Halo components
    static const int haloCompCount = 4;
    QGraphicsTextItem* halo[haloCompCount];
};

class ChildWidget : public QSplitter {
    Q_OBJECT

  public:
    explicit ChildWidget(QWidget* parent = 0);

    bool isModified() {
        return modified;
    }
    bool isBoxSelected();
    bool isUndoAvailable();
    bool isBold();
    bool isItalic();
    bool isUnderLine();
    bool isShowSymbol();
    bool isDirectTypingMode();
    bool isFontColumnsShown();
    bool isDrawBoxes();
    bool isDrawRect();

    QString userFriendlyCurrentFile();
    QString getSymbolHexCode();
    QString getBoxSize();
    QString currentBoxFile();
    QString canonicalImageFileName() {
        return imageFile;
    }

    bool save(const QString& fileName);
    bool splitToFeatureBF(const QString& fileName);
    bool saveString(const QString& fileName, const QString& qData);
    bool importSPLToChild(const QString& fileName);
    bool importTextToChild(const QString& fileName);
    bool exportTxt(const int& eType, const QString& fileName);
    bool loadImage(const QString& fileName);
    bool loadBoxes(const QString& fileName);
    bool qCreateBoxes(const QString &boxFileName, const QImage &img);
    bool fillTableData(QTextStream& boxdata);
    void setSelectionRect();
    void setBolded(bool v);
    void setItalic(bool v);
    void setUnderline(bool v);
    void getZoom();
    void setZoom(float scale);
    void setDirectTypingMode(bool v);
    void setShowFontColumns(bool v);
    void zoomOriginal();
    void zoomIn();
    void zoomOut();
    void zoomToSelection();
    void zoomToFit();
    void zoomToHeight();
    void zoomToWidth();
    void showSymbol();
    void drawBoxes();
    void copyFromCell();
    void pasteToCell();
    void drawRectangle(bool checked);

  public slots:
    void sbValueChanged(int sbdValue);
    void insertSymbol();
    void splitSymbol();
    void joinSymbol();
    void deleteSymbol();
    void undo();
    void cbFontToggleProxy(bool checked);

    void moveUp();
    void moveDown();
    void moveTo();
    void goToRow();
    void find();
    void findNext(const QString &symbol, Qt::CaseSensitivity mc);
    void findPrev(const QString &symbol, Qt::CaseSensitivity mc);

  private:
    void undoDelete(UndoItem& ui);
    void undoAdd(UndoItem& ui);
    void undoEdit(UndoItem& ui);
    void undoJoin(UndoItem& ui);
    void undoSplit(UndoItem& ui);
    void undoMoveBack(UndoItem& ui);
    bool symbolShown;
    bool boxesVisible;
    bool drawnRectangle;
    bool directTypingMode;

    void clearBalloons();
    void updateBalloons();

  private slots:
    void documentWasModified();
    void emitBoxChanged();
    void drawSelectionRects();

  private:
    QColor rectColor;
    QColor rectFillColor;
    QColor boxColor;
    QColor backgroundColor;
    QGraphicsItem* m_message;
    FindDialog *f_dialog;
    DrawRectangle *m_DrawRectangle;

    void moveSymbolRow(int direction);
    QList<QTableWidgetItem*> takeRow(int row);
    void removeMyItems(QVector<QGraphicsRectItem *> &graphicsItems);

  protected:

    void directType(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    bool eventFilter(QObject* object, QEvent* event);
    void closeEvent(QCloseEvent* event);
    bool maybeSave();
    void setCurrentImageFile(const QString& fileName);
    void setCurrentBoxFile(const QString& fileName);

    QString strippedName(const QString& fullFileName);

  signals:
    void boxChanged();
    void modifiedChanged();
    void zoomRatioChanged(qreal);

  protected:
    QGraphicsScene* imageScene;
    QGraphicsView* imageView;
    QGraphicsItem* imageItem;
    QGraphicsRectItem* rectangle;
    QVector<QGraphicsRectItem *> rectItem;
    QVector<QGraphicsRectItem *> boxesItem;

    QTableView* table;

    QAbstractItemModel* model;
    QItemSelectionModel* selectionModel;

    QString imageFile;
    QString boxFile;

    bool modified;
    int imageHeight;
    int imageWidth;
    int widgetWidth;

    QStack<UndoItem> m_undostack;

    // Overhead symbols
    // NOTE: Temp const, to be replaced by user-adjusted setting
    static const int balloonCount = 13;
    QVector<BalloonSymbol> balloons;
};

#endif  // SRC_INCLUDE_CHILDWIDGET_H_
