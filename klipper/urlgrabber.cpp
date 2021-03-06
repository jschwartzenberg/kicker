// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) (C) 2000,2001,2002 by Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <cstdlib>

#include <QCursor>
#include <QHash>
#include <QTimer>

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <ktextedit.h>
#include <klocale.h>
#include <kmenu.h>
#include <kprocess.h>
#include <kservice.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <netwm.h>
#include <kstringhandler.h>
#include <kmacroexpander.h>

#include "urlgrabber.h"
#include <QX11Info>
#include <kglobal.h>

// TODO:
// - script-interface?
// - Bug in KMenu::clear() (insertTitle() doesn't go away sometimes)

#define URL_EDIT_ITEM 10
#define DO_NOTHING_ITEM 11
#define DISABLE_POPUP 12

URLGrabber::URLGrabber(const KSharedConfigPtr &config)
 : m_config( config )
{
    if(!m_config) {
        m_config = KGlobal::config();
    }
    myCurrentAction = 0L;
    myMenu = 0L;
    myPopupKillTimeout = 8;
    m_trimmed = true;

    myActions = new ActionList();
    myActions->setAutoDelete( true );
    myMatches.setAutoDelete( false );

    readConfiguration( m_config.data() );

    myPopupKillTimer = new QTimer( this );
    myPopupKillTimer->setSingleShot( true );
    connect( myPopupKillTimer, SIGNAL( timeout() ),
             SLOT( slotKillPopupMenu() ));

    // testing
    /*
    ClipAction *action;
    action = new ClipAction( "^http:\\/\\/", "Web-URL" );
    action->addCommand("kfmclient exec %s", "Open with Konqi", true);
    action->addCommand("netscape -no-about-splash -remote \"openURL(%s, new-window)\"", "Open with Netscape", true);
    myActions->append( action );

    action = new ClipAction( "^mailto:", "Mail-URL" );
    action->addCommand("kmail --composer %s", "Launch kmail", true);
    myActions->append( action );

    action = new ClipAction( "^\\/.+\\.jpg$", "Jpeg-Image" );
    action->addCommand("kuickshow %s", "Launch KuickShow", true);
    action->addCommand("kview %s", "Launch KView", true);
    myActions->append( action );
    */
}


URLGrabber::~URLGrabber()
{
    delete myActions;
}

//
// Called from Klipper::slotRepeatAction, i.e. by pressing Ctrl-Alt-R
// shortcut. I.e. never from clipboard monitoring
//
void URLGrabber::invokeAction( const QString& clip )
{
    if ( !clip.isEmpty() )
	myClipData = clip;
    if ( m_trimmed )
        myClipData = myClipData.trimmed();

    actionMenu( false );
}


void URLGrabber::setActionList( ActionList *list )
{
    delete myActions;
    myActions = list;
}


const ActionList& URLGrabber::matchingActions( const QString& clipData )
{
    myMatches.clear();
    ClipAction *action = 0L;
    ActionListIterator it( *myActions );
    for ( action = it.current(); action; action = ++it ) {
        if ( action->matches( clipData ) )
            myMatches.append( action );
    }

    return myMatches;
}


bool URLGrabber::checkNewData( const QString& clipData )
{
    // kDebug() << "** checking new data: " << clipData;
    myClipData = clipData;
    if ( m_trimmed )
        myClipData = myClipData.trimmed();

    if ( myActions->isEmpty() )
        return false;

    actionMenu( true ); // also creates myMatches

    return ( !myMatches.isEmpty() &&
             (!m_config->group("General").readEntry("Put Matching URLs in history", true))); //XXX i am not sure this entry exists anymore
}


void URLGrabber::actionMenu( bool wm_class_check )
{
    if ( myClipData.isEmpty() )
        return;

    ActionListIterator it( matchingActions( myClipData ) );
    ClipAction *action = 0L;
    ClipCommand *command = 0L;

    if ( it.count() > 0 ) {
	// don't react on konqi's/netscape's urls...
        if ( wm_class_check && isAvoidedWindow() )
            return;

        QString item;
        myCommandMapper.clear();

        myPopupKillTimer->stop();
        delete myMenu;
        myMenu = new KMenu;
        connect( myMenu, SIGNAL( activated( int )),
                 SLOT( slotItemSelected( int )));

        for ( action = it.current(); action; action = ++it ) {
            Q3PtrListIterator<ClipCommand> it2( action->commands() );
            if ( it2.count() > 0 )
                myMenu->addTitle( SmallIcon( "klipper" ), action->description() +
				     i18n(" - Actions For: ") +
				     KStringHandler::csqueeze(myClipData, 45));
            for ( command = it2.current(); command; command = ++it2 ) {
                item = command->description;
                if ( item.isEmpty() )
                    item = command->command;

                int id;
                if ( command->pixmap.isEmpty() )
                    id = myMenu->insertItem( item );
                else
                    id = myMenu->insertItem( SmallIcon(command->pixmap), item);
                myCommandMapper.insert( id, command );
            }
        }

        // only insert this when invoked via clipboard monitoring, not from an
        // explicit Ctrl-Alt-R
        if ( wm_class_check )
        {
            myMenu->addSeparator();
            myMenu->insertItem( i18n( "Disable This Popup" ), DISABLE_POPUP );
        }
        myMenu->addSeparator();
        // add an edit-possibility
        myMenu->insertItem( SmallIcon("edit"), i18n("&Edit Contents..."),
                            URL_EDIT_ITEM );
        myMenu->insertItem( KIcon("cancel"), i18n("&Cancel"), DO_NOTHING_ITEM );

        if ( myPopupKillTimeout > 0 )
            myPopupKillTimer->start( 1000 * myPopupKillTimeout );

        emit sigPopup( myMenu );
    }
}


