/*
 * main.cpp
 *
 * Copyright (c) 2000 Yves Arrouye <yves@realnames.com>
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

#include <unistd.h>

#include <qlayout.h>

#include <kapp.h>
#include <dcopclient.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kurifilter.h>

#include "filteropts.h"
#include "main.h"

class FilterOptions;

KURIFilterModule::KURIFilterModule(QWidget *parent, const char *name)
    : KCModule(parent, name) {

    filter = KURIFilter::self();

    QVBoxLayout *layout = new QVBoxLayout(this);
    tab = new QTabWidget(this);
    layout->addWidget(tab);

#if 0
    opts = new FilterOptions(this);
    tab->addTab(opts, i18n("&Filters"));
    connect(opts, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
#endif

    modules.setAutoDelete(true);

    QListIterator<KURIFilterPlugin> it = filter->pluginsIterator();
    for (; it.current(); ++it) {
	KCModule *module = it.current()->configModule(this, 0);
	if (module) {
	    modules.append(module);

	    tab->addTab(module, it.current()->configName());
	    connect(module, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
	}
    }

    // Since there's nothing in the options tab yet, show the first plugin.

    KCModule *first = modules.first();
    if (first) {
	tab->showPage(first);
    }

    load();
}

void KURIFilterModule::load() {
    QListIterator<KCModule> it(modules);
    for (; it.current(); ++it) {
	KCModule *module = it.current();
	if (module) {
	    module->load();
	}
    }
}

void KURIFilterModule::save() {
    QListIterator<KCModule> it(modules);
    for (; it.current(); ++it) {
	KCModule *module = it.current();
	if (module) {
	    module->save();
	}
    }
}

void KURIFilterModule::defaults() {
    QListIterator<KCModule> it(modules);
    for (; it.current(); ++it) {
	KCModule *module = it.current();
	if (module) {
	    module->defaults();
	}
    }
}

void KURIFilterModule::moduleChanged(bool state) {
    emit changed(state);
}

QString KURIFilterModule::quickHelp() const
{
    return i18n("<h1>Enhanced Browsing</h1> In this module you can configure some special features"
      " the Konqueror browser offers. <h2>Internet Keywords</h2> Using internet keywords is a very"
      " quick way of using web search engines. For example, you can just enter \"altavista:frobozz\""
      " or \"av:frobozz\" and konqueror will do a search query on AltaVista for you, returning all"
      " hits AltaVista found on the word \"frobozz\". Even easier: just pressing Alt-F2 (if you haven't"
      " changed this shortcut) and enter your search query into the KDE Run Command dialog.");
}

#if 0

void KURIFilterModule::resizeEvent(QResizeEvent *) {
    tab->setGeometry(0,0,width(),height());
}

#endif

extern "C" {

KCModule *create_kurifilt(QWidget *parent, const char *name) {
    KGlobal::locale()->insertCatalogue("kcmkurifilt");
    return new KURIFilterModule(parent, name);
}

}

#include "main.moc"
