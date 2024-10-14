/**********************************************************************
* File:        ShortCutsDialog.h
* Description: Dialog for shorcuts
* Author:      Zdenko Podobny
* Created:     2011-03-21
*
* (C) Copyright 2011-2024, Zdenko Podobny
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

#ifndef DIALOGS_SHORTCUTSDIALOG_H_
#define DIALOGS_SHORTCUTSDIALOG_H_

#include <QDialog>
#include "ui_ShortCutDialog.h"

class ShortCutsDialog : public QDialog, public Ui::ShortCutDialog {
  Q_OBJECT

  public:
    explicit ShortCutsDialog(QWidget* = 0);
};

#endif  // DIALOGS_SHORTCUTSDIALOG_H_
