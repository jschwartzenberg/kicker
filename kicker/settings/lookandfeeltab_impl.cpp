/*
 *  lookandfeeltab.cpp
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2000 Aaron J. Seigo <aseigo@olympusproject.org>
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
 */

#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QRegExp>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconeffect.h>
#include <kimageio.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include <kickerSettings.h>
#include "advancedDialog.h"
// #include "global.h"
#include "main.h"

#include "lookandfeeltab_impl.h"
#include "lookandfeeltab_impl.moc"

#include <iostream>
using namespace std;

LookAndFeelTab::LookAndFeelTab( QWidget *parent, const char* name )
  : LookAndFeelTabBase(parent, name),
    m_advDialog(0)
{
    connect(m_kmenuTile, SIGNAL(activated(int)), SIGNAL(changed()));
    connect(m_desktopTile, SIGNAL(activated(int)), SIGNAL(changed()));
    connect(m_browserTile, SIGNAL(activated(int)), SIGNAL(changed()));
    connect(m_urlTile, SIGNAL(activated(int)), SIGNAL(changed()));
    connect(m_windowListTile, SIGNAL(activated(int)), SIGNAL(changed()));

    connect(m_kmenuTile, SIGNAL(activated(int)), SLOT(kmenuTileChanged(int)));
    connect(m_desktopTile, SIGNAL(activated(int)), SLOT(desktopTileChanged(int)));
    connect(m_browserTile, SIGNAL(activated(int)), SLOT(browserTileChanged(int)));
    connect(m_urlTile, SIGNAL(activated(int)), SLOT(urlTileChanged(int)));
    connect(m_windowListTile, SIGNAL(activated(int)), SLOT(wlTileChanged(int)));

    connect(kcfg_ColorizeBackground, SIGNAL(toggled(bool)), SLOT(browseTheme()));

    connect(kcfg_BackgroundTheme->lineEdit(), SIGNAL(lostFocus()), SLOT(browseTheme()));
    kcfg_BackgroundTheme->setFilter(KImageIO::pattern(KImageIO::Reading));
    kcfg_BackgroundTheme->setCaption(i18n("Select Image File"));

    fillTileCombos();
}

void LookAndFeelTab::browseTheme()
{
    browseTheme(kcfg_BackgroundTheme->url().url());
}

void LookAndFeelTab::browseTheme(const KUrl& newtheme)
{
    browseTheme(newtheme.path());
}

void LookAndFeelTab::browseTheme(const QString& newtheme)
{
    if (newtheme.isEmpty())
    {
        kcfg_BackgroundTheme->clear();
        m_backgroundLabel->setPixmap(QPixmap());
        emit changed();
        return;
    }

    previewBackground(newtheme, true);
}

void LookAndFeelTab::launchAdvancedDialog()
{
    if (!m_advDialog)
    {
        m_advDialog = new advancedDialog(this, "advancedDialog");
        connect(m_advDialog, SIGNAL(finished()), this, SLOT(finishAdvancedDialog()));
        m_advDialog->show();
    }
    m_advDialog->setActiveWindow();
}

void LookAndFeelTab::finishAdvancedDialog()
{
    m_advDialog->delayedDestruct();
    m_advDialog = 0;
}

void LookAndFeelTab::enableTransparency(bool useTransparency)
{
    bool useBgTheme = kcfg_UseBackgroundTheme->isChecked();

    kcfg_UseBackgroundTheme->setDisabled(useTransparency);
    kcfg_BackgroundTheme->setDisabled(useTransparency || !useBgTheme);
    m_backgroundLabel->setDisabled(useTransparency || !useBgTheme);
    kcfg_ColorizeBackground->setDisabled(useTransparency || !useBgTheme);
}

void LookAndFeelTab::previewBackground(const QString& themepath, bool isNew)
{
    QString theme = themepath;
    if (theme[0] != '/')
        theme = KStandardDirs::locate("data", "kicker/" + theme);

    QImage tmpImg(theme);
    if(!tmpImg.isNull())
    {
        tmpImg = tmpImg.smoothScale(m_backgroundLabel->contentsRect().width(),
                                    m_backgroundLabel->contentsRect().height());
        if (kcfg_ColorizeBackground->isChecked())
            Plasma::colorize(tmpImg);
        theme_preview.convertFromImage(tmpImg);
        if(!theme_preview.isNull()) {
            // avoid getting changed(true) from KConfigDialogManager for the default value
            fprintf(stderr, "Disabled KickerSettings::backgroundTheme\n");
// 	    if( KickerSettings::backgroundTheme() == themepath )
//                 KickerSettings::setBackgroundTheme( theme );
            kcfg_BackgroundTheme->lineEdit()->setText(theme);
            m_backgroundLabel->setPixmap(theme_preview);
            if (isNew)
                emit changed();
            return;
        }
    }

    KMessageBox::error(this,
                       i18n("Error loading theme image file.\n\n%1\n%2",
                            theme, themepath));
    kcfg_BackgroundTheme->clear();
    m_backgroundLabel->setPixmap(QPixmap());
}

