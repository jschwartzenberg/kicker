/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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

#include <qcheckbox.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qvalidator.h>
#include <qtoolbutton.h>
#include <qdir.h>
#include <qlistview.h>
#include <qtooltip.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include "menutab_impl.h"
#include "menutab_impl.moc"


extern int kickerconfig_screen_number;


MenuTab::MenuTab( QWidget *parent, const char* name )
  : MenuTabBase (parent, name)
{
    // connections
    connect(m_showPixmap, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_hiddenFiles, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_maxSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_maxSpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_detailedEntries, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_showBookmarks, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_showRecent, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_showQuickBrowser, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_num2ShowSpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));

    m_pRecentOrderGroup->setRadioButtonExclusive(true);
    connect(m_pRecentOrderGroup, SIGNAL(clicked(int)), SIGNAL(changed()));

    connect(m_addMenuBtn, SIGNAL(clicked()), SLOT(slotAddMenuClicked()));
    connect(m_removeMenuBtn, SIGNAL(clicked()), SLOT(slotRemoveMenuClicked()));\
    connect(m_availableMenus,SIGNAL(selectionChanged ()),SLOT(slotSelectionChanged()));

    connect(m_selectedMenus,SIGNAL(selectionChanged ()),SLOT(slotSelectionChanged()));

    // whats this help
    QWhatsThis::add(m_hiddenFiles, i18n("If this option is enabled, hidden files (i.e. files beginning "
                                        "with a dot) will be shown in the QuickBrowser menus."));

    QString maxstr = i18n("When browsing directories that contain a lot of files, the QuickBrowser "
                          "can sometimes hide your whole desktop. Here you can limit the number of "
                          "entries shown at a time in the QuickBrowser. "
                          "This is particularly useful for low screen resolutions.");

    QWhatsThis::add(m_maxSlider, maxstr);
    QWhatsThis::add(m_maxSpinBox, maxstr);

    QWhatsThis::add(m_detailedEntries, i18n("Toogle menu entry details."));

    QWhatsThis::add(m_showBookmarks, i18n("Enabling this option will make the panel show "
                                          "a bookmarks menu in your KDE menu"));

    QWhatsThis::add(m_showRecent, i18n("Enabling this option will make the panel show "
                                       "a recent documents menu in your KDE menu, containing shortcuts to "
                                       "your most recently edited documents. This assumes you've been "
                                       "using KDE applications to edit those documents, as other "
                                       "applications will not be able to take advantage of this feature."));

    QWhatsThis::add(m_showQuickBrowser, i18n("Enabling this option will show the 'Quick Browser' in your "
                                             "KDE menu, a fast way of accessing your files via submenus. "
                                             "You can also add a Quick Browser "
                                             "as a panel button, using the panel context menu."));

    QToolTip::add(m_addMenuBtn, i18n("Add selected item"));
    QToolTip::add(m_removeMenuBtn, i18n("Remove selected item"));
    QWhatsThis::add(m_availableMenus, i18n("The list of available dynamic menus that can be "
                                           "plugged into the KDE menu. Use the buttons to add "
                                           "or remove items."));
    QWhatsThis::add(m_selectedMenus, i18n("The list of selected dynamic menus that will be added "
                                          "to the KDE menu. Use the buttons to add or remove items. "));

    load();
    //at the beginning we didn't select a item
    m_addMenuBtn->setEnabled(false);
    m_removeMenuBtn->setEnabled(false);
}

