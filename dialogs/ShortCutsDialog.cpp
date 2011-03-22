/**********************************************************************
* File:        ShortCutsDialog.cpp
* Description: Dialog for shorcuts
* Author:      Zdenko Podobny
* Created:     2011-03-21
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

#include "ShortCutsDialog.h"

ShortCutsDialog::ShortCutsDialog(QWidget* parent)
  : QDialog(parent)
{
  setupUi(this);
  textEdit -> setHtml(tr(
    "<h1>Editor shortcuts:</h1>"
    "<p><b>F1</b> — this help<br/>"
    "<b>Ins</b> — insert symbol below current symbol<br/>"
    "<b>Del</b> — delete current symbol<br/>"
    "<b>CTRL + 1</b> — join current symbol with next symbol<br/>"
    "<b>CTRL + 2</b> — split current symbol to 2 symbols<br/>"

    "<b>CTRL + B</b> — set symbol as <b>bold</b><br/>"
    "<b>CTRL + I</b> — set symbol as <i>italic</i><br/>"
    "<b>CTRL + U</b> — set symbol as <u>undelined</u><br/>"

    "<b>CTRL + H</b> — display all symbols on image<br/>"

    "<b>CTRL + +/-</b> — zoom image<br/>"
    "<b>CTRL + *</b> — zoom image to original scale<br/>"
    "<b>CTRL + .</b> — zoom image to fit whole image in current view<br/>"
    "<b>CTRL + &gt;</b> — zoom image to fit height of image in current view<br/>"
    "<b>CTRL + &lt;</b> — zoom image to fit width of image in current view<br/>"
    "<b>CTRL + /</b> — zoom current box to fit in current view<br/>"

    "<b>CTRL + arrows up/down</b> — move symbol up/down in table<br/>"
    "<b>CTRL + T</b> — setting dialog<br/>"

    "</p>"));
}
