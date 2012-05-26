/**********************************************************************
* File:        DelegateEditors.h
* Description: SpinBoxDelegate that export signals of SpinBox to QTableView
               Created based on Nokia Qt\4.7.3\examples\itemviews\
               spinboxdelegate
* Author:      Zdenko Podobny
* Created:     2011-07-06
*
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

#ifndef SRC_INCLUDE_DELEGATEEDITORS_H_
#define SRC_INCLUDE_DELEGATEEDITORS_H_

#include <QtCore/QModelIndex>
#include <QtCore/QObject>
#include <QtCore/QSize>
#include <QtGui/QItemDelegate>
#include <QtGui/QSpinBox>

class SpinBoxDelegate : public QItemDelegate {
  Q_OBJECT

  public:
    explicit SpinBoxDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &index) const;

  signals:
    void sbd_valueChanged(int sbdValue);
    void sbd_editingFinished();
};

class CheckboxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    CheckboxDelegate(QObject* parent = 0);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                             const QModelIndex& index);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

signals:
  void toggled(bool checked);
};

#endif  // SRC_INCLUDE_DELEGATEEDITORS_H_
