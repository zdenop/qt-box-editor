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

#include "SettingsDialog.h"
#include "TessTools.h"
#include <QStyleFactory>

SettingsDialog::SettingsDialog(QWidget* parent, int tabIndex)
  : QDialog(parent) {
  setFixedSize(420, 370);
  setWindowTitle(tr("%1 :: Settings...").arg(SETTING_APPLICATION));

  setupUi(this);
  initSettings();
  initLangs();

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
    fontLabel->setText(tableFont.family().toLocal8Bit() +
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
    fontImageLabel->setText(imageFont.family().toLocal8Bit() +
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

  initStyles();
  if (settings.contains("GUI/Style")) {
      int styleindex = styleComboBox->findText(
            settings.value("GUI/Style").toString());
      styleComboBox->setCurrentIndex(styleindex);
  } else {
      styleComboBox->setCurrentIndex(0);
  }

  if (settings.contains("GUI/IconTheme")) {
      int themeindex = themeComboBox->findText(
            settings.value("GUI/IconTheme").toString());
      themeComboBox->setCurrentIndex(themeindex);
  } else {
      themeComboBox->setCurrentIndex(0);
  }

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
  fontLabel->setText(tableFont.family().toLocal8Bit() +
                     tr(", %1 pt").arg(tableFont.pointSize()));

  tempFont = imageFont;
  if (imageFont.pointSize() > 12)
        tempFont.setPointSize(12);
  fontImageLabel->setFont(tempFont);
  fontImageLabel->setText(imageFont.family().toLocal8Bit() +
                     tr(", %1 pt").arg(imageFont.pointSize()));

  updateColorButton(imageFontColorButton, imageFontColor);
  updateColorButton(colorRectButton, rectColor);
  updateColorButton(rectFillColorButton, rectFillColor);
  updateColorButton(colorBoxButton, boxColor);
  updateColorButton(backgroundColorButton, backgroundColor);

  // Tesseract datapath settings, langs should be set later
  if (settings.contains("Tesseract/DataPath")) {
    lnPrefix->setText(settings.value("Tesseract/DataPath").toString());
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
  settings.setValue("GUI/Style", styleComboBox->currentText());
  settings.setValue("GUI/IconTheme", themeComboBox->currentText());

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
  if (!cbLang->itemData(cbLang->currentIndex()).isNull())
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
    // we need only prefix endswith "/" and without tessdata!!!
    if (prefix_dir.contains(QRegExp("/tessdata$")))
            prefix_dir.replace(QRegExp("/tessdata$"),"/");
    else if (prefix_dir.contains(QRegExp("/tessdata/$")))
        prefix_dir.replace(QRegExp("/tessdata/$"),"/");
    if (prefix_dir.right(1) != "/" && prefix_dir != "")
            prefix_dir += "/";
    if (prefix_dir != "")
        lnPrefix->setText(prefix_dir);
    initLangs();
}

void SettingsDialog::on_pbCheck_clicked() {
    initLangs();
}

void SettingsDialog::initStyles() {
    QStringList styles =  QStyleFactory::keys();

    // Clean combobox
    int styleCount = styleComboBox->count();
    if (styleCount > 0)
      for (int i = styleCount; i >= 0; i--) {
        styleComboBox->removeItem(0);
      }

    QString style;
    foreach(style, styles)
      styleComboBox->addItem(style);
}

void SettingsDialog::initLangs() {
    QString datapath = lnPrefix->text();
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
    foreach(lang, languages) {
        if (getLangName(lang) == "") {
            cbLang->addItem(lang, lang);
        } else {
            cbLang->addItem(getLangName(lang), lang);
        }
    }
    // sort language list
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(cbLang);
    proxy->setSourceModel(cbLang->model());
    cbLang->model()->setParent(proxy);
    cbLang->setModel(proxy);
    cbLang->model()->sort(0);

    cbLang->setCurrentIndex(1);  // do not show empty combobox if there is lang
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                        SETTING_ORGANIZATION, SETTING_APPLICATION);
    if (settings.contains("Tesseract/Lang")) {
        int langindex = cbLang->findData(
              settings.value("Tesseract/Lang").toString());
        cbLang->setCurrentIndex(langindex);
    }
}

/**
 * Get Language name from ISO 639-1 code
 * http://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
 */
QString SettingsDialog::getLangName(QString lang) {
    QMap<QString,QString> map;
    map["abk"] = QObject::trUtf8("Abkhaz");
    map["aar"] = QObject::trUtf8("Afar");
    map["afr"] = QObject::trUtf8("Afrikaans");
    map["aka"] = QObject::trUtf8("Akan");
    map["sqi"] = QObject::trUtf8("Albanian");
    map["amh"] = QObject::trUtf8("Amharic");
    map["ara"] = QObject::trUtf8("Arabic");
    map["arg"] = QObject::trUtf8("Aragonese");
    map["hye"] = QObject::trUtf8("Armenian");
    map["asm"] = QObject::trUtf8("Assamese");
    map["ava"] = QObject::trUtf8("Avaric");
    map["ave"] = QObject::trUtf8("Avestan");
    map["aym"] = QObject::trUtf8("Aymara");
    map["aze"] = QObject::trUtf8("Azerbaijani");
    map["bam"] = QObject::trUtf8("Bambara");
    map["bak"] = QObject::trUtf8("Bashkir");
    map["eus"] = QObject::trUtf8("Basque");
    map["bel"] = QObject::trUtf8("Belarusian");
    map["ben"] = QObject::trUtf8("Bengali");
    map["bih"] = QObject::trUtf8("Bihari");
    map["bis"] = QObject::trUtf8("Bislama");
    map["bos"] = QObject::trUtf8("Bosnian");
    map["bre"] = QObject::trUtf8("Breton");
    map["bul"] = QObject::trUtf8("Bulgarian");
    map["mya"] = QObject::trUtf8("Burmese");
    map["cat"] = QObject::trUtf8("Catalan; Valencian");
    map["cha"] = QObject::trUtf8("Chamorro");
    map["che"] = QObject::trUtf8("Chechen");
    map["chr"] = QObject::trUtf8("Cherokee");
    map["nya"] = QObject::trUtf8("Chichewa; Chewa; Nyanja");
    map["zho"] = QObject::trUtf8("Chinese");
    map["chv"] = QObject::trUtf8("Chuvash");
    map["cor"] = QObject::trUtf8("Cornish");
    map["cos"] = QObject::trUtf8("Corsican");
    map["cre"] = QObject::trUtf8("Cree");
    map["hrv"] = QObject::trUtf8("Croatian");
    map["ces"] = QObject::trUtf8("Czech");
    map["dan"] = QObject::trUtf8("Danish");
    map["div"] = QObject::trUtf8("Divehi; Dhivehi; Maldivian;");
    map["nld"] = QObject::trUtf8("Dutch");
    map["dzo"] = QObject::trUtf8("Dzongkha");
    map["eng"] = QObject::trUtf8("English");
    map["epo"] = QObject::trUtf8("Esperanto");
    map["est"] = QObject::trUtf8("Estonian");
    map["ewe"] = QObject::trUtf8("Ewe");
    map["fao"] = QObject::trUtf8("Faroese");
    map["fij"] = QObject::trUtf8("Fijian");
    map["fin"] = QObject::trUtf8("Finnish");
    map["fra"] = QObject::trUtf8("French");
    map["ful"] = QObject::trUtf8("Fula; Fulah; Pulaar; Pular");
    map["glg"] = QObject::trUtf8("Galician");
    map["kat"] = QObject::trUtf8("Georgian");
    map["deu"] = QObject::trUtf8("German");
    map["ell"] = QObject::trUtf8("Greek, Modern");
    map["grn"] = QObject::trUtf8("Guaraní");
    map["guj"] = QObject::trUtf8("Gujarati");
    map["hat"] = QObject::trUtf8("Haitian; Haitian Creole");
    map["hau"] = QObject::trUtf8("Hausa");
    map["heb"] = QObject::trUtf8("Hebrew (modern)");
    map["her"] = QObject::trUtf8("Herero");
    map["hin"] = QObject::trUtf8("Hindi");
    map["hmo"] = QObject::trUtf8("Hiri Motu");
    map["hun"] = QObject::trUtf8("Hungarian");
    map["ina"] = QObject::trUtf8("Interlingua");
    map["ind"] = QObject::trUtf8("Indonesian");
    map["ile"] = QObject::trUtf8("Interlingue");
    map["gle"] = QObject::trUtf8("Irish");
    map["ibo"] = QObject::trUtf8("Igbo");
    map["ipk"] = QObject::trUtf8("Inupiaq");
    map["ido"] = QObject::trUtf8("Ido");
    map["isl"] = QObject::trUtf8("Icelandic");
    map["ita"] = QObject::trUtf8("Italian");
    map["iku"] = QObject::trUtf8("Inuktitut");
    map["jpn"] = QObject::trUtf8("Japanese");
    map["jav"] = QObject::trUtf8("Javanese");
    map["kal"] = QObject::trUtf8("Kalaallisut, Greenlandic");
    map["kan"] = QObject::trUtf8("Kannada");
    map["kau"] = QObject::trUtf8("Kanuri");
    map["kas"] = QObject::trUtf8("Kashmiri");
    map["kaz"] = QObject::trUtf8("Kazakh");
    map["khm"] = QObject::trUtf8("Khmer");
    map["kik"] = QObject::trUtf8("Kikuyu, Gikuyu");
    map["kin"] = QObject::trUtf8("Kinyarwanda");
    map["kir"] = QObject::trUtf8("Kirghiz, Kyrgyz");
    map["kom"] = QObject::trUtf8("Komi");
    map["kon"] = QObject::trUtf8("Kongo");
    map["kor"] = QObject::trUtf8("Korean");
    map["kur"] = QObject::trUtf8("Kurdish");
    map["kua"] = QObject::trUtf8("Kwanyama, Kuanyama");
    map["lat"] = QObject::trUtf8("Latin");
    map["ltz"] = QObject::trUtf8("Luxembourgish, Letzeburgesch");
    map["lug"] = QObject::trUtf8("Luganda");
    map["lim"] = QObject::trUtf8("Limburgish, Limburgan, Limburger");
    map["lin"] = QObject::trUtf8("Lingala");
    map["lao"] = QObject::trUtf8("Lao");
    map["lit"] = QObject::trUtf8("Lithuanian");
    map["lub"] = QObject::trUtf8("Luba-Katanga");
    map["lav"] = QObject::trUtf8("Latvian");
    map["glv"] = QObject::trUtf8("Manx");
    map["mkd"] = QObject::trUtf8("Macedonian");
    map["mlg"] = QObject::trUtf8("Malagasy");
    map["msa"] = QObject::trUtf8("Malay");
    map["mal"] = QObject::trUtf8("Malayalam");
    map["mlt"] = QObject::trUtf8("Maltese");
    map["mri"] = QObject::trUtf8("Māori");
    map["mar"] = QObject::trUtf8("Marathi (Marāṭhī)");
    map["mah"] = QObject::trUtf8("Marshallese");
    map["mon"] = QObject::trUtf8("Mongolian");
    map["nau"] = QObject::trUtf8("Nauru");
    map["nav"] = QObject::trUtf8("Navajo, Navaho");
    map["nob"] = QObject::trUtf8("Norwegian Bokmål");
    map["nde"] = QObject::trUtf8("North Ndebele");
    map["nep"] = QObject::trUtf8("Nepali");
    map["ndo"] = QObject::trUtf8("Ndonga");
    map["nno"] = QObject::trUtf8("Norwegian Nynorsk");
    map["nor"] = QObject::trUtf8("Norwegian");
    map["iii"] = QObject::trUtf8("Nuosu");
    map["nbl"] = QObject::trUtf8("South Ndebele");
    map["oci"] = QObject::trUtf8("Occitan");
    map["oji"] = QObject::trUtf8("Ojibwe, Ojibwa");
    map["chu"] = QObject::trUtf8("Old Church Slavonic, Church Slavic, Church Slavonic, Old Bulgarian, Old Slavonic");
    map["orm"] = QObject::trUtf8("Oromo");
    map["ori"] = QObject::trUtf8("Oriya");
    map["oss"] = QObject::trUtf8("Ossetian, Ossetic");
    map["pan"] = QObject::trUtf8("Panjabi, Punjabi");
    map["pli"] = QObject::trUtf8("Pāli");
    map["fas"] = QObject::trUtf8("Persian");
    map["pol"] = QObject::trUtf8("Polish");
    map["pus"] = QObject::trUtf8("Pashto, Pushto");
    map["por"] = QObject::trUtf8("Portuguese");
    map["que"] = QObject::trUtf8("Quechua");
    map["roh"] = QObject::trUtf8("Romansh");
    map["run"] = QObject::trUtf8("Kirundi");
    map["ron"] = QObject::trUtf8("Romanian, Moldavian, Moldovan");
    map["rus"] = QObject::trUtf8("Russian");
    map["san"] = QObject::trUtf8("Sanskrit (Saṁskṛta)");
    map["srd"] = QObject::trUtf8("Sardinian");
    map["snd"] = QObject::trUtf8("Sindhi");
    map["sme"] = QObject::trUtf8("Northern Sami");
    map["smo"] = QObject::trUtf8("Samoan");
    map["sag"] = QObject::trUtf8("Sango");
    map["srp"] = QObject::trUtf8("Serbian");
    map["gla"] = QObject::trUtf8("Scottish Gaelic; Gaelic");
    map["sna"] = QObject::trUtf8("Shona");
    map["sin"] = QObject::trUtf8("Sinhala, Sinhalese");
    map["slk"] = QObject::trUtf8("Slovak");
    map["slv"] = QObject::trUtf8("Slovene");
    map["som"] = QObject::trUtf8("Somali");
    map["sot"] = QObject::trUtf8("Southern Sotho");
    map["spa"] = QObject::trUtf8("Spanish; Castilian");
    map["sun"] = QObject::trUtf8("Sundanese");
    map["swa"] = QObject::trUtf8("Swahili");
    map["ssw"] = QObject::trUtf8("Swati");
    map["swe"] = QObject::trUtf8("Swedish");
    map["tam"] = QObject::trUtf8("Tamil");
    map["tel"] = QObject::trUtf8("Telugu");
    map["tgk"] = QObject::trUtf8("Tajik");
    map["tha"] = QObject::trUtf8("Thai");
    map["tir"] = QObject::trUtf8("Tigrinya");
    map["bod"] = QObject::trUtf8("Tibetan Standard, Tibetan, Central");
    map["tuk"] = QObject::trUtf8("Turkmen");
    map["tgl"] = QObject::trUtf8("Tagalog");
    map["tsn"] = QObject::trUtf8("Tswana");
    map["ton"] = QObject::trUtf8("Tonga (Tonga Islands)");
    map["tur"] = QObject::trUtf8("Turkish");
    map["tso"] = QObject::trUtf8("Tsonga");
    map["tat"] = QObject::trUtf8("Tatar");
    map["twi"] = QObject::trUtf8("Twi");
    map["tah"] = QObject::trUtf8("Tahitian");
    map["uig"] = QObject::trUtf8("Uighur, Uyghur");
    map["ukr"] = QObject::trUtf8("Ukrainian");
    map["urd"] = QObject::trUtf8("Urdu");
    map["uzb"] = QObject::trUtf8("Uzbek");
    map["ven"] = QObject::trUtf8("Venda");
    map["vie"] = QObject::trUtf8("Vietnamese");
    map["vol"] = QObject::trUtf8("Volapük");
    map["wln"] = QObject::trUtf8("Walloon");
    map["cym"] = QObject::trUtf8("Welsh");
    map["wol"] = QObject::trUtf8("Wolof");
    map["fry"] = QObject::trUtf8("Western Frisian");
    map["xho"] = QObject::trUtf8("Xhosa");
    map["yid"] = QObject::trUtf8("Yiddish");
    map["yor"] = QObject::trUtf8("Yoruba");
    map["zha"] = QObject::trUtf8("Zhuang, Chuang");
    map["zul"] = QObject::trUtf8("Zulu");
    // custom definition
    map["chi_tra"] = QObject::trUtf8("Chinese (Traditional)");
    map["chi_sim"] = QObject::trUtf8("Chinese (Simplified)");
    map["dan-frak"] = QObject::trUtf8("Danish (Fraktur)");
    map["deu-frak"] = QObject::trUtf8("German (Fraktur)");
    map["slk-frak"] = QObject::trUtf8("Slovak (Fraktur)");
    map["swe-frak"] = QObject::trUtf8("Swedish (Fraktur)");
    // xxxx
    map["enm"] = QObject::trUtf8("Middle English");
    map["frk"] = QObject::trUtf8("Frankish");
    map["frm"] = QObject::trUtf8("Middle French");
    map["ita_old"] = QObject::trUtf8("Italian - old");
    map["osd"] = QObject::trUtf8("Orientation and script detection");
    map["spa_old"] = QObject::trUtf8("Spanish - old");
    // unknown
    map["equ"] = QObject::trUtf8("equ");

    return map[lang];
}
