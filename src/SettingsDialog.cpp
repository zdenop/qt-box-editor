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
#include "include/TessTools.h"


SettingsDialog::SettingsDialog(QWidget* parent, int tabIndex)
  : QDialog(parent) {
  setFixedSize(420, 370);
  setWindowTitle(tr("%1 :: Settings...").arg(SETTING_APPLICATION));

  setupUi(this);
  initLangs();
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
    QFont tempFont = tableFont;
    // Lets use reasonable font size ;-)
    if (tableFont.pointSize() > 12)
        tempFont.setPointSize(12);
    fontLabel->setFont(tempFont);
    fontLabel->setText(tableFont.family().toAscii() +
                       tr(", %1 pt").arg(tableFont.pointSize()));
  }
}

void SettingsDialog::on_fontImageButton_clicked() {
  bool ok = false;

  imageFont = QFontDialog::getFont(&ok, fontImageLabel->font(), this,
                                   "Select font...");
  if (ok) {
    QFont tempFont = imageFont;
    // Lets use reasonable font size ;-)
    if (imageFont.pointSize() > 12)
          tempFont.setPointSize(12);
    fontImageLabel->setFont(imageFont);
    fontImageLabel->setText(imageFont.family().toAscii() +
                       tr(", %1 pt").arg(imageFont.pointSize()));
  }
}

void SettingsDialog::on_imageFontColorButton_clicked() {
  chooseColor(imageFontColorButton, &imageFontColor);
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

   QString fontImageName = settings.value("GUI/ImageFont").toString();
   if (fontImageName.isEmpty()) {
     imageFont.setFamily(TABLE_FONT);
     imageFont.setPointSize(TABLE_FONT_SIZE);
   } else {
     imageFont = settings.value("GUI/ImageFont").value<QFont>();
   }

  if (settings.contains("GUI/UseTheSameFont")) {
     useSameFontCB->setChecked(settings.value("GUI/UseTheSameFont").toBool());
     fontImageButton->setDisabled(useSameFontCB->isChecked());
     fontImageLabel->setDisabled(useSameFontCB->isChecked());
     fontImageLbl->setDisabled(useSameFontCB->isChecked());
  }

  if (settings.contains("GUI/ImageFontOffset"))
    offsetSpinBox->setValue(settings.value("GUI/ImageFontOffset").toInt());

  if (settings.contains("GUI/BalloonCount"))
    ballonsSpinBox->setValue(settings.value("GUI/BalloonCount").toInt());

  if (settings.contains("GUI/ImageFontColor")) {
    imageFontColor = settings.value("GUI/ImageFontColor").value<QColor>();
  } else {
    imageFontColor = Qt::red;
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

  // Text settings
  if (settings.contains("Text/OpenDialog"))
    cbOpenDialog->setChecked(settings.value("Text/OpenDialog").toBool());
  if (settings.contains("Text/WordSpace"))
    sbWordSpace->setValue(settings.value("Text/WordSpace").toInt());
  if (settings.contains("Text/ParagraphIndent"))
    spParaIndent->setValue(settings.value("Text/ParagraphIndent").toInt());
  if (settings.contains("Text/Ligatures"))
    pteLigatures->setPlainText(settings.value("Text/Ligatures").toString());

  QFont tempFont = tableFont;
  // Lets use reasonable font size ;-)
  if (tableFont.pointSize() > 12)
        tempFont.setPointSize(12);
  fontLabel->setFont(tempFont);
  fontLabel->setText(tableFont.family().toAscii() +
                     tr(", %1 pt").arg(tableFont.pointSize()));
  tempFont = imageFont;
  if (imageFont.pointSize() > 12)
        tempFont.setPointSize(12);
  fontImageLabel->setFont(tempFont);
  fontImageLabel->setText(imageFont.family().toAscii() +
                     tr(", %1 pt").arg(imageFont.pointSize()));

  updateColorButton(imageFontColorButton, imageFontColor);
  updateColorButton(colorRectButton, rectColor);
  updateColorButton(rectFillColorButton, rectFillColor);
  updateColorButton(colorBoxButton, boxColor);
  updateColorButton(backgroundColorButton, backgroundColor);

  // Tesseract settings
  if (settings.contains("Tesseract/DataPath")) {
    lnPrefix->setText(settings.value("Tesseract/DataPath").toString());
  }
  if (settings.contains("Tesseract/Lang")) {
      int langindex = cbLang->findData(
            settings.value("Tesseract/Lang").toString());
      cbLang->setCurrentIndex(langindex);
  }
}

void SettingsDialog::saveSettings() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                     SETTING_ORGANIZATION, SETTING_APPLICATION);

  settings.setValue("GUI/Font", tableFont);
  settings.setValue("GUI/ImageFont", imageFont);
  settings.setValue("GUI/UseTheSameFont", useSameFontCB->isChecked());
  settings.setValue("GUI/ImageFontOffset", offsetSpinBox->value());
  settings.setValue("GUI/BalloonCount", ballonsSpinBox->value());
  settings.setValue("GUI/ImageFontColor", imageFontColor);
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

  settings.setValue("Tesseract/DataPath", lnPrefix->text());
  settings.setValue("Tesseract/Lang",
                    cbLang->itemData(cbLang->currentIndex()).toString());

  emit settingsChanged();
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

