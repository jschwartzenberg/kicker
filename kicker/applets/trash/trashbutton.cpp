/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "trashbutton.h"

#include <QMenu>


#include <klocale.h>
#include <kmenu.h>
#include <krun.h>
#include <kstandardaction.h>

#include <kio/netaccess.h>

#include <konq_operations.h>
#include <konq_popupmenu.h>

TrashButton::TrashButton(QWidget *parent)
	: PanelPopupButton(parent), mActions(this),
	  mFileItem()
{
	mActions.associateWidget(this);

	KIO::UDSEntry entry;
	KIO::NetAccess::stat(KUrl("trash:/"), entry, 0L);
	mFileItem = KFileItem(entry, KUrl("trash:/"));

	QAction *a = KStandardAction::paste(this, SLOT(slotPaste()), this);
        mActions.addAction("paste", a);
	a->setShortcut(0);

	move(0, 0);
	resize(20, 20);

	setTitle(i18n("Trash"));
	setIcon( "user-trash" );

	setAcceptDrops(true);

	// Activate this code only if we find a way to have both an
	// action and a popup menu for the same kicker button
	//connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));

	setPopup(new QMenu());
}

TrashButton::~TrashButton()
{
}

void TrashButton::setItemCount(int count)
{
    if (count==0)
    {
        setIcon( "user-trash" );
        this->setToolTip( i18n("Empty"));
    }
    else
    {
        setIcon( "user-trash-full" );
        this->setToolTip( i18np("One item", "%1 items", count));
    }
}

void TrashButton::initPopup()
{
	QMenu *old_popup = popup();

	KFileItemList items;
	items.append(mFileItem);

	KParts::BrowserExtension::PopupFlags kpf =
		  KParts::BrowserExtension::ShowProperties;

	KParts::BrowserExtension::PopupFlags bef =
		  KParts::BrowserExtension::DefaultPopupItems;

	KonqPopupMenu *new_popup = new KonqPopupMenu(items,
	                                   KUrl("trash:/"), mActions, 0L,
	                                   kpf, bef, this);
//	KPopupTitle *title = new KPopupTitle(new_popup);
//	title->setTitle(i18n("Trash"));

//	new_popup->insertItem(title, -1, 0);
        new_popup->insertItem(i18n("Trash"), -1, 0);

	setPopup(new_popup);

	if (old_popup!=0L) delete old_popup;
}

// Activate this code only if we find a way to have both an
// action and a popup menu for the same kicker button
/*
void TrashButton::slotClicked()
{
	mFileItem.run();
}
*/

void TrashButton::slotPaste()
{
	KonqOperations::doPaste(this, mFileItem.url());
}

void TrashButton::dragEnterEvent(QDragEnterEvent* e)
{
	e->setAccepted(true);
}

void TrashButton::dropEvent(QDropEvent *e)
{
	KonqOperations::doDrop(mFileItem, mFileItem.url(), e, this);
}

QString TrashButton::tileName()
{
	return mFileItem.name();
}

void TrashButton::setPanelPosition(Plasma::Position position)
{
	switch(position)
	{
	case Plasma::Bottom:
		setPopupDirection(Plasma::Up);
		break;
	case Plasma::Top:
		setPopupDirection(Plasma::Down);
		break;
	case Plasma::Right:
		setPopupDirection(Plasma::Left);
		break;
	case Plasma::Left:
		setPopupDirection(Plasma::Right);
		break;
	case Plasma::Floating:
		setPopupDirection(Plasma::Floating);
		break;
	}
}

#include "trashbutton.moc"
