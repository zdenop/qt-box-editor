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

#include "dialogs/DrawRectangle.h"
#include "ui_DrawRectangle.h"

//http://qt.gitorious.org/qt-labs/qtspotify/blobs/b7b0ecae6f5ab815e941bd62c00b0f87d2810016/logindialog.cpp
DrawRectangle::DrawRectangle(QWidget* parent, QString title, int maxWidth,
                             int maxHeight) :
    QDialog(parent),
    ui(new Ui::DrawRect)
{
  ui->setupUi(this);

  if (!title.isEmpty())
      setWindowTitle(tr("Draw rectangle in %1:").arg(title));

  ui->spinBox_x1->setMaximum(maxWidth);
  ui->spinBox_x2->setMaximum(maxWidth);
  ui->spinBox_y1->setMaximum(maxHeight);
  ui->spinBox_y2->setMaximum(maxHeight);

}

DrawRectangle::~DrawRectangle()
{
    delete ui;
}
/**
 * Input: tesseract coord; e.g.:Bounding box=(197,27)->(206,41)
 * return: image coord ; e.g.: (197, , 206, )
 */
QRect DrawRectangle::getRectangle() const
{
    QRect coords;
    coords.setCoords(ui->spinBox_x1->value(),
                     ui->spinBox_y1->maximum() - ui->spinBox_y1->value(),
                     ui->spinBox_x2->value(),
                     ui->spinBox_y2->maximum() - ui->spinBox_y2->value());
    return coords;
}