void SettingsDialog::on_pbSelectDP_clicked() {
    QString prefix_dir = QFileDialog::getExistingDirectory(
                          this,
                          tr("Select Path Prefix To tessdata Directory..."),
                          lnPrefix->text(),
                          QFileDialog::ShowDirsOnly
                          | QFileDialog::DontResolveSymlinks);
    // we need only prefix without /tessdata!!!
    if (prefix_dir.contains(QRegExp("/tessdata$")))
            prefix_dir.replace(QRegExp("/tessdata$"),"/");
    else if (prefix_dir.contains(QRegExp("/tessdata/$")))
        prefix_dir.replace(QRegExp("/tessdata/$"),"/");
    if (prefix_dir != "")
        lnPrefix->setText(prefix_dir);
    initLangs();
}

void SettingsDialog::on_pbCheck_clicked() {
    initLangs();
}

void SettingsDialog::initLangs() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                    SETTING_ORGANIZATION, SETTING_APPLICATION);
    QString datapath = settings.value("Tesseract/DataPath").toString();
    datapath += "tessdata";
    TessTools tt;
    QList<QString> languages = tt.getLanguages(datapath);

    // Clean combobox with languages
    int langsCount = cbLang->count();
    if (langsCount > 0)
      for (int i = langsCount; i >= 0; i--) {
        cbLang->removeItem(0);
      }

    QString lang;
    foreach(lang, languages)
    if (getLangName(lang) == "")
      cbLang->addItem(lang, lang);
    else
      cbLang->addItem(getLangName(lang), lang);

    // sort language list
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(cbLang);
    proxy->setSourceModel(cbLang->model());
    cbLang->model()->setParent(proxy);
    cbLang->setModel(proxy);
    cbLang->model()->sort(0);
}

/**
 * Get Language name from ISO 639-1 code
 * http://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
 */
