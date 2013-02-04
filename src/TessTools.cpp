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

#include <tesseract/strngs.h>
#include <locale.h>
#include "TessTools.h"
#include "Settings.h"

#include <QApplication>
#include <QWidget>
#include <QTextStream>
#include <QSettings>
#include <QStringList>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QFile>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QGuiApplication>
#endif

const char *TessTools::kTrainedDataSuffix = "traineddata";

// TODO(zdenop): Improve code here...

TessTools::TessTools() {
}

TessTools::~TessTools() {
}

/*
 * QString to const char
 */
const char *TessTools::qString2Char(QString string) {
    QByteArray byteArray = string.toUtf8();
    const char * constChar = byteArray.data();
    return constChar;
}

/*!
 * Create tesseract box data from QImage
 */
QString TessTools::makeBoxes(const QImage& qImage) {
  PIX   *pixs;

  if ((pixs = qImage2PIX(qImage)) == NULL) {
    msg("Unsupported image type");
    return "";
  }

  // http://code.google.com/p/tesseract-ocr/issues/detail?id=228
  setlocale(LC_NUMERIC, "C");
  // QString to  const char *
  #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QByteArray byteArray = getLang().toAscii();
  #else
  QByteArray byteArray = getLang().toLocal8Bit();
  #endif
  const char * apiLang = byteArray.constData();

  // workaroung if datapath/TESSDATA_PREFIX is set...
  #ifdef _WIN32
  QString envQString = "TESSDATA_PREFIX=" + getDataPath() ;
  QByteArray byteArrayWin = envQString.toUtf8();
  const char * env = byteArrayWin.data();
  putenv(env);
  #else
  QByteArray byteArray1 = getDataPath().toUtf8();
  const char * datapath = byteArray1.data();
  setenv("TESSDATA_PREFIX", datapath, 1);
  #endif

  tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
  if (api->Init(NULL, apiLang)) {
    msg("Could not initialize tesseract.\n");
    return "";
  }

  api->SetVariable("tessedit_create_boxfile", "1");
  STRING text_out;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  //TODO(zdenop) take care about pages!
  if (!api->ProcessPage(pixs, 0, NULL, NULL, 0, &text_out)) {
    msg("Error during processing.\n");
  }

  QApplication::restoreOverrideCursor();

  pixDestroy(&pixs);
  api->End();
  delete api;
  return QString::fromUtf8(text_out.string());
}

/*!
 * Convert QT QImage to PIX
 * input: QImage
 * result: PIX
 */
PIX* TessTools::qImage2PIX(const QImage& qImage) {
  PIX * pixs;
  l_uint32 *lines;

  QImage myImage = qImage.rgbSwapped();
  int width = myImage.width();
  int height = myImage.height();
  int depth = myImage.depth();
  int wpl = myImage.bytesPerLine() / 4;

  pixs = pixCreate(width, height, depth);
  pixSetWpl(pixs, wpl);
  pixSetColormap(pixs, NULL);
  l_uint32 *datas = pixs->data;

  for (int y = 0; y < height; y++) {
    lines = datas + y * wpl;
    QByteArray a((const char*)myImage.scanLine(y), myImage.bytesPerLine());
    for (int j = 0; j < a.size(); j++) {
      *((l_uint8 *)lines + j) = a[j];
    }
  }

  const qreal toDPM = 1.0 / 0.0254;
  int resolutionX = myImage.dotsPerMeterX() / toDPM;
  int resolutionY = myImage.dotsPerMeterY() / toDPM;

  if (resolutionX < 300) resolutionX = 300;
  if (resolutionY < 300) resolutionY = 300;
  pixSetResolution(pixs, resolutionX, resolutionY);

  return pixEndianByteSwapNew(pixs);
}

/*!
 * Convert Leptonica PIX to QImage
 * input: PIX
 * result: QImage
 */
