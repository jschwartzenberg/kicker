/* 

    $Id$

    Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Log$
    Revision 1.1  2000/03/19 01:32:22  charles
    A rediculously early commit so that I can rm -rf all I want :)
    and, btw, applnk/Settings/System/Makefile.am is unchanged :)

    This is all for the sake of KNotify.

    I'm gonna finish this a lot sooner than I thought I would!

*/  


#ifndef _KNOTIFY_H
#define _KNOTIFY_H

#include "kcmodule.h"

#include <qstringlist.h>
#include <qstring.h>
#include <qlistview.h>
#include <qcheckbox.h>

#include "eventview.h"

class KNotifyWidget : public KCModule
{
Q_OBJECT

public:
	KNotifyWidget(QWidget *parent, const char *name);
	virtual ~KNotifyWidget();

	void defaults();

private slots:
	void changed();

protected:
	QListView *apps;
	QListView *events;
	EventView *eventview;

};

#endif