QString SettingsDialog::getLangName(QString lang) {
    QMap<QString,QString> map;
    map["abk"] = QObject::tr("Abkhaz");
    map["aar"] = QObject::tr("Afar");
    map["afr"] = QObject::tr("Afrikaans");
    map["aka"] = QObject::tr("Akan");
    map["sqi"] = QObject::tr("Albanian");
    map["amh"] = QObject::tr("Amharic");
    map["ara"] = QObject::tr("Arabic");
    map["arg"] = QObject::tr("Aragonese");
    map["hye"] = QObject::tr("Armenian");
    map["asm"] = QObject::tr("Assamese");
    map["ava"] = QObject::tr("Avaric");
    map["ave"] = QObject::tr("Avestan");
    map["aym"] = QObject::tr("Aymara");
    map["aze"] = QObject::tr("Azerbaijani");
    map["bam"] = QObject::tr("Bambara");
    map["bak"] = QObject::tr("Bashkir");
    map["eus"] = QObject::tr("Basque");
    map["bel"] = QObject::tr("Belarusian");
    map["ben"] = QObject::tr("Bengali");
    map["bih"] = QObject::tr("Bihari");
    map["bis"] = QObject::tr("Bislama");
    map["bos"] = QObject::tr("Bosnian");
    map["bre"] = QObject::tr("Breton");
    map["bul"] = QObject::tr("Bulgarian");
    map["mya"] = QObject::tr("Burmese");
    map["cat"] = QObject::tr("Catalan; Valencian");
    map["cha"] = QObject::tr("Chamorro");
    map["che"] = QObject::tr("Chechen");
    map["chr"] = QObject::tr("Cherokee");
    map["nya"] = QObject::tr("Chichewa; Chewa; Nyanja");
    map["zho"] = QObject::tr("Chinese");
    map["chv"] = QObject::tr("Chuvash");
    map["cor"] = QObject::tr("Cornish");
    map["cos"] = QObject::tr("Corsican");
    map["cre"] = QObject::tr("Cree");
    map["hrv"] = QObject::tr("Croatian");
    map["ces"] = QObject::tr("Czech");
    map["dan"] = QObject::tr("Danish");
    map["div"] = QObject::tr("Divehi; Dhivehi; Maldivian;");
    map["nld"] = QObject::tr("Dutch");
    map["dzo"] = QObject::tr("Dzongkha");
    map["eng"] = QObject::tr("English");
    map["epo"] = QObject::tr("Esperanto");
    map["est"] = QObject::tr("Estonian");
    map["ewe"] = QObject::tr("Ewe");
    map["fao"] = QObject::tr("Faroese");
    map["fij"] = QObject::tr("Fijian");
    map["fin"] = QObject::tr("Finnish");
    map["fra"] = QObject::tr("French");
    map["ful"] = QObject::tr("Fula; Fulah; Pulaar; Pular");
    map["glg"] = QObject::tr("Galician");
    map["kat"] = QObject::tr("Georgian");
    map["deu"] = QObject::tr("German");
    map["ell"] = QObject::tr("Greek, Modern");
    map["grn"] = QObject::tr("Guaraní");
    map["guj"] = QObject::tr("Gujarati");
    map["hat"] = QObject::tr("Haitian; Haitian Creole");
    map["hau"] = QObject::tr("Hausa");
    map["heb"] = QObject::tr("Hebrew (modern)");
    map["her"] = QObject::tr("Herero");
    map["hin"] = QObject::tr("Hindi");
    map["hmo"] = QObject::tr("Hiri Motu");
    map["hun"] = QObject::tr("Hungarian");
    map["ina"] = QObject::tr("Interlingua");
    map["ind"] = QObject::tr("Indonesian");
    map["ile"] = QObject::tr("Interlingue");
    map["gle"] = QObject::tr("Irish");
    map["ibo"] = QObject::tr("Igbo");
    map["ipk"] = QObject::tr("Inupiaq");
    map["ido"] = QObject::tr("Ido");
    map["isl"] = QObject::tr("Icelandic");
    map["ita"] = QObject::tr("Italian");
    map["iku"] = QObject::tr("Inuktitut");
    map["jpn"] = QObject::tr("Japanese");
    map["jav"] = QObject::tr("Javanese");
    map["kal"] = QObject::tr("Kalaallisut, Greenlandic");
    map["kan"] = QObject::tr("Kannada");
    map["kau"] = QObject::tr("Kanuri");
    map["kas"] = QObject::tr("Kashmiri");
    map["kaz"] = QObject::tr("Kazakh");
    map["khm"] = QObject::tr("Khmer");
    map["kik"] = QObject::tr("Kikuyu, Gikuyu");
    map["kin"] = QObject::tr("Kinyarwanda");
    map["kir"] = QObject::tr("Kirghiz, Kyrgyz");
    map["kom"] = QObject::tr("Komi");
    map["kon"] = QObject::tr("Kongo");
    map["kor"] = QObject::tr("Korean");
    map["kur"] = QObject::tr("Kurdish");
    map["kua"] = QObject::tr("Kwanyama, Kuanyama");
    map["lat"] = QObject::tr("Latin");
    map["ltz"] = QObject::tr("Luxembourgish, Letzeburgesch");
    map["lug"] = QObject::tr("Luganda");
    map["lim"] = QObject::tr("Limburgish, Limburgan, Limburger");
    map["lin"] = QObject::tr("Lingala");
    map["lao"] = QObject::tr("Lao");
    map["lit"] = QObject::tr("Lithuanian");
    map["lub"] = QObject::tr("Luba-Katanga");
    map["lav"] = QObject::tr("Latvian");
    map["glv"] = QObject::tr("Manx");
    map["mkd"] = QObject::tr("Macedonian");
    map["mlg"] = QObject::tr("Malagasy");
    map["msa"] = QObject::tr("Malay");
    map["mal"] = QObject::tr("Malayalam");
    map["mlt"] = QObject::tr("Maltese");
    map["mri"] = QObject::tr("Māori");
    map["mar"] = QObject::tr("Marathi (Marāṭhī)");
    map["mah"] = QObject::tr("Marshallese");
    map["mon"] = QObject::tr("Mongolian");
    map["nau"] = QObject::tr("Nauru");
    map["nav"] = QObject::tr("Navajo, Navaho");
    map["nob"] = QObject::tr("Norwegian Bokmål");
    map["nde"] = QObject::tr("North Ndebele");
    map["nep"] = QObject::tr("Nepali");
    map["ndo"] = QObject::tr("Ndonga");
    map["nno"] = QObject::tr("Norwegian Nynorsk");
    map["nor"] = QObject::tr("Norwegian");
    map["iii"] = QObject::tr("Nuosu");
    map["nbl"] = QObject::tr("South Ndebele");
    map["oci"] = QObject::tr("Occitan");
    map["oji"] = QObject::tr("Ojibwe, Ojibwa");
    map["chu"] = QObject::tr("Old Church Slavonic, Church Slavic, Church Slavonic, Old Bulgarian, Old Slavonic");
    map["orm"] = QObject::tr("Oromo");
    map["ori"] = QObject::tr("Oriya");
    map["oss"] = QObject::tr("Ossetian, Ossetic");
    map["pan"] = QObject::tr("Panjabi, Punjabi");
    map["pli"] = QObject::tr("Pāli");
    map["fas"] = QObject::tr("Persian");
    map["pol"] = QObject::tr("Polish");
    map["pus"] = QObject::tr("Pashto, Pushto");
    map["por"] = QObject::tr("Portuguese");
    map["que"] = QObject::tr("Quechua");
    map["roh"] = QObject::tr("Romansh");
    map["run"] = QObject::tr("Kirundi");
    map["ron"] = QObject::tr("Romanian, Moldavian, Moldovan");
    map["rus"] = QObject::tr("Russian");
    map["san"] = QObject::tr("Sanskrit (Saṁskṛta)");
    map["srd"] = QObject::tr("Sardinian");
    map["snd"] = QObject::tr("Sindhi");
    map["sme"] = QObject::tr("Northern Sami");
    map["smo"] = QObject::tr("Samoan");
    map["sag"] = QObject::tr("Sango");
    map["srp"] = QObject::tr("Serbian");
    map["gla"] = QObject::tr("Scottish Gaelic; Gaelic");
    map["sna"] = QObject::tr("Shona");
    map["sin"] = QObject::tr("Sinhala, Sinhalese");
    map["slk"] = QObject::tr("Slovak");
    map["slv"] = QObject::tr("Slovene");
    map["som"] = QObject::tr("Somali");
    map["sot"] = QObject::tr("Southern Sotho");
    map["spa"] = QObject::tr("Spanish; Castilian");
    map["sun"] = QObject::tr("Sundanese");
    map["swa"] = QObject::tr("Swahili");
    map["ssw"] = QObject::tr("Swati");
    map["swe"] = QObject::tr("Swedish");
    map["tam"] = QObject::tr("Tamil");
    map["tel"] = QObject::tr("Telugu");
    map["tgk"] = QObject::tr("Tajik");
    map["tha"] = QObject::tr("Thai");
    map["tir"] = QObject::tr("Tigrinya");
    map["bod"] = QObject::tr("Tibetan Standard, Tibetan, Central");
    map["tuk"] = QObject::tr("Turkmen");
    map["tgl"] = QObject::tr("Tagalog");
    map["tsn"] = QObject::tr("Tswana");
    map["ton"] = QObject::tr("Tonga (Tonga Islands)");
    map["tur"] = QObject::tr("Turkish");
    map["tso"] = QObject::tr("Tsonga");
    map["tat"] = QObject::tr("Tatar");
    map["twi"] = QObject::tr("Twi");
    map["tah"] = QObject::tr("Tahitian");
    map["uig"] = QObject::tr("Uighur, Uyghur");
    map["ukr"] = QObject::tr("Ukrainian");
    map["urd"] = QObject::tr("Urdu");
    map["uzb"] = QObject::tr("Uzbek");
    map["ven"] = QObject::tr("Venda");
    map["vie"] = QObject::tr("Vietnamese");
    map["vol"] = QObject::tr("Volapük");
    map["wln"] = QObject::tr("Walloon");
    map["cym"] = QObject::tr("Welsh");
    map["wol"] = QObject::tr("Wolof");
    map["fry"] = QObject::tr("Western Frisian");
    map["xho"] = QObject::tr("Xhosa");
    map["yid"] = QObject::tr("Yiddish");
    map["yor"] = QObject::tr("Yoruba");
    map["zha"] = QObject::tr("Zhuang, Chuang");
    map["zul"] = QObject::tr("Zulu");
    // custom definition
    map["chi_tra"] = QObject::tr("Chinese (Traditional)");
    map["chi_sim"] = QObject::tr("Chinese (Simplified)");
    map["dan-frak"] = QObject::tr("Danish (Fraktur)");
    map["deu-frak"] = QObject::tr("German (Fraktur)");
    map["slk-frak"] = QObject::tr("Slovak (Fraktur)");
    map["swe-frak"] = QObject::tr("Swedish (Fraktur)");
    // xxxx
    map["enm"] = QObject::tr("Middle English");
    map["frk"] = QObject::tr("Frankish");
    map["frm"] = QObject::tr("Middle French");
    map["ita_old"] = QObject::tr("Italian - old");
    map["osd"] = QObject::tr("Orientation and script detection");
    map["spa_old"] = QObject::tr("Spanish - old");
    // unknown
    map["equ"] = QObject::tr("equ");

    return map[lang];
}
