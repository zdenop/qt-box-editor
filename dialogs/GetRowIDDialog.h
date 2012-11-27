/**********************************************************************
* File:        GetRowIDDialog.h
* Description: Get Row ID Dialog
* Author:      Zdenko Podobny
* Created:     2011-04-14
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

#ifndef DIALOGS_GETROWIDDIALOG_H_
#define DIALOGS_GETROWIDDIALOG_H_

#include <QtGui>
#include <QDialog>
#include <QPushButton>

#include "ui_GetRowIDDialog.h"

class GetRowIDDialog : public QDialog, public Ui::GetRowIDDialog {
  Q_OBJECT

  public:
    explicit GetRowIDDialog(QWidget* parent = 0);

  private slots:
    void on_lineEdit_textChanged();
};


#endif  // DIALOGS_GETROWIDDIALOG_H_
