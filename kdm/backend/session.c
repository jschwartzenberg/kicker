/*

Copyright 1988, 1998  The Open Group
Copyright 2000-2004 Oswald Buddenhagen <ossi@kde.org>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the copyright holder.

*/

/*
 * xdm - display manager daemon
 * Author: Keith Packard, MIT X Consortium
 *
 * subdaemon event loop, etc.
 */

#include "dm.h"
#include "dm_error.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <ctype.h>
#include <signal.h>

struct display *td;
const char *td_setup = "auto";

static void deleteXloginResources( void );
static void loadXloginResources( void );
static void setupDisplay( const char *arg );


static Jmp_buf pingTime;

/* ARGSUSED */
static void
catchAlrm( int n ATTR_UNUSED )
{
	Longjmp( pingTime, 1 );
}

static Jmp_buf tenaciousClient;

/* ARGSUSED */
static void
waitAbort( int n ATTR_UNUSED )
{
	Longjmp( tenaciousClient, 1 );
}

static void
abortClient( int pid )
{
	int sig = SIGTERM;
	volatile int i;
	int retId;

	for (i = 0; i < 4; i++) {
		if (kill( -pid, sig ) == -1) {
			switch (errno) {
			case EPERM:
				logError( "Cannot kill client\n" );
			case EINVAL:
			case ESRCH:
				return;
			}
		}
		if (!Setjmp( tenaciousClient )) {
			(void)Signal( SIGALRM, waitAbort );
			(void)alarm( (unsigned)10 );
			retId = wait( 0 );
			(void)alarm( (unsigned)0 );
			(void)Signal( SIGALRM, SIG_DFL );
			if (retId == pid)
				break;
		} else
			(void)Signal( SIGALRM, SIG_DFL );
		sig = SIGKILL;
	}
}


static char *
conv_auto( int what, const char *prompt ATTR_UNUSED )
{
	switch (what) {
	case GCONV_USER:
		return curuser;
	case GCONV_PASS:
	case GCONV_PASS_ND:
		return curpass;
	default:
		logError( "Unknown authentication data type requested for autologin.\n" );
		return 0;
	}
}

static void
doAutoLogon( void )
{
	reStr( &curuser, td->autoUser );
	reStr( &curpass, td->autoPass );
	reStr( &curtype, "classic" );
	cursource = PWSRC_AUTOLOGIN;
}

static int
autoLogon( time_t tdiff )
{
	debug( "autoLogon, tdiff = %d, rLogin = %d, goodexit = %d, nuser = %s\n",
	       tdiff, td->hstent->rLogin, td->hstent->goodExit, td->hstent->nuser );
	if (td->hstent->rLogin == 2 ||
	    (td->hstent->rLogin == 1 &&
	     tdiff <= 0 && !td->hstent->goodExit && !td->hstent->lock))
	{
		curuser = td->hstent->nuser;
		td->hstent->nuser = 0;
		curpass = td->hstent->npass;
		td->hstent->npass = 0;
		newdmrc = td->hstent->nargs;
		td->hstent->nargs = 0;
		reStr( &curtype, "classic" );
		cursource = (td->hstent->rLogin == 1) ? PWSRC_RELOGIN : PWSRC_MANUAL;
		return 1;
	} else if (*td->autoUser && !td->autoDelay && (tdiff > 0 || td->autoAgain))
	{
		unsigned int lmask;
		Window dummy1, dummy2;
		int dummy3, dummy4, dummy5, dummy6;
		XQueryPointer( dpy, DefaultRootWindow( dpy ),
		               &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6,
		               &lmask );
		if (lmask & ShiftMask)
			return 0;
		doAutoLogon();
		return 1;
	}
	return 0;
}


static const struct {
  int vcode, echo, ndelay;
} grqs[] = {
	{ V_GET_TEXT, TRUE, FALSE },
	{ V_GET_TEXT, FALSE, FALSE },
	{ V_GET_TEXT, TRUE, FALSE },
	{ V_GET_TEXT, FALSE, FALSE },
	{ V_GET_TEXT, FALSE, TRUE },
	{ V_GET_BINARY, 0, 0 }
};

