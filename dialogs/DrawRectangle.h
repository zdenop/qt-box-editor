/**********************************************************************
* File:        FindDialog.h
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

#ifndef DIALOGS_DrawRectangle_H_
#define DIALOGS_DrawRectangle_H_

#include <QDialog>
#include <QRect>
#include <QLineEdit>

namespace Ui {
    class DrawRect;
}

class DrawRectangle : public QDialog {
  Q_OBJECT

public:
    DrawRectangle(QWidget* parent = 0, QString title = "", int maxWidth = 0,
                  int maxHeight = 0);
    ~DrawRectangle();

    QRect getRectangle() const;

private:
    Ui::DrawRect *ui;
    QLineEdit *edtTesseractOutput;
};


#endif  // DIALOGS_DrawRectangle_H_