void URLGrabber::slotItemSelected( int id )
{
    myMenu->hide(); // deleted by the timer or the next action

    switch ( id ) {
    case -1:
    case DO_NOTHING_ITEM:
        break;
    case URL_EDIT_ITEM:
        editData();
        break;
    case DISABLE_POPUP:
	emit sigDisablePopup();
	break;
    default:
        ClipCommand *command = myCommandMapper.find( id );
        if ( !command )
            qWarning("Klipper: cannot find associated action");
        else
            execute( command );
    }
}


void URLGrabber::execute( const struct ClipCommand *command ) const
{
    if ( command->isEnabled ) {
        QHash<QChar,QString> map;
        map.insert( 's', myClipData );
        QString cmdLine = KMacroExpander::expandMacrosShellQuote( command->command, map );

        if ( cmdLine.isEmpty() )
            return;

        KProcess proc;
        proc.setShellCommand(cmdLine.trimmed());
        if (!proc.startDetached())
            qWarning("Klipper: Could not start process!");
    }
}


void URLGrabber::editData()
{
    myPopupKillTimer->stop();
    KDialog *dlg = new KDialog( 0 );
    dlg->setModal( true );
    dlg->setCaption( i18n("Edit Contents") );
    dlg->setButtons( KDialog::Ok | KDialog::Cancel );

    KTextEdit *edit = new KTextEdit( dlg );
    edit->setText( myClipData );
    edit->setFocus();
    edit->setMinimumSize( 300, 40 );
    dlg->setMainWidget( edit );
    dlg->adjustSize();

    if ( dlg->exec() == KDialog::Accepted ) {
        myClipData = edit->text();
        delete dlg;
        QTimer::singleShot( 0, this, SLOT( slotActionMenu() ) );
    }
    else
    {
        delete dlg;
        myMenu->deleteLater();
        myMenu = 0L;
    }
}


void URLGrabber::readConfiguration( KConfig *kc )
{
    myActions->clear();
    KConfigGroup cg(kc, "General");
    int num = cg.readEntry("Number of Actions", 0);
    myAvoidWindows = cg.readEntry("No Actions for WM_CLASS",QStringList());
    myPopupKillTimeout = cg.readEntry( "Timeout for Action popups (seconds)", 8 );
    m_trimmed = cg.readEntry("Strip Whitespace before exec", true);
    QString group;
    for ( int i = 0; i < num; i++ ) {
        group = QString("Action_%1").arg( i );
        myActions->append( new ClipAction( kc, group ) );
    }
}


void URLGrabber::writeConfiguration( KConfig *kc )
{
    KConfigGroup cg(kc, "General");
    cg.writeEntry( "Number of Actions", myActions->count() );
    cg.writeEntry( "Timeout for Action popups (seconds)", myPopupKillTimeout);
    cg.writeEntry( "No Actions for WM_CLASS", myAvoidWindows );
    cg.writeEntry( "Strip Whitespace before exec", m_trimmed );

    ActionListIterator it( *myActions );
    ClipAction *action;

    int i = 0;
    QString group;
    while ( (action = it.current()) ) {
        group = QString("Action_%1").arg( i );
        action->save( kc, group );
        ++i;
        ++it;
    }
}