void LookAndFeelTab::load()
{
    KConfig config(KickerConfig::the()->configName());

    KConfigGroup cg(&config, "General");

    bool use_theme = kcfg_UseBackgroundTheme->isChecked();
    QString theme = kcfg_BackgroundTheme->lineEdit()->text().trimmed();

    bool transparent = kcfg_Transparent->isChecked();

    kcfg_BackgroundTheme->setEnabled(use_theme);
    m_backgroundLabel->setEnabled(use_theme);
    kcfg_ColorizeBackground->setEnabled(use_theme);
    m_backgroundLabel->clear();
    if (theme.length() > 0)
    {
        previewBackground(theme, false);
    }

    QString tile;
    KConfigGroup cgButtons(&config, "buttons");

    kmenuTileChanged(m_kmenuTile->currentItem());
    desktopTileChanged(m_desktopTile->currentItem());
    urlTileChanged(m_urlTile->currentItem());
    browserTileChanged(m_browserTile->currentItem());
    wlTileChanged(m_windowListTile->currentItem());

    if (cgButtons.readEntry("EnableTileBackground", false))
    {
        KConfigGroup cgButtonTiles(&cgButtons, "button_tiles");

        if (cgButtonTiles.readEntry("EnableKMenuTiles", false))
        {
            tile = cgButtonTiles.readEntry("KMenuTile", "solid_blue");
            m_kmenuTile->setCurrentItem(tile);
            kcfg_KMenuTileColor->setEnabled(tile == "Colorize");
        }

        if (cgButtonTiles.readEntry("EnableDesktopButtonTiles", false))
        {
            tile = cgButtonTiles.readEntry("DesktopButtonTile", "solid_orange");
            m_desktopTile->setCurrentItem(tile);
            kcfg_DesktopButtonTileColor->setEnabled(tile == "Colorize");
        }

        if (cgButtonTiles.readEntry("EnableURLTiles", false))
        {
            tile = cgButtonTiles.readEntry("URLTile", "solid_gray");
            m_urlTile->setCurrentItem(tile);
            kcfg_URLTileColor->setEnabled(tile == "Colorize");
        }

        if (cgButtonTiles.readEntry("EnableBrowserTiles", false))
        {
            tile = cgButtonTiles.readEntry("BrowserTile", "solid_green");
            m_browserTile->setCurrentItem(tile);
            kcfg_BrowserTileColor->setEnabled(tile == "Colorize");
        }

        if (cgButtonTiles.readEntry("EnableWindowListTiles", false))
        {
            tile = cgButtonTiles.readEntry("WindowListTile", "solid_green");
            m_windowListTile->setCurrentItem(tile);
            kcfg_WindowListTileColor->setEnabled(tile == "Colorize");
        }
    }
    enableTransparency( transparent );
}

void LookAndFeelTab::save()
{
    KConfig config(KickerConfig::the()->configName());

    KConfigGroup cgGeneral(&config, "General");

    KConfigGroup cgGeneralButtonTiles(&config, "button_tiles");
    bool enableTiles = false;
    int tile = m_kmenuTile->currentItem();
    if (tile > 0)
    {
        enableTiles = true;
        cgGeneralButtonTiles.writeEntry("EnableKMenuTiles", true);
        cgGeneralButtonTiles.writeEntry("KMenuTile", m_tilename[m_kmenuTile->currentItem()]);
    }
    else
    {
        cgGeneralButtonTiles.writeEntry("EnableKMenuTiles", false);
    }

    tile = m_desktopTile->currentItem();
    if (tile > 0)
    {
        enableTiles = true;
        cgGeneralButtonTiles.writeEntry("EnableDesktopButtonTiles", true);
        cgGeneralButtonTiles.writeEntry("DesktopButtonTile", m_tilename[m_desktopTile->currentItem()]);
    }
    else
    {
        cgGeneralButtonTiles.writeEntry("EnableDesktopButtonTiles", false);
    }

    tile = m_urlTile->currentItem();
    if (tile > 0)
    {
        enableTiles = true;
        cgGeneralButtonTiles.writeEntry("EnableURLTiles", tile > 0);
        cgGeneralButtonTiles.writeEntry("URLTile", m_tilename[m_urlTile->currentItem()]);
    }
    else
    {
        cgGeneralButtonTiles.writeEntry("EnableURLTiles", false);
    }

    tile = m_browserTile->currentItem();
    if (tile > 0)
    {
        enableTiles = true;
        cgGeneralButtonTiles.writeEntry("EnableBrowserTiles", tile > 0);
        cgGeneralButtonTiles.writeEntry("BrowserTile", m_tilename[m_browserTile->currentItem()]);
    }
    else
    {
        cgGeneralButtonTiles.writeEntry("EnableBrowserTiles", false);
    }

    tile = m_windowListTile->currentItem();
    if (tile > 0)
    {
        enableTiles = true;
        cgGeneralButtonTiles.writeEntry("EnableWindowListTiles", tile > 0);
        cgGeneralButtonTiles.writeEntry("WindowListTile", m_tilename[m_windowListTile->currentItem()]);
    }
    else
    {
        cgGeneralButtonTiles.writeEntry("EnableWindowListTiles", false);
    }

    KConfigGroup buttons(&cgGeneralButtonTiles, "buttons");
    buttons.writeEntry("EnableTileBackground", enableTiles);

    config.sync();
}