QImage TessTools::PIX2qImage(PIX *pixImage) {
  int width = pixGetWidth(pixImage);
  int height = pixGetHeight(pixImage);
  int depth = pixGetDepth(pixImage);
  int bytesPerLine = pixGetWpl(pixImage) * 4;
  l_uint32 * datas = pixGetData(pixEndianByteSwapNew(pixImage));

  QImage::Format format;
  if (depth == 1)
    format = QImage::Format_Mono;
  else if (depth == 8)
    format = QImage::Format_Indexed8;
  else
    format = QImage::Format_RGB32;

  QImage result((uchar*)datas, width, height, bytesPerLine, format);

  // Set resolution
  l_int32 	xres, yres;
  pixGetResolution(pixImage, &xres, &yres);
  const qreal toDPM = 1.0 / 0.0254;
  result.setDotsPerMeterX(xres * toDPM);
  result.setDotsPerMeterY(yres * toDPM);

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
    qDebug("Invalid format!!!\n");
    return none;
  }

  pixDestroy(&pixImage);
  return result.rgbSwapped();
}

QImage TessTools::GetThresholded(const QImage& qImage) {
    // TODO(zdenop): Check if here is not  memory leak
    PIX * pixs = qImage2PIX(qImage);

    // Set tessdata as Enviromental Variable to avoid problems
    QByteArray byteArray1 = getDataPath().toUtf8();
    #ifdef _WIN32
    QString envQString = "TESSDATA_PREFIX=" + byteArray1 ;
    const char * env = qString2Char(envQString);
    putenv(env);
    #else
    const char * datapath = byteArray1.data();
    setenv("TESSDATA_PREFIX", datapath, 1);
    #endif

    // TODO(zdenop): Why apiLang = qString2Char(getLang()) do not work???
    #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QByteArray byteArray = getLang().toAscii();
    #else
    QByteArray byteArray = getLang().toLocal8Bit();
    #endif
    const char * apiLang = byteArray.constData();

    setlocale(LC_NUMERIC, "C");

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    if (api->Init(NULL, apiLang)) {
        msg("Could not initialize tesseract.\n");
        return QImage();
    }
    api->SetImage(pixs);
    PIX * pixq = api->GetThresholdedImage();

    api->End();
    delete api;
    pixDestroy(&pixs);

    return PIX2qImage(pixq);
}

QString TessTools::getDataPath() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       SETTING_ORGANIZATION, SETTING_APPLICATION);
    QString dataPath;
    if (settings.contains("Tesseract/DataPath")) {
      dataPath = settings.value("Tesseract/DataPath").toString();
    }
    return dataPath;
}

QString TessTools::getLang() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       SETTING_ORGANIZATION, SETTING_APPLICATION);
    QString lang;
    if (settings.contains("Tesseract/Lang")) {
        lang = settings.value("Tesseract/Lang").toString();
    }

    if (lang.isNull()) {
        msg("You need to configure tesseract in Settings!");
        return "";
    }
   return lang;
}

/*!
 * Get QList<QString> with list of available languages
 */
QList<QString> TessTools::getLanguages(QString datapath) {
    QList<QString> languages;
    QDir dir(datapath);

    if (!dir.exists()) {
      QMessageBox msgBox;
      msgBox.setText(QObject::tr("Cannot find the tessdata directory '%1'!\n").arg(datapath) +
                  QObject::tr("Please check your configuration or tesseract instalation"));
      msgBox.exec();
      return languages;
      }

    QString filter = "*.";
    filter += kTrainedDataSuffix;
    QStringList filters;
    filters << filter.trimmed();
    dir.setNameFilters(filters);

    QFileInfoList list = dir.entryInfoList();

    for (int i = 0; i < list.size(); ++i) {
      QFileInfo fileInfo = list.at(i);
      languages.append(QString("%1").arg(fileInfo.baseName()));
    }
    qSort(languages);

    return languages;
}

void TessTools::msg(QString messageText) {
    QMessageBox msgBox;
    msgBox.setText(messageText);
    msgBox.exec();
}

