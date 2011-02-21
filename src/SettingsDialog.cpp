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
#include <QtGui>
#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
  : QDialog(parent)
{
  setFixedSize(420, 370);
  setWindowTitle(tr("%1 :: Settings...").arg(SETTING_APPLICATION));

  initSettings();

  QVBoxLayout *verticalLayout = new QVBoxLayout(this);

  tabSetting = new QTabWidget(this);
  tabSetting->resize(width(), height());

  fontLabel = new QLabel(tableFont.family().toAscii() + tr(", %1 pt").arg(tableFont.pointSize()), this);
  fontLabel->setFont(tableFont);
  QPushButton *fontButton = new QPushButton(tr("Change..."));
  fontButton->setMaximumSize(QSize(75, 26));
  connect(fontButton, SIGNAL(clicked()), this, SLOT(on_fontButton_clicked()));

  QSpacerItem *horizontalSpacer_1 = new QSpacerItem(158, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  QGroupBox *fontGroupBox = new QGroupBox(tr("Font"));

  QGridLayout *gridLayout_1 = new QGridLayout(fontGroupBox);
  gridLayout_1->addWidget(fontLabel, 0, 0, 1, 1);
  gridLayout_1->addItem(horizontalSpacer_1, 0, 1, 1, 1);
  gridLayout_1->addWidget(fontButton, 0, 2, 1, 1);

  fontGroupBox->setLayout(gridLayout_1);

  QGroupBox *colorsGroupBox = new QGroupBox(tr("Colors"));
  QGridLayout *gridLayout_2 = new QGridLayout(colorsGroupBox);

  QLabel *colorRectLabel = new QLabel(tr("Selection rectangle:"), colorsGroupBox);
  gridLayout_2->addWidget(colorRectLabel, 0, 0, 1, 1);

  QSpacerItem *horizontalSpacer_2 = new QSpacerItem(25, 12, QSizePolicy::Fixed, QSizePolicy::Minimum);
  gridLayout_2->addItem(horizontalSpacer_2, 0, 1, 1, 1);

  colorRectButton = new QPushButton(colorsGroupBox);
  colorRectButton->setMaximumSize(QSize(75, 26));
  connect(colorRectButton, SIGNAL(clicked()), this, SLOT(on_colorRectButton_clicked()));
  updateColorButton(colorRectButton, rectColor);
  gridLayout_2->addWidget(colorRectButton, 0, 2, 1, 1);

  QLabel *colorFillRectLabel = new QLabel(tr("Selection fill:"), colorsGroupBox);
  gridLayout_2->addWidget(colorFillRectLabel, 2, 0, 1, 1);

  QSpacerItem *horizontalSpacer_3 = new QSpacerItem(25, 12, QSizePolicy::Fixed, QSizePolicy::Minimum);
  gridLayout_2->addItem(horizontalSpacer_3, 2, 1, 1, 1);

  rectFillColorButton = new QPushButton(colorsGroupBox);
  rectFillColorButton->setMaximumSize(QSize(75, 26));
  connect(rectFillColorButton, SIGNAL(clicked()), this, SLOT(on_rectFillColorButton_clicked()));
  updateColorButton(rectFillColorButton, rectFillColor);
  gridLayout_2->addWidget(rectFillColorButton, 2, 2, 1, 1);

  QLabel *colorBoxLabel = new QLabel(tr("Boxes:"), colorsGroupBox);
  gridLayout_2->addWidget(colorBoxLabel, 3, 0, 1, 1);

  QSpacerItem *horizontalSpacer_4 = new QSpacerItem(25, 12, QSizePolicy::Fixed, QSizePolicy::Minimum);
  gridLayout_2->addItem(horizontalSpacer_4, 3, 1, 1, 1);

  colorBoxButton = new QPushButton(colorsGroupBox);
  colorBoxButton->setMaximumSize(QSize(75, 26));
  connect(colorBoxButton, SIGNAL(clicked()), this, SLOT(on_colorBoxButton_clicked()));
  updateColorButton(colorBoxButton, boxColor);
  gridLayout_2->addWidget(colorBoxButton, 3, 2, 1, 1);

  QLabel *backgroundColorLabel = new QLabel(tr("Background:"), colorsGroupBox);
  gridLayout_2->addWidget(backgroundColorLabel, 4, 0, 1, 1);

  QSpacerItem *horizontalSpacer_5 = new QSpacerItem(25, 12, QSizePolicy::Fixed, QSizePolicy::Minimum);
  gridLayout_2->addItem(horizontalSpacer_5, 4, 1, 1, 1);

  backgroundColorButton = new QPushButton(colorsGroupBox);
  backgroundColorButton->setMaximumSize(QSize(75, 26));
  connect(backgroundColorButton, SIGNAL(clicked()), this, SLOT(on_backgroundColorButton_clicked()));
  updateColorButton(backgroundColorButton, backgroundColor);
  gridLayout_2->addWidget(backgroundColorButton, 4, 2, 1, 1);

  QSpacerItem *verticalSpacer = new QSpacerItem(20, 53, QSizePolicy::Minimum, QSizePolicy::Expanding);

  QWidget *layoutWidget1 = new QWidget(tabSetting);
  QVBoxLayout *fontAndColorsLayout = new QVBoxLayout(layoutWidget1);
  fontAndColorsLayout->addWidget(fontGroupBox);
  fontAndColorsLayout->addWidget(colorsGroupBox);
  fontAndColorsLayout->addItem(verticalSpacer);

  QWidget *fontAndColorsSett = new QWidget;
  fontAndColorsSett->setLayout(fontAndColorsLayout);
  tabSetting->addTab(fontAndColorsSett, tr("Font && Colors"));   //&amp;

  verticalLayout->addWidget(tabSetting);

  buttonBox = new QDialogButtonBox(this);
  buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
  buttonBox->setGeometry(QRect(30, 240, 341, 32));
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  verticalLayout->addWidget(buttonBox);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::on_fontButton_clicked()
{
  bool ok = false;

  tableFont = QFontDialog::getFont(&ok, fontLabel->font(), this, "Select font...");
  if (ok)
    {
      fontLabel->setFont(tableFont);
      fontLabel->setText(tableFont.family().toAscii() + tr(", %1 pt").arg(tableFont.pointSize()));
    }
}

void SettingsDialog::on_colorRectButton_clicked()
{
  chooseColor(colorRectButton, &rectColor);
}

void SettingsDialog::on_rectFillColorButton_clicked()
{
  chooseColor(rectFillColorButton, &rectFillColor);
}

void SettingsDialog::on_colorBoxButton_clicked()
{
  chooseColor(colorBoxButton, &boxColor);
}

void SettingsDialog::on_backgroundColorButton_clicked()
{
  chooseColor(backgroundColorButton, &backgroundColor);
}


void SettingsDialog::initSettings()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);

  QString fontname = settings.value("GUI/Font").toString();

  if (fontname.isEmpty())
    {
      tableFont.setFamily(TABLE_FONT);
      tableFont.setPointSize(TABLE_FONT_SIZE);
    }
  else
    {
      tableFont = settings.value("GUI/Font").value<QFont>();
    }

  if (settings.contains("GUI/Rectagle"))
    {
      rectColor = settings.value("GUI/Rectagle").value<QColor>();
    }
  else
    {
      rectColor = Qt::red;
    }

  if (settings.contains("GUI/Rectagle_fill"))
    rectFillColor = settings.value("GUI/Rectagle_fill").value<QColor>();
  else
    {
      rectFillColor = Qt::red;
      rectFillColor.setAlpha(127);
    }

  if (settings.contains("GUI/Box"))
    {
      boxColor = settings.value("GUI/Box").value<QColor>();
    }
  else
    {
      boxColor = Qt::green;
    }

  if (settings.contains("GUI/BackgroundColor"))
    {
      backgroundColor = settings.value("GUI/BackgroundColor").value<QColor>();
    }
  else
    {
      backgroundColor = (Qt::gray);
    }
}

void SettingsDialog::saveSettings()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, SETTING_ORGANIZATION, SETTING_APPLICATION);

  settings.setValue("GUI/Font", tableFont);
  settings.setValue("GUI/Rectagle", rectColor);
  settings.setValue("GUI/Rectagle_fill", rectFillColor);
  settings.setValue("GUI/Box", boxColor);
  settings.setValue("GUI/BackgroundColor", backgroundColor);
  //emit setTableFont(tableFont); // TODO: use font for open child windows
  emit accept();
}

void SettingsDialog::chooseColor(QPushButton *button, QColor *color)
{
#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor(*color, this, tr("Select color..."), QColorDialog::ShowAlphaChannel);
#else
  QColor newColor = QColorDialog::getColor(*color, this, tr("Select color..."));
#endif
  if (newColor.isValid())
    {
      *color = newColor;
      updateColorButton(button, *color);
    }
}

void SettingsDialog::updateColorButton(QPushButton *button,
                                       const QColor &color)
{
  QPixmap pixmap(68, 20);
  pixmap.fill(color);
  QIcon icon(pixmap);
  QSize iconSize(pixmap.width(), pixmap.height());
  button->setIconSize(iconSize);
  button->setIcon(icon);

}
