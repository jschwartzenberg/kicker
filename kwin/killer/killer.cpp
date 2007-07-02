/****************************************************************************

 Copyright (C) 2003 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <QX11Info>
#include <QProcess>
#include <signal.h>

int main( int argc, char* argv[] )
    {
    KCmdLineArgs::init( argc, argv, "kwin_killer_helper", "kwin", ki18n( "KWin" ), "1.0" ,
	ki18n( "KWin helper utility" ));

    KCmdLineOptions options;
    options.add("pid <pid>", ki18n("PID of the application to terminate."));
    options.add("hostname <hostname>", ki18n("Hostname on which the application is running."));
    options.add("windowname <caption>", ki18n("Caption of the window to be terminated."));
    options.add("applicationname <name>", ki18n("Name of the application to be terminated."));
    options.add("wid <id>", ki18n("ID of resource belonging to the application."));
    options.add("timestamp <time>", ki18n("Time of user action causing killing."));
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    QString hostname = args->getOption( "hostname" );
    bool pid_ok = false;
    pid_t pid = QString( args->getOption( "pid" ) ).toULong( &pid_ok );
    QString caption = args->getOption( "windowname" );
    QString appname = args->getOption( "applicationname" );
    bool id_ok = false;
    Window id = QString( args->getOption( "wid" ) ).toULong( &id_ok );
    bool time_ok = false;
    Time timestamp =QString( args->getOption( "timestamp" ) ).toULong( &time_ok );
    args->clear();
    if( !pid_ok || pid == 0 || !id_ok || id == None || !time_ok || timestamp == CurrentTime
	|| hostname.isEmpty() || caption.isEmpty() || appname.isEmpty())
        {
	KCmdLineArgs::usageError( i18n( "This helper utility is not supposed to be called directly." ));
	return 1;
        }
    QString question = i18n(
	"<qt>Window with title \"<b>%2</b>\" is not responding. "
	"This window belongs to application <b>%1</b> (PID=%3, hostname=%4).<p>"
	"Do you wish to terminate this application? (All unsaved data in this application will be lost.)</qt>" ,
	  appname, caption, pid, QString( hostname ) );
    app.updateUserTimestamp( timestamp );
    if( KMessageBox::warningYesNoWId( id, question, QString(), KGuiItem(i18n("Terminate")), KGuiItem(i18n("Keep Running")) ) == KMessageBox::Yes )
        {    
	if( hostname != "localhost" )
            {
		  QStringList lst;
		  lst << hostname << "kill" << QString::number( pid );
		  QProcess::startDetached("xon",lst);
	    }
	else
	    ::kill( pid, SIGKILL );
	XKillClient( QX11Info::display(), id );
        }
    }
