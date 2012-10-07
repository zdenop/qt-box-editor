/**********************************************************************
* File:        ShortCutsDialog.cpp
* Description: Dialog for shorcuts
* Author:      Zdenko Podobny
* Created:     2011-03-21
*
* (C) Copyright 2011-2012, Zdenko Podobny
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

#include "dialogs/ShortCutsDialog.h"

ShortCutsDialog::ShortCutsDialog(QWidget* parent)
  : QDialog(parent) {
  setupUi(this);
  textEdit -> setHtml(tr(
                        "<h1>Editor shortcuts</h1>"
                        "<p><b>F1</b> — this help<br/>"
                        "<b>Ins</b> — insert symbol below current symbol<br/>"
                        "<b>Del</b> — delete current symbol<br/>"
                        "<b>CTRL + 1</b> — join current symbol with next symbol"
                        "<br/>"
                        "<b>CTRL + 2</b> — split current symbol to 2 symbols"
                        "<br/>"
                        "<br/>"
                        "<b>CTRL + B</b> — set symbol as <b>bold</b><br/>"
                        "<b>CTRL + I</b> — set symbol as <i>italic</i><br/>"
                        "<b>CTRL + U</b> — set symbol as <u>underlined</u><br/>"
                        "<br/>"
                        "<b>CTRL + Z</b> — undo last change<br/>"
                        "<b>CTRL + Y</b> — redo last undo<br/>"
                        "<br/>"
                        "<b>CTRL + +/-</b> — zoom image<br/>"
                        "<b>CTRL + *</b> — zoom image to original scale<br/>"
                        "<b>CTRL + .</b> — zoom image to fit whole image in "
                        "current view<br/>"
                        "<b>CTRL + &gt;</b> — zoom image to fit height of "
                        "image in current view"
                        "<br/>"
                        "<b>CTRL + &lt;</b> — zoom image to fit width of image"
                        " in current view<br/>"
                        "<b>CTRL + /</b> — zoom current box to fit in current"
                        " view<br/>"
                        "<br/>"
                        "<b>CTRL + D</b> — direct type mode<br/>"
                        "<br/>"
                        "<b>CTRL + Arrows up/down</b> — move symbol up/down in"
                            " table<br/>"
                        "<b>CTRL + G</b> — go to row id…<br/>"
                        "<b>CTRL + M</b> — Move row to row id…<br/>"
                        "<b>CTRL + T</b> — setting dialog<br/>"
                        "<br/>"
                        "<b>CTRL + F</b> — find symbol<br/>"
                        "<br/>"
                        "<b>CTRL + L</b> — show/hide balloon symbols on "
                            "image<br/>"
                        "<b>CTRL + H</b> — show/hide all boxes on image<br/>"
                        "<b>CTRL + R</b> — draw/hide rectangle on image<br/>"
                        "<b>Arrows</b> <i>in table area</i> — move "
                        "focus/selection<br/>"
                        "<b>Arrows</b> <i>in image area</i> — scroll image<br/>"
                        "</p>"
                        "<hr/>"
                        "<h1>Mouse tips for image area</h1>"
                        "<b>Wheel</b> — scroll image<br/>"
                        "<b>CTRL + Wheel</b> — zoom in/out<br/>"
                        "<b>CTRL + Click</b> <i>(left button)</i> on symbol — "
                            "select/highlight symbol<br/>"
                        "<b>CTRL + Click</b> <i>(left button)</i><B> and CTRL +"
                            " drag</b> — add symbols to selection<br/>"
                          "<b>Click (Left button) and Drag symbol border</b> — "
                            "change symbol box. It works with single symbol "
                            "selection only.<br/>"
                        "</p>"));
}
