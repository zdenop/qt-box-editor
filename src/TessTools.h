/**********************************************************************
* File:        TessTools.h
* Description: Tesseract tools
* Author:      Zdenko Podobny
* Created:     2012-03-27
*
* (C) Copyright 2012-2013, Zdenko Podobny
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

#ifndef SRC_INCLUDE_TESSTOOLS_H_
#define SRC_INCLUDE_TESSTOOLS_H_

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include "TessTools.h"
#include <QString>
#include <QImage>

class TessTools {

public:
  TessTools();
  ~TessTools();
  QString makeBoxes(const QImage &qImage, const int page);
  static PIX* qImage2PIX(const QImage &qImage);
  static QImage PIX2qImage(PIX *pixImage);
  static QImage GetThresholded(const QImage& qImage);
  static const char *qString2Char(QString string);
  QList<QString> getLanguages(QString datapath);

private:
  static QString getDataPath();
  static QString getLang();
  static void msg(QString messageText);
  static const char *kTrainedDataSuffix;
};

#endif  // SRC_INCLUDE_TESSTOOLS_H_
