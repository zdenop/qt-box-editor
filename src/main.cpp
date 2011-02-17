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

#include <QApplication>
#include <QTextCodec>

#include "MainWindow.h"
#include "Settings.h"

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(application);

  QApplication app(argc, argv);
  app.setOrganizationName(SETTING_ORGANIZATION);
  app.setApplicationName(SETTING_APPLICATION);

  QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

  MainWindow mainWin;
  mainWin.show();
  for (int i = 1; i < argc; ++i)
    {
      mainWin.addChild(argv[i]);
    }

  return app.exec();
}

