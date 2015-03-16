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

  ui->spinBox_x1->setFocus();
  // workaround selectAll() do not work...
  ui->spinBox_x1->stepUp();
  ui->spinBox_x1->stepDown();

  QLabel *lbl = new QLabel("<b>OR</b> you can insert line from Tesseract output e.g. <i>((1645,6315),(1756,6432))</i>", this);
  lbl->setToolTip("Use this, if you got Tesseract's output like <br/><pre>FAIL! APPLY_BOXES: boxfile line 58/D"
                  "((3166,6595),(3267,6686)): FAILURE! Couldn't find a matching blob</pre>");

  // NOTE: We do next here:
  //    1) delete buttonBox from layout
  //    2) insert our widgets with respective span parameters
  //    3) insert original buttonBox
  QLayoutItem *buttonBox = ui->gridLayout->itemAt(ui->gridLayout->rowCount());
  ui->gridLayout->addWidget(lbl, ui->gridLayout->rowCount(), 0, 4, 0);
  edtTesseractOutput = new QLineEdit(this);
  edtTesseractOutput->setPlaceholderText("Enter here numbers like '((3183,6605),(3291,6716))'");
  ui->gridLayout->addWidget(edtTesseractOutput, ui->gridLayout->rowCount(), 0, 4, 0);
  ui->gridLayout->addWidget(buttonBox->widget(), ui->gridLayout->rowCount(), 0, 4, 0);
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
    if (edtTesseractOutput && !edtTesseractOutput->text().isEmpty()) {
        const QString output(edtTesseractOutput->text());
        QRegExp rx("(\\d+)");
        QStringList list;
        int pos = 0;
        while ((pos = rx.indexIn(output, pos)) != -1) {
            list << rx.cap(1);
            pos += rx.matchedLength();
        }
        if (list.isEmpty() || list.count() != 4) {
            coords.setCoords(0, 0, 0, 0);
        } else {
            coords.setTopLeft(QPoint(list.at(0).toInt(), ui->spinBox_y1->maximum() - list.at(1).toInt()));
            coords.setBottomRight(QPoint(list.at(2).toInt(), ui->spinBox_y2->maximum() - list.at(3).toInt()));
        }
        edtTesseractOutput->clear();
    } else {
        ui->spinBox_x1->setFocus();
        ui->spinBox_x1->selectAll();

        if (ui->spinBox_x1->value() == 0
                && ui->spinBox_y1->value() == 0
                && ui->spinBox_x2->value() == 0
                && ui->spinBox_y2->value() == 0) {
            // NOTE: User hasn't changed anything
            coords.setCoords(0, 0, 0, 0);
        } else {
            // NOTE: User input some values
            coords.setCoords(ui->spinBox_x1->value(),
                             ui->spinBox_y1->maximum() - ui->spinBox_y1->value(),
                             ui->spinBox_x2->value(),
                             ui->spinBox_y2->maximum() - ui->spinBox_y2->value());
        }
    }
    return coords;
}
