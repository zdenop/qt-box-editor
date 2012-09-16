/**********************************************************************
* File:        ChildWidget.h
* Description: ChildWidget functions
* Author:      Marcel Kolodziejczyk
* Created:     2010-01-06
*
* (C) Copyright 2010, Marcel Kolodziejczyk
* (C) Copyright 2011-2012, Zdenko Podobny
* (C) Copyright 2012, Zohar Gofer
* (C) Copyright 2012, Dmitri Silaev
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**                http://www.apache.org/licenses/LICENSE-2.0
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
#include <QtCore/QFileSystemWatcher>
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
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QGraphicsView>
#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>
#include <QtGui/QRubberBand>
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

enum undoOperation {
    euoAdd=1,
    euoDelete=2,
    euoChange=4,
    euoJoin=8,
    euoSplit=16,
    euoReplace=32
};

struct UndoItem {
    undoOperation m_eop;
    int m_origrow;
    int m_extrarow;
    QVariant m_vdata[9];
    QVariant m_vextradata[9];
};

// Overhead symbol displayed in Show symbol mode
struct BalloonSymbol {
    // Symbol itself
    QGraphicsTextItem* symbol;

    // Halo components
    static const int haloCompCount = 8;
    // TODO(all): Temp const, to be replaced by user-adjusted setting
    static const int haloShift = 2;
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
    bool isRedoAvailable();
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
    void setZoomStatus();
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
    void readSettings();

  public slots:
    void updateColWidthsOnSplitter(int pos, int index);

    void letterStartEdit();
    void letterEditFinished();
    void sbValueChanged(int sbdValue);
    void sbFinished();
    void insertSymbol();
    void splitSymbol();
    void joinSymbol();
    void deleteSymbol();
    void undo();
    void redo();
    void cbFontToggleProxy(bool checked, int column);

    void moveUp();
    void moveDown();
    void moveTo();
    void goToRow();
    void find();
    void findNext(const QString &symbol, Qt::CaseSensitivity mc);
    void findPrev(const QString &symbol, Qt::CaseSensitivity mc);

  private:
    void initTable();
    void deleteSymbolByRow(int row);
    void undoDelete(UndoItem& ui, bool bIsRedo = false);
    void undoAdd(UndoItem& ui, bool bIsRedo = false);
    void undoEdit(UndoItem& ui, bool bIsRedo = false);
    void undoJoin(UndoItem& ui, bool bIsRedo = false);
    void undoSplit(UndoItem& ui, bool bIsRedo = false);
    void undoMoveBack(UndoItem& ui, bool bIsRedo = false);
    bool symbolShown;
    bool boxesVisible;
    bool drawnRectangle;
    bool directTypingMode;

    void clearBalloons();
    void updateBalloons();

    int fontOffset;
    QColor rectColor;
    QColor rectFillColor;
    QColor boxColor;
    QColor backgroundColor;
    QColor imageFontColor;
    QGraphicsItem* m_message;
    FindDialog *f_dialog;
    DrawRectangle *m_DrawRectangle;
    QFileSystemWatcher *fileWatcher;
    void setFileWatcher(const QString & fileName);

    void moveSymbolRow(int direction);
    QList<QTableWidgetItem*> takeRow(int row);
    void calculateTableWidth();

  private slots:
    void documentWasModified();
    void emitBoxChanged();
    void selectionChanged(const QItemSelection& selected,
                          const QItemSelection& deselected);
    void drawSelectionRects();
    void slotfileChanged(const QString& fileName);

  signals:
    void boxChanged();
    void modifiedChanged();
    void zoomRatioChanged(qreal);

  protected:
    void directType(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    bool eventFilter(QObject* object, QEvent* event);
    void closeEvent(QCloseEvent* event);
    bool maybeSave();
    void setCurrentImageFile(const QString& fileName);
    void setCurrentBoxFile(const QString& fileName);

    QString strippedName(const QString& fullFileName);

    QGraphicsScene* imageScene;
    QGraphicsView* imageView;
    QGraphicsItem* imageItem;
    QGraphicsRectItem* rectangle;

    // Returns model item's associated bbox. "row" determines item's row number.
    // If row = -1 then returns bbox of the last item in current selection
    QGraphicsRectItem* modelItemBox(int row = -1);
    QGraphicsRectItem* createModelItemBox(int row);
    void updateModelItemBox(int row);
    void deleteModelItemBox(int row);

    QTableView* table;

    QStandardItemModel* model;
    QItemSelectionModel* selectionModel;

    QString imageFile;
    QString boxFile;

    bool modified;
    int imageHeight;
    int imageWidth;
    int widgetWidth;

    QFont m_imageFont;

    // Overhead symbols
    int balloonCount;
    QVector<BalloonSymbol> balloons;

    QRubberBand* rubberBand;
    QPoint rbOrigin;

    template<class T>
    class UndoStack : public QStack<T> {
      public:
        void SetRedoStack(QStack<T>* pRedoStack) {
            m_pRedoStack = pRedoStack;
        }

        //Overide
        void push(const T &t, bool bClearRedo = true) {
            //When a new undo item is push we need
            //To clear the redo stack.
            if (m_pRedoStack && bClearRedo) {
                m_pRedoStack->clear();
            }
            //Now call base class implementation
            QStack<T>::push(t);
        }

      private:
        QStack<T>*  m_pRedoStack;
    };

    UndoStack<UndoItem> m_undostack;
    QStack<UndoItem> m_redostack;
    bool bIsSpinBoxChanged;
    bool bIsLineEditChanged;

};

#endif   // SRC_INCLUDE_CHILDWIDGET_H_
