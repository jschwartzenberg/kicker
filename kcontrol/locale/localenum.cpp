/*
 * localenum.cpp
 *
 * Copyright (c) 1999 Hans Petter Bieker <bieker@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>

#include <kglobal.h>

#define KLocaleConfigAdvanced KLocaleConfigNumber
#include <klocale.h>
#undef KLocaleConfigAdvanced

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "main.h"
#include "localenum.h"
#include "localenum.moc"

#define i18n(a) (a)

extern KLocale *locale;

KLocaleConfigNumber::KLocaleConfigNumber(QWidget *parent, const char*name)
 : QWidget(parent, name)
{
  QLabel *label;

  // Numbers
  QGridLayout *tl1 = new QGridLayout(this, 1, 1, 10, 5);
  tl1->setColStretch(2, 1); 

  label = new QLabel("1", this, i18n("Decimal symbol"));
  edDecSym = new QLineEdit(this);
  connect( edDecSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDecSymChanged(const QString &) ) );
  tl1->addWidget(label, 0, 1);
  tl1->addWidget(edDecSym, 0, 2);

  label = new QLabel("1", this, i18n("Thousands separator"));
  edThoSep = new QLineEdit(this);
  connect( edThoSep, SIGNAL( textChanged(const QString &) ), this, SLOT( slotThoSepChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edThoSep, 1, 2);

  label = new QLabel("1", this, i18n("Positive sign"));
  edMonPosSign = new QLineEdit(this);
  connect( edMonPosSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonPosSignChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edMonPosSign, 2, 2);

  label = new QLabel("1", this, i18n("Negative sign"));
  edMonNegSign = new QLineEdit(this);
  connect( edMonNegSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonNegSignChanged(const QString &) ) );
  tl1->addWidget(label, 3, 1);
  tl1->addWidget(edMonNegSign, 3, 2);

  tl1->setRowStretch(4, 1);

  load();
}

KLocaleConfigNumber::~KLocaleConfigNumber()
{
}

void KLocaleConfigNumber::load()
{
  edDecSym->setText(locale->_decimalSymbol);
  edThoSep->setText(locale->_thousandsSeparator);
  edMonPosSign->setText(locale->_positiveSign);
  edMonNegSign->setText(locale->_negativeSign);
}

void KLocaleConfigNumber::save()
{
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");
  KSimpleConfig ent(locate("locale", "l10n/" + locale->number + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;

  str = ent.readEntry("DecimalSymbol", ".");
  str = str==locale->_decimalSymbol?QString::null:locale->_decimalSymbol;
  config->writeEntry("DecimalSymbol", str, true, true);

  str = ent.readEntry("ThousandsSeparator", ",");
  str = str==locale->_thousandsSeparator?QString::null:"$0"+locale->_thousandsSeparator+"$0";
  config->writeEntry("ThousandsSeparator", str, true, true);

  config->sync();
}

void KLocaleConfigNumber::defaults()
{
  reset();
}

void KLocaleConfigNumber::slotDecSymChanged(const QString &t)
{
  locale->_decimalSymbol = t;
  emit resample();
}

void KLocaleConfigNumber::slotThoSepChanged(const QString &t)
{
  locale->_thousandsSeparator = t;
  emit resample();
}

void KLocaleConfigNumber::slotMonPosSignChanged(const QString &t)
{
  locale->_positiveSign = t;
  emit resample();
}

void KLocaleConfigNumber::slotMonNegSignChanged(const QString &t)
{
  locale->_negativeSign = t;
  emit resample();
}

void KLocaleConfigNumber::reset()
{
  KSimpleConfig ent(locate("locale", "l10n/" + locale->number + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;

  locale->_decimalSymbol = ent.readEntry("DecimalSymbol", ".");
  locale->_thousandsSeparator = ent.readEntry("ThousandsSeparator", ",");
  locale->_positiveSign = ent.readEntry("PositiveSign");
  locale->_negativeSign = ent.readEntry("NegativeSign", "-");

  load();
}
