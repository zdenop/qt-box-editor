/**********************************************************************
* File:        GetRowIDDialog.cpp
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

#include "dialogs/GetRowIDDialog.h"

GetRowIDDialog::GetRowIDDialog(QWidget* parent)
  : QDialog(parent) {
  setupUi(this);
  buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  QRegExp regExp("^\\d\\d*$");
  lineEdit->setValidator(new QRegExpValidator(regExp, this));

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void GetRowIDDialog::on_lineEdit_textChanged() {
  buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
    lineEdit->hasAcceptableInput());
}

