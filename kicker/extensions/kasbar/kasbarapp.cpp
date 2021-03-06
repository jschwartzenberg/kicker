/* test_kasbar.cpp
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kwindowsystem.h>
#include <kglobal.h>
#include <klocale.h>

#include "kasitem.h"
#include "kastasker.h"
#include "kasclockitem.h"
#include "kasloaditem.h"

#include "version.h"

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "kasbar", "kasbarextension", ki18n("KasBar"), VERSION_STRING , ki18n( "An alternative task manager" ));

  KCmdLineOptions options;
  options.add("test", ki18n("Test the basic kasbar code"));
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  kDebug(1345) << "Kasbar starting...";

  int wflags = Qt::WStyle_Customize | Qt::WX11BypassWM | Qt::WStyle_DialogBorder | Qt::WStyle_StaysOnTop;
  KasBar *kasbar;
  KConfig conf( "kasbarrc" );

  if ( args->isSet("test") ) {
      kasbar = new KasBar( Qt::Vertical, 0, "testkas", wflags );
      kasbar->setItemSize( KasBar::Large );
      kasbar->append( new KasClockItem(kasbar) );
      kasbar->append( new KasItem(kasbar) );
      kasbar->append( new KasLoadItem(kasbar) );
      kasbar->append( new KasItem(kasbar) );
      kasbar->addTestItems();
  }
  else {
      KasTasker *kastasker = new KasTasker( Qt::Vertical, 0, "testkas", wflags );
      kastasker->setConfig( &conf );
      kastasker->setStandAlone( true );
      kasbar = kastasker;

      kastasker->readConfig();
      kastasker->move( kastasker->detachedPosition() );
      kastasker->connect( kastasker->resources(), SIGNAL(changed()), SLOT(readConfig()) );
      kastasker->refreshAll();
  }

  kDebug(1345) << "Kasbar about to show";
  app.setMainWidget( kasbar );
  kasbar->show();

  kasbar->setDetached( true );
  KWindowSystem::setOnAllDesktops( kasbar->winId(), true );
  kDebug() << "kasbar: Window id is " << kasbar->winId();

  KApplication::kApplication()->dcopClient()->registerAs( "kasbar" );

  app.connect( &app, SIGNAL( lastWindowClosed() ), SLOT(quit()) );

  return app.exec();
}