void LookAndFeelTab::defaults()
{
    m_kmenuTile->setCurrentItem(0);
    m_urlTile->setCurrentItem(0);
    m_browserTile->setCurrentItem(0);
    m_windowListTile->setCurrentItem(0);
    m_desktopTile->setCurrentItem(0);

    kcfg_KMenuTileColor->setEnabled(false);
    kcfg_URLTileColor->setEnabled(false);
    kcfg_DesktopButtonTileColor->setEnabled(false);
    kcfg_BrowserTileColor->setEnabled(false);
    kcfg_WindowListTileColor->setEnabled(false);

    m_backgroundLabel->clear();

    kcfg_BackgroundTheme->setEnabled(true);
    m_backgroundLabel->setEnabled(true);
    kcfg_ColorizeBackground->setEnabled(true);
    previewBackground(kcfg_BackgroundTheme->lineEdit()->text(), false);
}

void LookAndFeelTab::fillTileCombos()
{
/*  m_kmenuTile->clear();
  m_kmenuTile->insertItem(i18n("Default"));
  m_desktopTile->clear();
  m_desktopTile->insertItem(i18n("Default"));
  m_urlTile->clear();
  m_urlTile->insertItem(i18n("Default"));
  m_browserTile->clear();
  m_browserTile->insertItem(i18n("Default"));
  m_windowListTile->clear();
  m_windowListTile->insertItem(i18n("Default"));*/
  m_tilename.clear();
  m_tilename << "" << "Colorize";

  QStringList list = KGlobal::dirs()->findAllResources("tiles","*_tiny_up.png");
  int minHeight = 0;

  for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
  {
    QString tile = (*it);
    QPixmap pix(tile);
    QFileInfo fi(tile);
    tile = fi.fileName();
    tile.truncate(tile.find("_tiny_up.png"));
    m_tilename << tile;

    // Transform tile to words with title case
    // The same is done when generating messages for translation
    QStringList words = tile.split( "[_ ]");
    for (QStringList::iterator w = words.begin(); w != words.end(); ++w)
      (*w)[0] = (*w)[0].toUpper();
    tile = i18n(words.join(" ").toUtf8());

    m_kmenuTile->insertItem(pix, tile);
    m_desktopTile->insertItem(pix, tile);
    m_urlTile->insertItem(pix, tile);
    m_browserTile->insertItem(pix, tile);
    m_windowListTile->insertItem(pix, tile);

    if (pix.height() > minHeight)
    {
        minHeight = pix.height();
    }
  }

  minHeight += 6;
  m_kmenuTile->setMinimumHeight(minHeight);
  m_desktopTile->setMinimumHeight(minHeight);
  m_urlTile->setMinimumHeight(minHeight);
  m_browserTile->setMinimumHeight(minHeight);
  m_windowListTile->setMinimumHeight(minHeight);
}

void LookAndFeelTab::kmenuTileChanged(int i)
{
    kcfg_KMenuTileColor->setEnabled(i == 1);
}

void LookAndFeelTab::desktopTileChanged(int i)
{
    kcfg_DesktopButtonTileColor->setEnabled(i == 1);
}

void LookAndFeelTab::browserTileChanged(int i)
{
    kcfg_BrowserTileColor->setEnabled(i == 1);
}

void LookAndFeelTab::urlTileChanged(int i)
{
    kcfg_URLTileColor->setEnabled(i == 1);
}

void LookAndFeelTab::wlTileChanged(int i)
{
    kcfg_WindowListTileColor->setEnabled(i == 1);
}
