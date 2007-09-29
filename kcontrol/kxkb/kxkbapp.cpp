/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
	Copyright (C) 2006, Andriy Rysin <rysin@kde.org>. Derived from an
    original by Matthias H�zer-Klpfel released under the QPL.
    This file is part of the KDE project

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

DESCRIPTION

    KDE Keyboard Tool. Manages XKB keyboard mappings.
*/
#include <assert.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>

#include "kxkb_adaptor.h"

#include "kxkbapp.h"
#include "kxkbwidget.h"

#include "kxkbapp.moc"


KXKBApp::KXKBApp(bool allowStyles, bool GUIenabled)
    : KUniqueApplication(allowStyles, GUIenabled)
{
	KxkbSysTrayIcon* kxkbWidget = new KxkbSysTrayIcon();
	m_kxkbCore = new KxkbCore( kxkbWidget );
// 	kxkbWidget->show();
    //TODO: don't do this if kxkb does not become a daemon
    new KXKBAdaptor( this );
}


KXKBApp::~KXKBApp()
{
}

int KXKBApp::newInstance()
{
	return m_kxkbCore->newInstance();
}

bool KXKBApp::settingsRead()
{
//	return m_kxkbCore->settingsRead();
    return false;
}

// This function activates the keyboard layout specified by the
// configuration members (m_currentLayout)
void KXKBApp::layoutApply()
{
//	return m_kxkbCore->layoutApply();
}


void KXKBApp::slotSettingsChanged(int category)
{
//	return m_kxkbCore->slotSettingsChanged(category);
}


const char * DESCRIPTION =
  I18N_NOOP("A utility to switch keyboard maps");

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
    KAboutData about("kxkb", 0, ki18n("KDE Keyboard Layout Switcher"), "2.0",
                     ki18n(DESCRIPTION), KAboutData::License_GPL,
                     ki18n("Copyright (C) 2006-2007 Andriy Rysin"));
    KCmdLineArgs::init(argc, argv, &about);
    KXKBApp::addCmdLineOptions();

    if (!KXKBApp::start())
        return 0;

    KXKBApp app;
    app.disableSessionManagement();
    app.exec();
    return 0;
}