char *
conv_interact( int what, const char *prompt )
{
	char *ret;
	int tag;

	gSendInt( grqs[what].vcode );
	if (what == GCONV_BINARY) {
		unsigned const char *up = (unsigned const char *)prompt;
		int len = up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24);
		gSendArr( len, prompt );
		gSendInt( FALSE ); /* ndelay */
		return gRecvArr( &len );
	} else {
		gSendStr( prompt );
		gSendInt( grqs[what].echo );
		gSendInt( grqs[what].ndelay );
		ret = gRecvStr();
		if (ret) {
			tag = gRecvInt();
			switch (what) {
			case GCONV_USER:
				/* assert( tag & V_IS_USER );*/
				if (curuser)
					free( curuser );
				curuser = ret;
				break;
			case GCONV_PASS:
			case GCONV_PASS_ND:
				/* assert( tag & V_IS_PASSWORD );*/
				if (curpass)
					free( curpass );
				curpass = ret;
				break;
			default:
				if (tag & V_IS_USER)
					reStr( &curuser, ret );
				else if (tag & V_IS_PASSWORD)
					reStr( &curpass, ret );
				else if (tag & V_IS_NEWPASSWORD)
					reStr( &newpass, ret );
				else if (tag & V_IS_OLDPASSWORD)
					reStr( &ret, curpass );
			}
		}
		return ret;
	}
}

GProc grtproc;
GTalk grttalk;

GTalk mstrtalk; /* make static; see dm.c */

int
ctrlGreeterWait( int wreply )
{
	int i, cmd, type, rootok;
	char *name, *pass, **avptr;
#ifdef XDMCP
	ARRAY8Ptr aptr;
#endif

	if (Setjmp( mstrtalk.errjmp )) {
		closeGreeter( TRUE );
		sessionExit( EX_UNMANAGE_DPY );
	}

	while (gRecvCmd( &cmd )) {
		switch (cmd)
		{
		case G_Ready:
			debug( "G_Ready\n" );
			return 0;
		case G_GetCfg:
			/*debug( "G_GetCfg\n" );*/
			type = gRecvInt();
			/*debug( " index %#x\n", type );*/
			if (type == C_isLocal)
				i = (td->displayType & d_location) == dLocal;
			else if (type == C_isReserve)
				i = (td->displayType & d_lifetime) == dReserve;
			else if (type == C_hasConsole)
#ifdef HAVE_VTS
				i = *consoleTTYs != 0;
#else
				i = td->console != 0;
#endif
			else if (type == C_isAuthorized)
				i = td->authorizations != 0;
			else
				goto normal;
			gSendInt( GE_Ok );
			/*debug( " -> bool %d\n", i );*/
			gSendInt( i );
			break;
		  normal:
			if (!(avptr = findCfgEnt( td, type ))) {
				/*debug( " -> not found\n" );*/
				gSendInt( GE_NoEnt );
				break;
			}
			switch (type & C_TYPE_MASK) {
			default:
				/*debug( " -> unknown type\n" );*/
				gSendInt( GE_BadType );
				break;
			case C_TYPE_INT:
			case C_TYPE_STR:
			case C_TYPE_ARGV:
#ifdef XDMCP
			case C_TYPE_ARR:
#endif
				gSendInt( GE_Ok );
				switch (type & C_TYPE_MASK) {
				case C_TYPE_INT:
					/*debug( " -> int %#x (%d)\n", *(int *)avptr, *(int *)avptr );*/
					gSendInt( *(long *)avptr );
					break;
				case C_TYPE_STR:
					/*debug( " -> string %\"s\n", *avptr );*/
					gSendStr( *avptr );
					break;
				case C_TYPE_ARGV:
					/*debug( " -> sending argv %\"[{s\n", *(char ***)avptr );*/
					gSendArgv( *(char ***)avptr );
					break;
#ifdef XDMCP
				case C_TYPE_ARR:
					aptr = *(ARRAY8Ptr *)avptr;
					/*debug( " -> sending array %02[*:hhx\n",
					         aptr->length, aptr->data );*/
					gSendArr( aptr->length, (char *)aptr->data );
					break;
#endif
				}
				break;
			}
			break;
		case G_ReadDmrc:
			debug( "G_ReadDmrc\n" );
			name = gRecvStr();
			debug( " user %\"s\n", name );
			if (strCmp( dmrcuser, name )) {
				if (curdmrc) { free( curdmrc ); curdmrc = 0; }
				if (dmrcuser)
					free( dmrcuser );
				dmrcuser = name;
				i = readDmrc();
				debug( " -> status %d\n", i );
				gSendInt( i );
				debug( " => %\"s\n", curdmrc );
			} else {
				if (name)
					free( name );
				debug( " -> status " stringify( GE_Ok ) "\n" );
				gSendInt( GE_Ok );
				debug( " => keeping old\n" );
			}
			break;
		case G_GetDmrc:
			debug( "G_GetDmrc\n" );
			name = gRecvStr();
			debug( " key %\"s\n", name );
			pass = iniEntry( curdmrc, "Desktop", name, 0 );
			debug( " -> %\"s\n", pass );
			gSendStr( pass );
			if (pass)
				free( pass );
			free( name );
			break;
/*		case G_ResetDmrc:
			debug( "G_ResetDmrc\n" );
			if (newdmrc) { free( newdmrc ); newdmrc = 0; }
			break; */
		case G_PutDmrc:
			debug( "G_PutDmrc\n" );
			name = gRecvStr();
			debug( " key %\"s\n", name );
			pass = gRecvStr();
			debug( " value %\"s\n", pass );
			newdmrc = iniEntry( newdmrc, "Desktop", name, pass );
			free( pass );
			free( name );
			break;
		case G_VerifyRootOK:
			debug( "G_VerifyRootOK\n" );
			rootok = TRUE;
			goto doverify;
		case G_Verify:
			debug( "G_Verify\n" );
			rootok = FALSE;
		  doverify:
			if (curuser) { free( curuser ); curuser = 0; }
			if (curpass) { free( curpass ); curpass = 0; }
			if (curtype) free( curtype );
			curtype = gRecvStr();
			debug( " type %\"s\n", curtype );
			cursource = PWSRC_MANUAL;
			if (verify( conv_interact, rootok )) {
				debug( " -> return success\n" );
				gSendInt( V_OK );
			} else
				debug( " -> failure returned\n" );
			break;
		case G_AutoLogin:
			debug( "G_AutoLogin\n" );
			doAutoLogon();
			if (verify( conv_auto, FALSE )) {
				debug( " -> return success\n" );
				gSendInt( V_OK );
			} else
				debug( " -> failure returned\n" );
			break;
		case G_SetupDpy:
			debug( "G_SetupDpy\n" );
			setupDisplay( 0 );
			td_setup = 0;
			gSendInt( 0 );
			break;
		default:
			return cmd;
		}
		if (!wreply)
			return -1;
	}
	debug( "lost connection to greeter\n" );
	return -2;
}

