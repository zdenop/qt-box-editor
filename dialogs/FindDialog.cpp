/**********************************************************************
* File:        FindDialog.cpp
* Description: Find Dialog
* Author:      Zdenko Podobny
* Created:     2011-09-23
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

#include "dialogs/FindDialog.h"

FindDialog::FindDialog(QWidget* parent)
  : QDialog(parent) {
  setupUi(this);

  findNextButton = new QPushButton(tr("&Next"));
  findNextButton->setDefault(true);
  findNextButton->setEnabled(false);
  findPrevButton = new QPushButton(tr("&Previous"));
  findPrevButton->setEnabled(false);
  closeButton = new QPushButton(tr("&Close"));

  buttonBox->addButton(findNextButton,
                       QDialogButtonBox::ActionRole);
  buttonBox->addButton(findPrevButton,
                       QDialogButtonBox::ActionRole);
  buttonBox->addButton(closeButton,
                       QDialogButtonBox::RejectRole);

  QRegExp regExp(".");
  lineEdit->setValidator(new QRegExpValidator(regExp, this));

  connect(findNextButton, SIGNAL(clicked()), this, SLOT(findNext()));
  connect(findPrevButton, SIGNAL(clicked()), this, SLOT(findPrev()));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

void FindDialog::on_lineEdit_textChanged() {
  findNextButton->setEnabled(lineEdit->hasAcceptableInput());
  findPrevButton->setEnabled(lineEdit->hasAcceptableInput());
}

void FindDialog::findNext() {
  QString symbol = lineEdit->text();
  Qt::CaseSensitivity mc =
    checkBox_Mc->isChecked() ? Qt::CaseSensitive
    : Qt::CaseInsensitive;
  emit findNext(symbol, mc);
}

void FindDialog::findPrev() {
  QString symbol = lineEdit->text();
  Qt::CaseSensitivity mc =
    checkBox_Mc->isChecked() ? Qt::CaseSensitive
    : Qt::CaseInsensitive;
  emit findPrev(symbol, mc);
}
