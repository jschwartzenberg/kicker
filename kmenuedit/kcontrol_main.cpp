/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org>
 *   Copyright (C) 2004 Waldo Bastian <bastian@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <unistd.h>

#include <kuniqueapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>

#include "kmenuedit.h"

static const char description[] = I18N_NOOP("KDE control center editor");
static const char version[] = "1.0";

extern "C" int KDE_EXPORT kdemain( int argc, char **argv )
{
    KAboutData aboutData("kcontroledit", "kmenuedit", ki18n("KDE Control Center Editor"),
			 version, ki18n(description), KAboutData::License_GPL,
			 ki18n("(C) 2000-2004, Waldo Bastian, Raffaele Sandrini, Matthias Elter"));
    aboutData.addAuthor(ki18n("Waldo Bastian"), ki18n("Maintainer"), "bastian@kde.org");
    aboutData.addAuthor(ki18n("Raffaele Sandrini"), ki18n("Previous Maintainer"), "sandrini@kde.org");
    aboutData.addAuthor(ki18n("Matthias Elter"), ki18n("Original Author"), "elter@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start()) 
	return 1;

    KUniqueApplication app;

    KMenuEdit *menuEdit = new KMenuEdit(true);
    menuEdit->show();

    app.setMainWidget(menuEdit);
    return  app.exec();
}