void
openGreeter()
{
	char *name, **env;
	static time_t lastStart;
	int cmd;
	Cursor xcursor;

	gSet( &grttalk );
	if (grtproc.pid > 0)
		return;
	if (time( 0 ) < lastStart + 10) /* XXX should use some readiness indicator instead */
		sessionExit( EX_UNMANAGE_DPY );
	ASPrintf( &name, "greeter for display %s", td->name );
	debug( "starting %s\n", name );

	/* Hourglass cursor */
	if ((xcursor = XCreateFontCursor( dpy, XC_watch ))) {
		XDefineCursor( dpy, DefaultRootWindow( dpy ), xcursor );
		XFreeCursor( dpy, xcursor );
	}
	XFlush( dpy );

	/* Load system default Resources (if any) */
	loadXloginResources();

	grttalk.pipe = &grtproc.pipe;
	env = systemEnv( (char *)0 );
	if (gOpen( &grtproc, (char **)0, "_greet", env, name, &td->gpipe ))
		sessionExit( EX_UNMANAGE_DPY );
	freeStrArr( env );
	if ((cmd = ctrlGreeterWait( TRUE ))) {
		logError( "Received unknown or unexpected command %d from greeter\n", cmd );
		closeGreeter( TRUE );
		sessionExit( EX_UNMANAGE_DPY );
	}
	debug( "%s ready\n", name );
	time( &lastStart );
}

int
closeGreeter( int force )
{
	int ret;

	if (grtproc.pid <= 0)
		return EX_NORMAL;

	ret = gClose( &grtproc, 0, force );
	debug( "greeter for %s stopped\n", td->name );
	if (wcCode( ret ) > EX_NORMAL && wcCode( ret ) <= EX_MAX) {
		debug( "greeter-initiated session exit, code %d\n", wcCode( ret ) );
		sessionExit( wcCode( ret ) );
	}

	deleteXloginResources();

	return ret;
}

void
prepareErrorGreet()
{
	if (grtproc.pid <= 0) {
		openGreeter();
		gSendInt( G_ErrorGreet );
		gSendStr( curuser );
	}
	gSet( &grttalk );
}

void
finishGreet()
{
	int ret;

	if (grtproc.pid > 0) {
		gSet( &grttalk );
		gSendInt( V_OK );
		if ((ret = closeGreeter( FALSE )) != EX_NORMAL) {
			logError( "Abnormal greeter termination, code %d, sig %d\n",
			          wcCode( ret ), wcSig( ret ) );
			sessionExit( EX_RESERVER_DPY );
		}
	}
}

