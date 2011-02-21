/**********************************************************************
* File:        SettingsDialog.h
* Description: Settings Dialog
* Author:      Zdenko Podobny
* Created:     2011-02-11
*
* (C) Copyright 2010, Marcel Kolodziejczyk
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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtCore/QSettings>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QButtonGroup>
#include <QtGui/QColorDialog>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFontDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include "Settings.h"

class SettingsDialog : public QDialog
{
Q_OBJECT
public:
SettingsDialog(QWidget *pard = 0);
~SettingsDialog();
QDialogButtonBox *buttonBox;
QTabWidget *tabSetting;
QVBoxLayout *fontAndColorsLayout;
QLabel *fontLabel;
QPushButton *colorRectButton;
QPushButton *rectFillColorButton;
QPushButton *colorBoxButton;
QPushButton *backgroundColorButton;

void initSettings();

private slots:
    void on_fontButton_clicked();
    void on_colorRectButton_clicked();
    void on_rectFillColorButton_clicked();
    void on_colorBoxButton_clicked();
    void on_backgroundColorButton_clicked();
    void saveSettings();

private:
    void chooseColor(QPushButton *button, QColor *color);
    void updateColorButton(QPushButton *button, const QColor &color);

    QFont tableFont;
    QColor rectColor;
    QColor rectFillColor;
    QColor boxColor;
    QColor backgroundColor;
};
#endif // SETTINGSDIALOG_H
