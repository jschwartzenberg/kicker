/*
    KTip, the KDE Tip Of the Day

    Copyright (c) 2000, Matthias Hoelzer-Kluepfel
    Copyright (c) 2002 Tobias Koenig <tokoe82@yahoo.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktip.h>
#include <kuniqueapplication.h>
#include <stdlib.h>
#include <QtDBus/QtDBus>

static const char description[] = I18N_NOOP("Useful tips");

int main(int argc, char *argv[])
{
	KAboutData aboutData("ktip", 0, ki18n("KTip"),
				"0.3", ki18n(description), KAboutData::License_GPL,
				ki18n("(c) 1998-2002, KDE Developers"));
	KCmdLineArgs::init( argc, argv, &aboutData );
	KUniqueApplication::addCmdLineOptions();

	if (!KUniqueApplication::start())
		exit(-1);

	KUniqueApplication app;

	KTipDialog *tipDialog = new KTipDialog(new KTipDatabase(KStandardDirs::locate("data", QString("kdewizard/tips"))));
	Q_CHECK_PTR(tipDialog);

	tipDialog->setWindowFlags(Qt::WindowStaysOnTopHint);

	tipDialog->setCaption(i18n("Useful Tips"));
	tipDialog->show();

	QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

        return app.exec();
}