static Jmp_buf idleTOJmp;

/* ARGSUSED */
static void
IdleTimeout( int n ATTR_UNUSED )
{
	Longjmp( idleTOJmp, 1 );
}


static Jmp_buf abortSession;

/* ARGSUSED */
static void
catchTerm( int n ATTR_UNUSED )
{
	Longjmp( abortSession, EX_AL_RESERVER_DPY );
}

/*
 * We need our own error handlers because we can't be sure what exit code Xlib
 * will use, and our Xlib does exit(1) which matches EX_REMANAGE_DPY, which
 * can cause a race condition leaving the display wedged.  We need to use
 * EX_RESERVER_DPY for IO errors, to ensure that the manager waits for the
 * server to terminate.  For other X errors, we should give up.
 */

/*ARGSUSED*/
static int
IOErrorHandler( Display *dspl ATTR_UNUSED )
{
	logError( "Fatal X server IO error: %m\n" );
	/* The only X interaction during the session are pings, and those
	   have an own IOErrorHandler -> not EX_AL_RESERVER_DPY */
	Longjmp( abortSession, EX_RESERVER_DPY );
	/*NOTREACHED*/
	return 0;
}

/*ARGSUSED*/
static int
errorHandler( Display *dspl ATTR_UNUSED, XErrorEvent *event )
{
	logError( "X error\n" );
	if (event->error_code == BadImplementation)
		Longjmp( abortSession, EX_UNMANAGE_DPY );
	return 0;
}

void
manageSession( struct display *d )
{
	int ex, cmd;
	volatile int clientPid = -1;
	volatile time_t tdiff = 0;

	td = d;
	debug( "manageSession %s\n", d->name );
	if ((ex = Setjmp( abortSession ))) {
		closeGreeter( TRUE );
		if (clientPid > 0)
			abortClient( clientPid );
		sessionExit( ex );
		/* NOTREACHED */
	}
	(void)XSetIOErrorHandler( IOErrorHandler );
	(void)XSetErrorHandler( errorHandler );
	(void)Signal( SIGTERM, catchTerm );

	(void)Signal( SIGHUP, SIG_IGN );

	if (Setjmp( grttalk.errjmp ))
		Longjmp( abortSession, EX_RESERVER_DPY ); /* EX_RETRY_ONCE */

#ifdef XDMCP
	if (d->useChooser)
		doChoose();
		/* NOTREACHED */
#endif

	if (d->hstent->sdRec.how) {
		openGreeter();
		gSendInt( G_ConfShutdown );
		gSendInt( d->hstent->sdRec.how );
		gSendInt( d->hstent->sdRec.uid );
		gSendStr( d->hstent->sdRec.osname );
		if ((cmd = ctrlGreeterWait( TRUE )) != G_Ready) {
			logError( "Received unknown command %d from greeter\n", cmd );
			closeGreeter( TRUE );
		}
		goto regreet;
	}

	tdiff = time( 0 ) - td->hstent->lastExit - td->openDelay;
	if (autoLogon( tdiff )) {
		if (!verify( conv_auto, FALSE ))
			goto gcont;
	} else {
	  regreet:
		openGreeter();
		if (Setjmp( idleTOJmp )) {
			closeGreeter( TRUE );
			sessionExit( EX_NORMAL );
		}
		Signal( SIGALRM, IdleTimeout );
		alarm( td->idleTimeout );
#ifdef XDMCP
		if (((d->displayType & d_location) == dLocal) &&
		    d->loginMode >= LOGIN_DEFAULT_REMOTE)
			goto choose;
#endif
		for (;;) {
			debug( "manageSession, greeting, tdiff = %d\n", tdiff );
			gSendInt( (*td->autoUser && td->autoDelay &&
			           (tdiff > 0 || td->autoAgain)) ?
			              G_GreetTimed : G_Greet );
		  gcont:
			cmd = ctrlGreeterWait( TRUE );
#ifdef XDMCP
		  recmd:
			if (cmd == G_DChoose) {
			  choose:
				cmd = doChoose();
				goto recmd;
			}
			if (cmd == G_DGreet)
				continue;
#endif
			alarm( 0 );
			if (cmd == G_Ready)
				break;
			if (cmd == -2)
				closeGreeter( FALSE );
			else {
				logError( "Received unknown command %d from greeter\n", cmd );
				closeGreeter( TRUE );
			}
			goto regreet;
		}
	}

	if (td_setup)
		setupDisplay( td_setup );

	if (!startClient( &clientPid )) {
		logError( "Client start failed\n" );
		sessionExit( EX_NORMAL ); /* XXX maybe EX_REMANAGE_DPY? -- enable in dm.c! */
	}
	debug( "client Started\n" );

	/*
	 * Wait for session to end,
	 */
	for (;;) {
		if (!Setjmp( pingTime )) {
			(void)Signal( SIGALRM, catchAlrm );
			(void)alarm( d->pingInterval * 60 ); /* may be 0 */
			(void)Wait4( &clientPid );
			(void)alarm( 0 );
			break;
		} else {
			(void)alarm( 0 );
			if (!pingServer( d ))
				catchTerm( SIGTERM );
		}
	}
	/*
	 * Sometimes the Xsession somehow manages to exit before
	 * a server crash is noticed - so we sleep a bit and wait
	 * for being killed.
	 */
	if (!pingServer( d )) {
		debug( "X server dead upon session exit.\n" );
		if ((d->displayType & d_location) == dLocal)
			sleep( 10 );
		sessionExit( EX_AL_RESERVER_DPY );
	}
	sessionExit( EX_NORMAL ); /* XXX maybe EX_REMANAGE_DPY? -- enable in dm.c! */
}

