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

*/

#include "eventview.h"
#include "eventview.moc"

#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include "eventconfig.h"


#include <iostream.h>
EventView::EventView(QWidget *parent, const char *name):
	QWidget(parent, name), event(0), localEvent(0), oldListItem(-1)
{
	QGridLayout *layout=new QGridLayout(this,2,4);
	
	static QStringList presentation;
	presentation << i18n("Sound")
	             << i18n("MessageBox")
	             << i18n("Log File")
	             << i18n("Standard Error");

	eventslist=new QListBox(this);
	eventslist->insertStringList(presentation);
	
	layout->addMultiCellWidget(eventslist, 0,3, 0,0);
	layout->addWidget(enabled=new QCheckBox(i18n("&Enabled"),this), 0,1);
	layout->addWidget(file=new KLineEdit(this), 2,1);
	layout->addWidget(new QLabel(file, i18n("&File:"), this), 1,1);
	layout->addWidget(todefault=new QPushButton(i18n("&Default"), this), 3,1);
	
	file->setEnabled(false);
	connect(eventslist, SIGNAL(highlighted(int)), SLOT(itemSelected(int)));
	connect(enabled, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
	connect(file, SIGNAL(textChanged(QString)), SLOT(changed(textChanged(QString))));
};

EventView::~EventView()
{
}

void EventView::defaults()
{
//	emit changed();
	unload();
}

void EventView::textChanged(const QString &str)
{
	(void)str;
}

void EventView::itemSelected(int item)
{
	file->setEnabled(false);
	// charger la nouvelle chose
	
	enabled->setChecked((localEvent->present & enumNum(item)) ? true : false);
	if (enumNum(item) == KNotifyClient::Sound)
		file->setEnabled(true), file->setText(localEvent->soundfile);
	
	if (enumNum(item) == KNotifyClient::Logfile)
		file->setEnabled(true), file->setText(localEvent->logfile);
	oldListItem=item;
}

void EventView::itemToggled(bool on)
{
	if (!localEvent) return;
	if (on)
		localEvent->present|=enumNum(eventslist->currentItem());
	else
		localEvent->present&= ~enumNum(eventslist->currentItem());
	setPixmaps();
}

void EventView::load(EventConfig *_event)
{
	unload();
	event=_event;
	localEvent=new EventConfig(_event);
	setEnabled(true);
	setPixmaps();
	eventslist->setSelected(0, true);
	kapp->processEvents();
	eventslist->setContentsPos(0,0); // go to the top
	itemSelected(0);
}

void EventView::setPixmaps()
{ // Handle all of 'dem at once
	int current=eventslist->currentItem();
	eventslist->blockSignals(true);
	if (!localEvent) return;
	int i=0;
	for (int c=1; c <=8; c*=2)
	{
		if (localEvent->present & c)
			eventslist->changeItem(SmallIcon("flag"), eventslist->text(i), i);
		else
			eventslist->changeItem(eventslist->text(i), i);
		i++;
	}
	eventslist->setCurrentItem(current);
	eventslist->blockSignals(false);
}

void EventView::unload(bool save)
{
	if (save && localEvent)
		event->set(localEvent);
	if (localEvent)
		delete localEvent;

	localEvent=0;
	
	event=0;
	enabled->setChecked(false);
	setPixmaps();
	file->setText("");
	file->setEnabled(false);
}

int EventView::listNum(int enumNum)
{
	switch (enumNum)
	{
	case (1): return 0;
	case (2): return 1;
	case (4): return 2;
	case (8): return 3;
	default: return 1;
	}
}

int EventView::enumNum(int listNum)
{
	switch (listNum)
	{
	case (0): return 1;
	case (1): return 2;
	case (2): return 4;
	case (3): return 8;
	default: return 0;
	}
}
