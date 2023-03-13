/**********************************************************************
* File:        FindDialog.cpp
* Description: Find Dialog
* Author:      Zdenko Podobny
* Created:     2011-09-23
*
* (C) Copyright 2011-2014, Zdenko Podobny
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
#include "Settings.h"

#if (!defined(_WIN32))
#include <unistd.h>
#endif

#include <QDebug>

FindDialog::FindDialog(QWidget* parent, QString title)
  : QDialog(parent) {
  setupUi(this);

  if (!title.isEmpty())
      setWindowTitle(tr("Find in %1:").arg(title));
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

  QRegExp regExp(".+");  // no empty string test
  lineEdit->setValidator(new QRegExpValidator(regExp, this));

  connect(findNextButton, SIGNAL(clicked()), this, SLOT(findNext()));
  connect(findPrevButton, SIGNAL(clicked()), this, SLOT(findPrev()));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(checkBox_Mc, SIGNAL(toggled(bool)), this, SLOT(changed_Mc(bool)));
  connect(parent, SIGNAL(blinkFindDialog()), this, SLOT(blinkFindDialog()));
  timerBlink = new QTimeLine(10);
  originalBackColor = this->palette().color(QPalette::Window);;
  getSettings();
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

void FindDialog::changed_Mc(bool status) {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  settings.setValue("Find/MatchCase", status);
}

void FindDialog::blinkFindDialog() {
  QApplication::beep();
  if(timerBlink->state() == QTimeLine::NotRunning)
    {
    timerBlink->resume();
    connect(timerBlink, SIGNAL(finished()), this, SLOT(blinkFinished()));
    }
  QPalette pal = this->palette();
  pal.setColor(QPalette::Window, Qt::red);
  this->setPalette(pal);
}

void FindDialog::blinkFinished() {
  disconnect(timerBlink, SIGNAL(finished()), this, SLOT(blinkFinished()));
  QPalette pal = this->palette();
  pal.setColor(QPalette::Window, originalBackColor);
  this->setPalette(pal);
}

void FindDialog::closeEvent(QCloseEvent* event) {
    writeGeometry();
    event->accept();
}

void FindDialog::getSettings() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  if (settings.contains("Find/MatchCase"))
    checkBox_Mc->setChecked(settings.value("Find/MatchCase").toBool());

  QPoint pos = settings.value("Find/Pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("Find/Size", QSize(300, 100)).toSize();
  resize(size);
  move(pos);
}

void FindDialog::writeGeometry() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);
  settings.setValue("Find/Pos", pos());
  settings.setValue("Find/Size", size());
}
