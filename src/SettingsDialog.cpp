/**********************************************************************
* File:        SettingsDialog.cpp
* Description: Settings Dialog
* Author:      Zdenko Podobny
* Created:     2011-02-11
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

#include "include/SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget* parent, int tabIndex)
  : QDialog(parent) {
  setFixedSize(420, 370);
  setWindowTitle(tr("%1 :: Settings...").arg(SETTING_APPLICATION));

  setupUi(this);
  initSettings();

  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  tabSetting->setCurrentIndex(tabIndex);
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::on_fontButton_clicked() {
  bool ok = false;

  tableFont = QFontDialog::getFont(&ok, fontLabel->font(), this,
                                   "Select font...");
  if (ok) {
    fontLabel->setFont(tableFont);
    fontLabel->setText(tableFont.family().toAscii() +
                       tr(", %1 pt").arg(tableFont.pointSize()));
  }
}

void SettingsDialog::on_colorRectButton_clicked() {
  chooseColor(colorRectButton, &rectColor);
}

void SettingsDialog::on_rectFillColorButton_clicked() {
  chooseColor(rectFillColorButton, &rectFillColor);
}

void SettingsDialog::on_colorBoxButton_clicked() {
  chooseColor(colorBoxButton, &boxColor);
}

void SettingsDialog::on_backgroundColorButton_clicked() {
  chooseColor(backgroundColorButton, &backgroundColor);
}


void SettingsDialog::initSettings() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);

  QString fontname = settings.value("GUI/Font").toString();

  if (fontname.isEmpty()) {
    tableFont.setFamily(TABLE_FONT);
    tableFont.setPointSize(TABLE_FONT_SIZE);
  } else {
    tableFont = settings.value("GUI/Font").value<QFont>();
  }

  if (settings.contains("GUI/Rectagle")) {
    rectColor = settings.value("GUI/Rectagle").value<QColor>();
  } else {
    rectColor = Qt::red;
  }

  if (settings.contains("GUI/Rectagle_fill")) {
    rectFillColor = settings.value("GUI/Rectagle_fill").value<QColor>();
  } else {
    rectFillColor = Qt::red;
    rectFillColor.setAlpha(127);
  }

  if (settings.contains("GUI/Box")) {
    boxColor = settings.value("GUI/Box").value<QColor>();
  } else {
    boxColor = Qt::green;
  }

  if (settings.contains("GUI/BackgroundColor")) {
    backgroundColor = settings.value("GUI/BackgroundColor").value<QColor>();
  } else {
    backgroundColor = (Qt::gray);
  }

  if (settings.contains("Text/OpenDialog"))
    cbOpenDialog->setChecked(settings.value("Text/OpenDialog").toBool());
  if (settings.contains("Text/WordSpace"))
    sbWordSpace->setValue(settings.value("Text/WordSpace").toInt());
  if (settings.contains("Text/ParagraphIndent"))
    spParaIndent->setValue(settings.value("Text/ParagraphIndent").toInt());
  if (settings.contains("Text/Ligatures"))
    pteLigatures->setPlainText(settings.value("Text/Ligatures").toString());

  fontLabel->setFont(tableFont);
  fontLabel->setText(tableFont.family().toAscii() +
                     tr(", %1 pt").arg(tableFont.pointSize()));
  updateColorButton(colorRectButton, rectColor);
  updateColorButton(rectFillColorButton, rectFillColor);
  updateColorButton(colorBoxButton, boxColor);
  updateColorButton(backgroundColorButton, backgroundColor);
}

void SettingsDialog::saveSettings() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);

  settings.setValue("GUI/Font", tableFont);
  settings.setValue("GUI/Rectagle", rectColor);
  settings.setValue("GUI/Rectagle_fill", rectFillColor);
  settings.setValue("GUI/Box", boxColor);
  settings.setValue("GUI/BackgroundColor", backgroundColor);

  settings.setValue("Text/OpenDialog", cbOpenDialog->isChecked());
  settings.setValue("Text/WordSpace", sbWordSpace->value());
  settings.setValue("Text/ParagraphIndent", spParaIndent->value());

  // remove duplicates and blank lines
  QList<QString> ligatures = pteLigatures->toPlainText().split("\n");
  QHash<QString, bool> h;
  QString str;

  // make unique list
  for (int i = 0; i < ligatures.size(); ++i)
    h.insert(ligatures.at(i), true);
  ligatures = h.keys();

  // convert list to string separated with "\n"
  for (int i = 0; i < ligatures.size(); ++i) {
    if (i == 0)
      str = ligatures[i];
    else
      str += "\n" + ligatures[i];
  }
  str = str.remove(QRegExp("^\n"));
  settings.setValue("Text/Ligatures", str);

  // emit setTableFont(tableFont);
  // TODO(zdenop): use font for open child windows
  emit accept();
}

void SettingsDialog::chooseColor(QPushButton* button, QColor* color) {
#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor(*color, this, tr("Select color..."),
                    QColorDialog::ShowAlphaChannel);
#else
  QColor newColor = QColorDialog::getColor(*color, this, tr("Select color..."));
#endif
  if (newColor.isValid()) {
    *color = newColor;
    updateColorButton(button, *color);
  }
}

void SettingsDialog::updateColorButton(QPushButton* button,
                                       const QColor& color) {
  QPixmap pixmap(68, 20);
  pixmap.fill(color);
  QIcon icon(pixmap);
  QSize iconSize(pixmap.width(), pixmap.height());
  button->setIconSize(iconSize);
  button->setIcon(icon);
}