static int xResLoaded;

void
loadXloginResources()
{
	char **args;
	char **env;

	if (!xResLoaded && td->resources[0] && access( td->resources, 4 ) == 0) {
		env = systemEnv( (char *)0 );
		if ((args = parseArgs( (char **)0, td->xrdb )) &&
		    (args = addStrArr( args, td->resources, -1 )))
		{
			debug( "loading resource file: %s\n", td->resources );
			(void)runAndWait( args, env );
			freeStrArr( args );
		}
		freeStrArr( env );
		xResLoaded = TRUE;
	}
}

void
setupDisplay( const char *arg )
{
	char **env;

	env = systemEnv( (char *)0 );
	(void)source( env, td->setup, arg );
	freeStrArr( env );
}

void
deleteXloginResources()
{
	int i;
	Atom prop;

	if (!xResLoaded)
		return;
	xResLoaded = FALSE;
	prop = XInternAtom( dpy, "SCREEN_RESOURCES", True );
	XDeleteProperty( dpy, RootWindow( dpy, 0 ), XA_RESOURCE_MANAGER );
	if (prop)
		for (i = ScreenCount(dpy); --i >= 0; )
			XDeleteProperty( dpy, RootWindow( dpy, i ), prop );
	XSync( dpy, 0 );
}


int
source( char **env, const char *file, const char *arg )
{
	char **args;
	int ret;

	if (file && file[0]) {
		debug( "source %s\n", file );
		if (!(args = parseArgs( (char **)0, file )))
			return wcCompose( 0, 0, 127 );
		if (arg && !(args = addStrArr( args, arg, -1 )))
			return wcCompose( 0, 0, 127 );
		ret = runAndWait( args, env );
		freeStrArr( args );
		return ret;
	}
	return 0;
}

char **
inheritEnv( char **env, const char **what )
{
	char *value;

	for (; *what; ++what)
		if ((value = getenv( *what )))
			env = setEnv( env, *what, value );
	return env;
}

char **
baseEnv( const char *user )
{
	char **env;

	env = 0;

#ifdef _AIX
	/* we need the tags SYSENVIRON: and USRENVIRON: in the call to setpenv() */
	env = setEnv( env, "SYSENVIRON:", 0 );
#endif

	if (user) {
		env = setEnv( env, "USER", user );
#ifdef _AIX
		env = setEnv( env, "LOGIN", user );
#endif
		env = setEnv( env, "LOGNAME", user );
	}

#ifdef _AIX
	env = setEnv( env, "USRENVIRON:", 0 );
#endif

	env = inheritEnv( env, (const char **)exportList );

	env = setEnv( env, "DISPLAY",
	              memcmp( td->name, "localhost:", 10 ) ?
	              td->name : td->name + 9 );

#ifdef HAVE_VTS
	/* Support for assistive technologies. */
	if (td->serverVT > 0) {
		char vtstr[4];
		sprintf( vtstr, "%d", td->serverVT );
		env = setEnv( env, "WINDOWPATH", vtstr );
	}
#endif

	if (td->ctrl.path)
		env = setEnv( env, "DM_CONTROL", fifoDir );

	return env;
}

char **
systemEnv( const char *user )
{
	char **env;

	env = baseEnv( user );
	if (td->authFile)
		env = setEnv( env, "XAUTHORITY", td->authFile );
	env = setEnv( env, "PATH", td->systemPath );
	env = setEnv( env, "SHELL", td->systemShell );
	return env;
}
