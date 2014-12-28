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

#include <QCheckBox>
#include <QDir>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>

#include <kapplication.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kiconloader.h>
#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>

#include "main.h"

#include "menutab_impl.h"
#include "menutab_impl.moc"

kSubMenuItem::kSubMenuItem(Q3ListView* parent,
                           const QString& visibleName,
                           const QString& desktopFile,
                           const QPixmap& icon,
                           bool checked)
    : Q3CheckListItem(parent, visibleName, Q3CheckListItem::CheckBox),
      m_desktopFile(desktopFile)
{
    setPixmap(0, icon);
    setOn(checked);
}

QString kSubMenuItem::desktopFile()
{
    return m_desktopFile;
}

void kSubMenuItem::stateChange(bool state)
{
    emit toggled(state);
}

MenuTab::MenuTab( QWidget *parent, const char* name )
  : MenuTabBase (parent, name),
    m_bookmarkMenu(0),
    m_quickBrowserMenu(0)
{
    // connections
    connect(m_editKMenuButton, SIGNAL(clicked()), SLOT(launchMenuEditor()));

    m_browserGroup->setColumnLayout( 1, Qt::Horizontal );
    m_pRecentOrderGroup->setColumnLayout( 1, Qt::Horizontal );
}

void MenuTab::load()
{
    KSharedConfig::Ptr c = KSharedConfig::openConfig(KickerConfig::the()->configName());

    KConfigGroup cg(c, "menus");

    m_subMenus->clear();

    // show the bookmark menu?
    m_bookmarkMenu = new kSubMenuItem(m_subMenus,
                                      i18n("Bookmarks"),
                                      QString(),
                                      SmallIcon("bookmark"),
                                      cg.readEntry("UseBookmarks", true));
    connect(m_bookmarkMenu, SIGNAL(toggled(bool)), SIGNAL(changed()));

    // show the quick menus menu?
    m_quickBrowserMenu = new kSubMenuItem(m_subMenus,
                                          i18n("Quick Browser"),
                                          QString(),
                                          SmallIcon("kdisknav"),
                                          cg.readEntry("UseBrowser", true));
    connect(m_quickBrowserMenu, SIGNAL(toggled(bool)), SIGNAL(changed()));

    QStringList ext = cg.readEntry("Extensions", QStringList());
    QStringList dirs = KGlobal::dirs()->findDirs("data", "kicker/menuext");
    kSubMenuItem* menuItem(0);
    for (QStringList::ConstIterator dit=dirs.begin(); dit!=dirs.end(); ++dit)
    {
        QDir d(*dit, "*.desktop");
        QStringList av = d.entryList();
        for (QStringList::ConstIterator it=av.begin(); it!=av.end(); ++it)
        {
            KDesktopFile df(d.absoluteFilePath(*it));
            menuItem = new kSubMenuItem(m_subMenus,
                                        df.readName(),
                                        *it,
                                        SmallIcon(df.readIcon()),
                                        qFind(ext.begin(), ext.end(), *it) != ext.end());
            connect(menuItem, SIGNAL(toggled(bool)), SIGNAL(changed()));
        }
    }

    m_showFrequent->setChecked(true);
}

void MenuTab::save()
{
    KSharedConfig::Ptr c = KSharedConfig::openConfig(KickerConfig::the()->configName());

    KConfigGroup cg(c, "menus");

    QStringList ext;
    Q3ListViewItem *item(0);
    for (item = m_subMenus->firstChild(); item; item = item->nextSibling())
    {
        bool isOn = static_cast<kSubMenuItem*>(item)->isOn();
        if (item == m_bookmarkMenu)
        {
            cg.writeEntry("UseBookmarks", isOn);
        }
        else if (item == m_quickBrowserMenu)
        {
            cg.writeEntry("UseBrowser", isOn);
        }
        else if (isOn)
        {
            ext << static_cast<kSubMenuItem*>(item)->desktopFile();
        }
    }
    cg.writeEntry("Extensions", ext);

    cg.sync();
}

void MenuTab::defaults()
{
    //disable all
    Q3ListViewItem *item(0);
    for (item = m_subMenus->firstChild(); item; item = item->nextSibling())
    {
         static_cast<kSubMenuItem*>( item )->setOn( false );
    }
    m_bookmarkMenu->setOn(true);
    m_quickBrowserMenu->setOn(true);

    m_showFrequent->setChecked(true);
}

void MenuTab::launchMenuEditor()
{
    if ( KToolInvocation::startServiceByDesktopName( "kmenuedit",
                                                  QString() /*url*/,
                                                  0 /*error*/,
                                                  0 /*dcopservice*/,
                                                  0 /*pid*/,
                                                  "" /*startup_id*/,
                                                  true /*nowait*/ ) != 0 )
    {
        KMessageBox::error(this,
                           i18n("The KDE menu editor (kmenuedit) could not be launched.\n"
                           "Perhaps it is not installed or not in your path."),
                           i18n("Application Missing"));
    }
}
