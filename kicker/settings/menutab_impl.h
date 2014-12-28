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

#ifndef __menutab_impl_h__
#define __menutab_impl_h__

#include <Q3ListView>

#include "menutab.h"

class kSubMenuItem : public QObject, public Q3CheckListItem
{
    Q_OBJECT

    public:
        kSubMenuItem(Q3ListView* parent, 
                     const QString& visibleName,
                     const QString& desktopFile,
                     const QPixmap& icon,
                     bool checked);
        ~kSubMenuItem() {}

        QString desktopFile();

    Q_SIGNALS:
        void toggled(bool);

    protected:
        void stateChange(bool state);

        QString m_desktopFile;
};

class MenuTab : public MenuTabBase
{
    Q_OBJECT

public:
    MenuTab( QWidget *parent=0, const char* name=0 );

    void load();
    void save();
    void defaults();

Q_SIGNALS:
    void changed();

public Q_SLOTS:
    void launchMenuEditor();

protected:
    kSubMenuItem *m_bookmarkMenu;
    kSubMenuItem *m_quickBrowserMenu;
};

#endif