void MenuTab::load()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("KMenu");

    m_showPixmap->setChecked(c->readBoolEntry("UseSidePixmap", true));

    c->setGroup("menus");

    m_maxSlider->setValue(c->readNumEntry("MaxEntries2", 30));
    m_maxSpinBox->setValue(c->readNumEntry("MaxEntries2", 30));

    m_detailedEntries->setChecked(c->readBoolEntry("DetailedMenuEntries", true));
    m_showBookmarks->setChecked(c->readBoolEntry("UseBookmarks", true));
    m_showRecent->setChecked(c->readBoolEntry("UseRecent", true));
    m_showQuickBrowser->setChecked(c->readBoolEntry("UseBrowser", true));

    m_hiddenFiles->setChecked(c->readBoolEntry("ShowHiddenFiles", false));

    m_num2ShowSpinBox->setValue(c->readNumEntry("NumVisibleEntries", 5));

    bool bRecentVsOften = c->readBoolEntry("RecentVsOften", false);
    if (bRecentVsOften)
        m_pRecent->setChecked(true);
    else
        m_pOften->setChecked(true);

    m_availableMenus->clear();
    m_selectedMenus->clear();
    QStringList ext = c->readListEntry("Extensions");
    QListView *lv(0);
    QStringList dirs = KGlobal::dirs()->findDirs("data", "kicker/menuext");
    for (QStringList::ConstIterator dit=dirs.begin(); dit!=dirs.end(); ++dit)
    {
        QDir d(*dit, "*.desktop");
        QStringList av = d.entryList();
        QListViewItem *item(0);
        for (QStringList::ConstIterator it=av.begin(); it!=av.end(); ++it)
        {
            KDesktopFile df(d.absFilePath(*it), true);
            if (qFind(ext.begin(), ext.end(), *it) == ext.end())
                lv = m_availableMenus;
            else
                lv = m_selectedMenus;
            item = new QListViewItem(lv, item, df.readName(), *it);
            item->setPixmap(0, SmallIcon(df.readIcon()));
        }
    }

    delete c;
}

void MenuTab::save()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);


    c->setGroup("KMenu");

    c->writeEntry("UseSidePixmap", m_showPixmap->isChecked());

    c->setGroup("menus");

    c->writeEntry("MaxEntries2", m_maxSlider->value());
    c->writeEntry("DetailedMenuEntries", m_detailedEntries->isChecked());
    c->writeEntry("UseBookmarks", m_showBookmarks->isChecked());
    c->writeEntry("UseRecent", m_showRecent->isChecked());
    c->writeEntry("UseBrowser", m_showQuickBrowser->isChecked());
    c->writeEntry("ShowHiddenFiles", m_hiddenFiles->isChecked());
    c->writeEntry("NumVisibleEntries", m_num2ShowSpinBox->value());

    bool bRecentVsOften = m_pRecent->isChecked();
    c->writeEntry("RecentVsOften", bRecentVsOften);

    QStringList ext;
    QListViewItem *item = m_selectedMenus->firstChild();
    while (item)
    {
        ext << item->text(1);
        item = item->nextSibling();
    }
    c->writeEntry("Extensions", ext);

    c->sync();

    delete c;
}

void MenuTab::defaults()
{
  m_showPixmap->setChecked(true);
  m_maxSlider->setValue(30);
  m_maxSpinBox->setValue(30);
  m_detailedEntries->setChecked(true);
  m_showRecent->setChecked(true);
  m_showQuickBrowser->setChecked(true);
  m_hiddenFiles->setChecked(false);
  m_showBookmarks->setChecked(true);

  m_pOften->setChecked(true);
  m_num2ShowSpinBox->setValue(5);
}

void MenuTab::slotAddMenuClicked()
{
    QListViewItem *item = m_availableMenus->currentItem();
    if (item)
    {
        QListViewItem *newItem = new QListViewItem(m_selectedMenus, m_selectedMenus->lastItem(), item->text(0), item->text(1));
        if (item->pixmap(0))
            newItem->setPixmap(0, *(item->pixmap(0)));
        delete item;

        emit changed();
    }
}

void MenuTab::slotRemoveMenuClicked()
{
    QListViewItem *item = m_selectedMenus->currentItem();
    if (item)
    {
        QListViewItem *newItem = new QListViewItem(m_availableMenus, m_availableMenus->lastItem(), item->text(0), item->text(1));
        if (item->pixmap(0))
            newItem->setPixmap(0, *(item->pixmap(0)));
        delete item;

        emit changed();
    }
}

void MenuTab::slotSelectionChanged()
{
    QListViewItem *item = m_selectedMenus->currentItem();
    m_removeMenuBtn->setEnabled(item);

    item=m_availableMenus->currentItem();
    m_addMenuBtn->setEnabled(item);
}
