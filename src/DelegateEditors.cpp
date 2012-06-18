/**********************************************************************
* File:        DelegateEditors.cpp
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

#include "include/DelegateEditors.h"
#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>

/********************************
 * SpinBoxDelegate
 ********************************/
SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
  : QItemDelegate(parent) {
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &/* index */) const {
  QSpinBox *editor = new QSpinBox(parent);
  editor->setRange(0, 1000000);
  connect(editor, SIGNAL(valueChanged(int)), SIGNAL(sbd_valueChanged(int)));
  connect(editor, SIGNAL(editingFinished()), SIGNAL(sbd_editingFinished()));
  return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const {
  int value = index.model()->data(index, Qt::EditRole).toInt();

  QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
  spinBox->setValue(value);
}
void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const {
  QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
  spinBox->interpretText();
  int value = spinBox->value();
  model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const {
  editor->setGeometry(option.rect);
}

/********************************
 * CheckboxDelegate
 ********************************/
CheckboxDelegate::CheckboxDelegate(QObject* parent)
  : QItemDelegate(parent) {
}

QWidget* CheckboxDelegate::createEditor(QWidget* /*parent*/, const QStyleOptionViewItem& /*option*/,
                                        const QModelIndex& /*index*/) const {
  // NOTE: This prevents standard bool editor from appearing on double click
  return NULL;
}

void CheckboxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  bool value = index.data(Qt::EditRole).toBool();
  QSize size = check(option, option.rect, Qt::Checked).size();
  QRect checkboxRect = QStyle::alignedRect(option.direction, Qt::AlignCenter, size, option.rect);
  drawCheck(painter, option, checkboxRect, (value ? Qt::Checked : Qt::Unchecked));
}

bool CheckboxDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) {
  bool value = index.data(Qt::EditRole).toBool();

  if (event->type() == QEvent::MouseButtonRelease) {
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                          option.decorationSize,
                                          QRect(option.rect.x() + (2 * textMargin), option.rect.y(),
                                              option.rect.width() - (2 * textMargin),
                                              option.rect.height()));
    if (!checkRect.contains(static_cast<QMouseEvent*>(event)->pos()))
      return false;
  } else if (event->type() == QEvent::KeyPress) {
    if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
      return false;
  } else {
    return false;
  }

  value = !value;
  emit toggled(value, index.column());

  return model->setData(index, value, Qt::EditRole);
}
