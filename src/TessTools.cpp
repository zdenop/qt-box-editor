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

/*!
 * Create tesseract box data
 * input: filename
 */
QString TessTools::makeBoxes(const char* image) {
  QApplication::setOverrideCursor(Qt::WaitCursor);
  PIX   *pixs;
  if ((pixs = pixRead(image)) == NULL) {
    printf("Unsupported image type.\n");
    return "";
  }
  pixDestroy(&pixs);

  tesseract::TessBaseAPI api;

  // http://code.google.com/p/tesseract-ocr/issues/detail?id=228
  setlocale(LC_NUMERIC, "C");

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

/*!
 * Create tesseract box data from QImage
 */
QString TessTools::makeBoxes(QImage& qImage) {
  QApplication::setOverrideCursor(Qt::WaitCursor);
  PIX   *pixs;
  if ((pixs = qImage2PIX(qImage)) == NULL) {
    printf("Unsupported image type.\n");
    return "";
  }
  tesseract::TessBaseAPI api;

  // http://code.google.com/p/tesseract-ocr/issues/detail?id=228
  setlocale(LC_NUMERIC, "C");

  //TODO(zdenop): select path, select language
  if (api.Init(NULL,"eng")) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    return "";
  }

  api.SetVariable("tessedit_create_boxfile", "1");
  STRING text_out;
  //TODO(zdenop) take care about pages!
  if (!api.ProcessPage(pixs, 0, NULL, NULL, 0, &text_out)) {
    printf("Error during processing.\n");
  }

  QApplication::restoreOverrideCursor();
  pixDestroy(&pixs);
  return QString::fromUtf8(text_out.string());
}

/*!
 * Convert QT QImage to PIX
 * input: QImage
 * result: PIX
 */
PIX* TessTools::qImage2PIX(QImage& qImage) {
  PIX * pixs;
  l_uint32 *lines;

  qImage = qImage.rgbSwapped();
  int width = qImage.width();
  int height = qImage.height();
  int depth = qImage.depth();
  int wpl = qImage.bytesPerLine() / 4;
  
  pixs = pixCreate(width, height, depth);
  pixSetWpl(pixs, wpl);
  pixSetColormap(pixs, NULL);
  l_uint32 *datas = pixs->data;

  for (int y = 0; y < height; y++) {
    lines = datas + y * wpl;
    QByteArray a((const char*)qImage.scanLine(y), qImage.bytesPerLine());
    for (int j = 0; j < a.size(); j++) {
      *((l_uint8 *)lines + j) = a[j];
    }
  }

  return pixEndianByteSwapNew(pixs);
}

/*!
 * Convert Leptonica PIX to QImage
 * input: PIX
 * result: QImage
 */
QImage TessTools::PIX2qImage(PIX *pixImage) {
  l_uint32 * datas = pixGetData(pixEndianByteSwapNew(pixImage));

  int width = pixGetWidth(pixImage);
  int height = pixGetHeight(pixImage);
  int depth = pixGetDepth(pixImage);
  int bytesPerLine = pixGetWpl(pixImage) * 4;

  QImage::Format format;
  if (depth == 1)
    format = QImage::Format_Mono;
  else if (depth == 8)
    format = QImage::Format_Indexed8;
  else
    format = QImage::Format_RGB32;

  QImage result((uchar*)datas, width, height, bytesPerLine, format);

  // Handle pallete
  QVector<QRgb> _bwCT;
  _bwCT.append(qRgb(255,255,255));
  _bwCT.append(qRgb(0,0,0));

  QVector<QRgb> _grayscaleCT(256);
  for (int i = 0; i < 256; i++)  {
    _grayscaleCT.append(qRgb(i, i, i));
  }
  if (depth == 1) {
    result.setColorTable(_bwCT);
  }  else if (depth == 8)  {
    result.setColorTable(_grayscaleCT);

  } else {
    result.setColorTable(_grayscaleCT);
  }

  if (result.isNull()) {
    static QImage none(0,0,QImage::Format_Invalid);
    qDebug() << "***Invalid format!!!";
    return none;
  }

  return result.rgbSwapped();
}