// find out whether the active window's WM_CLASS is in our avoid-list
// digged a little bit in netwm.cpp
bool URLGrabber::isAvoidedWindow() const
{
    Display *d = QX11Info::display();
    static Atom wm_class = XInternAtom( d, "WM_CLASS", true );
    static Atom active_window = XInternAtom( d, "_NET_ACTIVE_WINDOW", true );
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret, unused;
    unsigned char *data_ret;
    long BUFSIZE = 2048;
    bool ret = false;
    Window active = 0L;
    QString wmClass;

    // get the active window
    if (XGetWindowProperty(d, DefaultRootWindow( d ), active_window, 0l, 1l,
                           False, XA_WINDOW, &type_ret, &format_ret,
                           &nitems_ret, &unused, &data_ret)
        == Success) {
        if (type_ret == XA_WINDOW && format_ret == 32 && nitems_ret == 1) {
            active = *((Window *) data_ret);
        }
        XFree(data_ret);
    }
    if ( !active )
        return false;

    // get the class of the active window
    if ( XGetWindowProperty(d, active, wm_class, 0L, BUFSIZE, False, XA_STRING,
                            &type_ret, &format_ret, &nitems_ret,
                            &unused, &data_ret ) == Success) {
        if ( type_ret == XA_STRING && format_ret == 8 && nitems_ret > 0 ) {
            wmClass = QString::fromUtf8( (const char *) data_ret );
            ret = (myAvoidWindows.find( wmClass ) != myAvoidWindows.end());
        }

        XFree( data_ret );
    }

    return ret;
}


void URLGrabber::slotKillPopupMenu()
{
    if ( myMenu && myMenu->isVisible() )
    {
        if ( myMenu->geometry().contains( QCursor::pos() ) &&
             myPopupKillTimeout > 0 )
        {
            myPopupKillTimer->start( 1000 * myPopupKillTimeout );
            return;
        }
    }

    delete myMenu;
    myMenu = 0L;
}

///////////////////////////////////////////////////////////////////////////
////////

ClipCommand::ClipCommand(const QString &_command, const QString &_description,
                         bool _isEnabled, const QString &_icon)
    : command(_command),
      description(_description),
      isEnabled(_isEnabled)
{
    int len = command.indexOf(" ");
    if (len == -1)
        len = command.length();

    if (!_icon.isEmpty())
        pixmap = _icon;
    else
    {
    KService::Ptr service= KService::serviceByDesktopName(command.left(len));
    if (service)
        pixmap = service->icon();
    else
        pixmap.clear();
    }
}


ClipAction::ClipAction( const QString& regExp, const QString& description )
    : myRegExp( regExp ), myDescription( description )
{
    myCommands.setAutoDelete( true );
}


ClipAction::ClipAction( const ClipAction& action )
{
    myCommands.setAutoDelete( true );
    myRegExp      = action.myRegExp;
    myDescription = action.myDescription;

    ClipCommand *command = 0L;
    Q3PtrListIterator<ClipCommand> it( myCommands );
    for ( ; it.current(); ++it ) {
        command = it.current();
        addCommand(command->command, command->description, command->isEnabled);
    }
}


ClipAction::ClipAction( KConfig *kc, const QString& group )
    : myRegExp( kc->group(group).readEntry("Regexp") ),
      myDescription (kc->group(group).readEntry("Description") )
{
    KConfigGroup cg(kc, group);

    myCommands.setAutoDelete( true );
    int num = cg.readEntry( "Number of commands", 0 );

    // read the commands
    for ( int i = 0; i < num; i++ ) {
        QString _group = group + "/Command_%1";
        KConfigGroup _cg(kc, _group.arg(i));

        addCommand( _cg.readPathEntry( "Commandline" ),
                    _cg.readEntry( "Description" ), // i18n'ed
                    _cg.readEntry( "Enabled" , false),
                    _cg.readEntry( "Icon") );
    }
}


ClipAction::~ClipAction()
{
}


void ClipAction::addCommand( const QString& command,
                             const QString& description, bool enabled, const QString& icon )
{
    if ( command.isEmpty() )
        return;

    struct ClipCommand *cmd = new ClipCommand( command, description, enabled, icon );
    //    cmd->id = myCommands.count(); // superfluous, I think...
    myCommands.append( cmd );
}


// precondition: we're in the correct action's group of the KConfig object
void ClipAction::save( KConfig *kc, const QString& group ) const
{
    KConfigGroup cg(kc, group);
    cg.writeEntry( "Description", description() );
    cg.writeEntry( "Regexp", regExp() );
    cg.writeEntry( "Number of commands", myCommands.count() );

    struct ClipCommand *cmd;
    Q3PtrListIterator<struct ClipCommand> it( myCommands );

    // now iterate over all commands of this action
    int i = 0;
    while ( (cmd = it.current()) ) {
        QString _group = group + "/Command_%1";
        KConfigGroup cg(kc, _group.arg(i));

        cg.writePathEntry( "Commandline", cmd->command );
        cg.writeEntry( "Description", cmd->description );
        cg.writeEntry( "Enabled", cmd->isEnabled );

        ++i;
        ++it;
    }
}

#include "urlgrabber.moc"
