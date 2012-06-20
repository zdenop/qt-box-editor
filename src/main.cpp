/**********************************************************************
* File:        main.cpp
* Description: Main function
* Author:      Marcel Kolodziejczyk
* Created:     2010-01-04
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

#include <QtCore/QTextCodec>
#include <QtGui/QApplication>

#if defined _COMPOSE_STATIC_
#include <QtPlugin>
Q_IMPORT_PLUGIN(qsvg)
#endif

#include "include/MainWindow.h"
#include "include/Settings.h"

int main(int argc, char* argv[]) {
  Q_INIT_RESOURCE(application);

  QApplication app(argc, argv);
  app.setOrganizationName(SETTING_ORGANIZATION);
  app.setApplicationName(SETTING_APPLICATION);

  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       SETTING_ORGANIZATION, SETTING_APPLICATION);
  //Set the style saved inside the configuration if any
  QString style = settings.value("GUI/Style").toString();
  if (style.isEmpty()) {
    app.setStyle(QStyleFactory::create("Plastique"));
  } else {
    app.setStyle(QStyleFactory::create(style));
  }

  //Set the icon theme saved inside the configuration if any
  QString iconTheme = settings.value("GUI/IconTheme").toString();
  if (!iconTheme.isEmpty()) {
      QIcon::setThemeName(iconTheme);
  }
  static const char * GENERIC_ICON_TO_CHECK = "fileopen";
  static const char * FALLBACK_ICON_THEME = "faenza";
  if (!QIcon::hasThemeIcon(GENERIC_ICON_TO_CHECK)) {
      //If there is no default working icon theme then we should
      //use an icon theme that we provide via a .qrc file
      //This case happens under Windows and Mac OS X
      //This does not happen under GNOME or KDE
      QIcon::setThemeName(FALLBACK_ICON_THEME);
  }

  QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

  MainWindow mainWin;
  mainWin.show();
  for (int i = 1; i < argc; ++i) {
    mainWin.addChild(argv[i]);
  }

  return app.exec();
}

