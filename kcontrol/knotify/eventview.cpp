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


#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>

#include <iostream.h>

static QString presentation[5] = {
	i18n("Sound"),
	i18n("MessageBox"),
	i18n("Log Window"),
	i18n("Log File"),
	i18n("Standard Error")
};


EventView::EventView(QWidget *parent, const char *name):
	QWidget(parent, name), conf(0)
{
	QGridLayout *layout=new QGridLayout(this,2,3);
	
	eventslist=new QListBox(this);
	{	eventslist->insertItem(presentation[0]);
		eventslist->insertItem(presentation[1]);
		eventslist->insertItem(presentation[2]);
		eventslist->insertItem(presentation[3]);
		eventslist->insertItem(presentation[4]);
	}
	
	layout->addMultiCellWidget(eventslist, 0,2, 0,0);
	layout->addWidget(enabled=new QCheckBox(i18n("&Enabled"),this), 0,1);
	layout->addWidget(file=new KLineEdit(this), 1,1);
	layout->addWidget(todefault=new QPushButton(i18n("&Default"), this), 2,1);
	
	file->hide();
	connect(eventslist, SIGNAL(highlighted(int)), SLOT(itemSelected(int)));
};

EventView::~EventView()
{
}

void EventView::defaults()
{

}


void EventView::itemSelected(int item)
{
	enabled->setChecked(false);
	file->setEnabled(false);
	
	switch (item)
	{
	case (0):
		file->show();
		file->setText(soundfile);
		if (present & KNotifyClient::Sound)
		{
			enabled->setChecked(true);
			file->setEnabled(true);
		}
		break;
	case (1):
		if (present & KNotifyClient::Messagebox)
			enabled->setChecked(true);
		break;
	case (2):
		if (present & KNotifyClient::Logwindow)
			enabled->setChecked(true);
		break;
	case (3):
		file->show();
		file->setText(logfile);
		if (present & KNotifyClient::Logfile)
		{
			enabled->setChecked(true);
			file->setEnabled(true);
		}
		break;
	case (4):
		if (present & KNotifyClient::Stderr)
			enabled->setChecked(true);
	}
}

void EventView::load(KConfig *config, const QString &section)
{
	config->setGroup(section);
	unload();
	conf=config;
	this->section=section;
	setEnabled(true);
	typedef KNotifyClient::Presentation Presentation;
	
	{ // Load the presentation
		present=(Presentation)conf->readNumEntry("presentation", -1);
		if (present==KNotifyClient::Default)
			present=(Presentation)conf->readNumEntry("default_presentation", 0);
	}

	{ // Load the files
		soundfile=conf->readEntry("soundfile");
		if (soundfile.isNull())
			soundfile=conf->readEntry("default_soundfile");
	}
	
	{ // Load the files
		logfile=conf->readEntry("logfile");
		if (logfile.isNull())
			logfile=conf->readEntry("default_logfile");
	}	
	
	// Stick the flags on the list for that which is present
	if (present & KNotifyClient::Sound)
		setPixmap(0, true);
	if (present & KNotifyClient::Messagebox)
		setPixmap(1, true);
	if (present & KNotifyClient::Logwindow)
		setPixmap(2, true);
	if (present & KNotifyClient::Logfile)
		setPixmap(3, true);
	if (present & KNotifyClient::Stderr)
		setPixmap(4, true);
}

void EventView::setPixmap(int item, bool on)
{
// KGlobal::instance()->iconLoader()->loadIcon("toolbars/flag", KIconLoader::Small)
	if (on)
		eventslist->changeItem(
			KGlobal::instance()->iconLoader()->loadIcon("toolbars/flag", KIconLoader::Small),
			eventslist->text(item),
			item);
}

void EventView::save()
{
	unload();
}

void EventView::unload()
{
	
	delete conf;
	enabled->setChecked(false);
	file->setText("");
	setEnabled(false);
}
