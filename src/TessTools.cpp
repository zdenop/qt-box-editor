/**********************************************************************
* File:        TessTools.cpp
* Description: Tesseract tools
* Author:      Zdenko Podobny
* Created:     2012-03-27
*
* (C) Copyright 2012, Zdenko Podobny
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

#include <baseapi.h>
#include <allheaders.h>
#include "TessTools.h"
#include <locale.h>

#include <QtCore/QDebug>
#include <QtGui/QApplication>
#include <QTextStream>
#include <QStringList>
#include <QFile>

QString TessTools::makeBoxes(const char* image) {
    //TODO(zdenop): remove
    qDebug() << "makebox:" << image;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    PIX   *pixs;
    if ((pixs = pixRead(image)) == NULL) {
      printf("Unsupported image type.\n");
      return "";
    }
    pixDestroy(&pixs);

    tesseract::TessBaseAPI api;

    // http://code.google.com/p/tesseract-ocr/issues/detail?id=228
    setlocale (LC_NUMERIC, "C");

    //TODO(zdenop): select path, select language
    if (api.Init(NULL,"eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        return "";
    }

    api.SetVariable("tessedit_create_boxfile", "1");
    STRING text_out;
    if (!api.ProcessPages(image, NULL, 0, &text_out)) {
      printf("Error during processing.\n");
    }

    QApplication::restoreOverrideCursor();
    return QString::fromUtf8(text_out.string());
}
