// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project

   Copyright (C) by Andrew Stanley-Jones
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

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

#include <QClipboard>
#include <QCursor>
#include <QDateTime>
#include <QFile>
#include <QPainter>

#include <QtGui/QMacMime>
#include <Qt3Support/Q3ColorDrag>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QX11Info>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksavefile.h>
#include <ksessionmanager.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <ksystemtrayicon.h>
#include <k3urldrag.h>
#include <kwindowsystem.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <QtDBus/QDBusConnection>
#include <kiconloader.h>
#include <khelpmenu.h>
#include <kstandardguiitem.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kconfiggroup.h>

#include "configdialog.h"
#include "klipper.h"
#include "urlgrabber.h"
#include "version.h"
#include "clipboardpoll.h"
#include "history.h"
#include "historyitem.h"
#include "historystringitem.h"
#include "klipperpopup.h"

#include <zlib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

//#define NOISY_KLIPPER

namespace {
    /**
     * Use this when manipulating the clipboard
     * from within clipboard-related signals.
     *
     * This avoids issues such as mouse-selections that immediately
     * disappear.
     * pattern: Resource Acqusition is Initialisation (RAII)
     *
     * (This is not threadsafe, so don't try to use such in threaded
     * applications).
     */
    struct Ignore {
        Ignore(int& locklevel) : locklevelref(locklevel)  {
            locklevelref++;
        }
        ~Ignore() {
            locklevelref--;
        }
    private:
        int& locklevelref;
    };
}

/**
 * Helper class to save history upon session exit.
 */
class KlipperSessionManager : public KSessionManager
{
public:
    KlipperSessionManager( Klipper* k )
        : klipper( k )
        {}

    virtual ~KlipperSessionManager() {}

    /**
     * Save state upon session exit.
     *
     * Saving history on session save
     */
    virtual bool commitData( QSessionManager& ) {
        klipper->saveSession();
        return true;
    }
private:
    Klipper* klipper;
};

static void ensureGlobalSyncOff(KSharedConfigPtr config);

// config == KGlobal::config for process, otherwise applet
Klipper::Klipper(QObject *parent, const KSharedConfigPtr &config)
    : QObject( parent )
    , m_overflowCounter( 0 )
    , locklevel( 0 )
    , m_config( config )
    , m_pendingContentsCheck( false )
    , session_managed( new KlipperSessionManager( this ))
{
    QDBusConnection::sessionBus().registerObject("/klipper", this, QDBusConnection::ExportScriptableSlots);

    // We don't use the clipboardsynchronizer anymore, and it confuses Klipper
    ensureGlobalSyncOff(m_config);

    updateTimestamp(); // read initial X user time
    clip = kapp->clipboard();

    connect( &m_overflowClearTimer, SIGNAL( timeout()), SLOT( slotClearOverflow()));
    m_overflowClearTimer.start( 1000 );

    m_pendingCheckTimer.setSingleShot( true );
    connect( &m_pendingCheckTimer, SIGNAL( timeout()), SLOT( slotCheckPending()));

    m_history = new History( this );

    // we need that collection, otherwise KToggleAction is not happy :}
    //QString defaultGroup( "default" );
    KActionCollection *collection = new KActionCollection( this );
    toggleURLGrabAction = new KToggleAction( this );
    collection->addAction( "toggleUrlGrabAction", toggleURLGrabAction );
    toggleURLGrabAction->setEnabled( true );
    //toggleURLGrabAction->setGroup( defaultGroup );
    clearHistoryAction = collection->addAction( "clearHistoryAction" );
    clearHistoryAction->setIcon( KIcon("history-clear") );
    clearHistoryAction->setText( i18n("C&lear Clipboard History") );
    connect(clearHistoryAction, SIGNAL(triggered(bool) ), history(), SLOT( slotClear() ));
    connect( clearHistoryAction, SIGNAL( activated() ), SLOT( slotClearClipboard() ) );
    //clearHistoryAction->setGroup( defaultGroup );
    configureAction = collection->addAction( "configureAction" );
    configureAction->setIcon( KIcon("configure") );
    configureAction->setText( i18n("&Configure Klipper...") );
    connect(configureAction, SIGNAL(triggered(bool) ), SLOT( slotConfigure() ));
    //configureAction->setGroup( defaultGroup );
    quitAction = collection->addAction( "quitAction" );
    quitAction->setIcon( KIcon("application-exit") );
    quitAction->setText( i18n("&Quit") );
    connect(quitAction, SIGNAL(triggered(bool) ), SLOT( slotQuit() ));
    //quitAction->setGroup( "exit" );
    myURLGrabber = 0L;
    KConfig *kc = m_config.data();
    readConfiguration( kc );
    setURLGrabberEnabled( bURLGrabber );

    hideTimer = new QTime();
    showTimer = new QTime();

    readProperties(m_config.data());
    connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)));

    poll = new ClipboardPoll;
    connect( poll, SIGNAL( clipboardChanged( bool ) ),
             this, SLOT( newClipData( bool ) ) );

    m_pixmap = KSystemTrayIcon::loadIcon( "klipper" ).pixmap();

    globalKeys = KGlobalAccel::self();
    KActionCollection* actionCollection = collection;
    QAction* a = 0L;
#ifdef __GNUC__
#warning TODO PORT ME (KGlobalAccel related)
#endif
#include "klipperbindings.cpp"
    // the keys need to be read from kdeglobals, not kickerrc --ellis, 22/9/02
    //globalKeys->readSettings();
    //globalKeys->updateConnections();
    //toggleURLGrabAction->setShortcut(globalKeys->shortcut("Enable/Disable Clipboard Actions"));

    connect( toggleURLGrabAction, SIGNAL( toggled( bool )),
             this, SLOT( setURLGrabberEnabled( bool )));

    KlipperPopup* popup = history()->popup();
    connect ( history(), SIGNAL( topChanged() ), SLOT( slotHistoryTopChanged() ) );
    connect( popup, SIGNAL( aboutToHide() ), SLOT( slotStartHideTimer() ) );
    connect( popup, SIGNAL( aboutToShow() ), SLOT( slotStartShowTimer() ) );

    popup->plugAction( toggleURLGrabAction );
    popup->plugAction( clearHistoryAction );
    popup->plugAction( configureAction );
    if ( !isApplet() ) {
        popup->plugAction( quitAction );
    }
}

Klipper::~Klipper()
{
    delete poll;
    delete session_managed;
    delete showTimer;
    delete hideTimer;
    delete myURLGrabber;
}

// DCOP
QString Klipper::getClipboardContents()
{
    return getClipboardHistoryItem(0);
}

// DCOP - don't call from Klipper itself
void Klipper::setClipboardContents(QString s)
{
    Ignore lock( locklevel );
    updateTimestamp();
    HistoryStringItem* item = new HistoryStringItem( s );
    setClipboard( *item, Clipboard | Selection);
    history()->insert( item );
}

// DCOP - don't call from Klipper itself
void Klipper::clearClipboardContents()
{
    updateTimestamp();
    slotClearClipboard();
}

// DCOP - don't call from Klipper itself
void Klipper::clearClipboardHistory()
{
    updateTimestamp();
    slotClearClipboard();
    history()->slotClear();
    saveSession();
}


void Klipper::slotStartHideTimer()
{
    hideTimer->start();
}

void Klipper::slotStartShowTimer()
{
    showTimer->start();
}

void Klipper::showPopupMenu( QMenu *menu )
{
    Q_ASSERT( menu != 0L );

    QSize size = menu->sizeHint(); // geometry is not valid until it's shown
    if (bPopupAtMouse) {
        QPoint g = QCursor::pos();
        if ( size.height() < g.y() )
            menu->popup(QPoint( g.x(), g.y() - size.height()));
        else
            menu->popup(QPoint(g.x(), g.y()));
    } else {
        if( KSystemTrayIcon* tray = dynamic_cast< KSystemTrayIcon* >( parent())) {
            QRect g = tray->geometry();
            QRect screen = KGlobalSettings::desktopGeometry(g.center());

            if ( g.x()-screen.x() > screen.width()/2 &&
                 g.y()-screen.y() + size.height() > screen.height() )
                menu->popup(QPoint( g.x(), g.y() - size.height()));
            else
                menu->popup(QPoint( g.x() + g.width(), g.y() + g.height()));
        } else
            abort();

        //      menu->exec(mapToGlobal(QPoint( width()/2, height()/2 )));
    }
}

bool Klipper::loadHistory() {
    static const char* const failed_load_warning =
        "Failed to load history resource. Clipboard history cannot be read.";
    // don't use "appdata", klipper is also a kicker applet
    QString history_file_name = KStandardDirs::locateLocal( "data", "klipper/history2.lst" );
    QFile history_file( history_file_name );
    bool oldfile = false;
    if ( !history_file.exists() ) { // backwards compatibility
        oldfile = true;
        history_file_name = KStandardDirs::locateLocal( "data", "klipper/history.lst" );
        history_file.setFileName( history_file_name );
        if ( !history_file.exists() ) {
            history_file_name = KStandardDirs::locateLocal( "data", "kicker/history.lst" );
            history_file.setFileName( history_file_name );
            if ( !history_file.exists() ) {
                return false;
            }
        }
    }
    if ( !history_file.open( QIODevice::ReadOnly ) ) {
        kWarning() << failed_load_warning << ": " << history_file.errorString() ;
        return false;
    }
    QDataStream file_stream( &history_file );
    if( file_stream.atEnd()) {
        kWarning() << failed_load_warning ;
        return false;
    }
    QDataStream* history_stream = &file_stream;
    QByteArray data;
    if( !oldfile ) {
        quint32 crc;
        file_stream >> crc >> data;
        if( crc32( 0, reinterpret_cast<unsigned char *>( data.data() ), data.size() ) != crc ) {
            kWarning() << failed_load_warning << ": " << history_file.errorString() ;
            return false;
        }
        history_stream = new QDataStream( &data, QIODevice::ReadOnly );
    }
    char* version;
    *history_stream >> version;
    delete[] version;

    // The list needs to be reversed, as it is saved
    // youngest-first to keep the most important clipboard
    // items at the top, but the history is created oldest
    // first.
    QList<HistoryItem*> reverseList;
    for ( HistoryItem* item = HistoryItem::create( *history_stream );
          item;
          item = HistoryItem::create( *history_stream ) )
    {
        reverseList.prepend( item );
    }

    for ( QList<HistoryItem*>::const_iterator it = reverseList.begin();
          it != reverseList.end();
          ++it )
    {
        history()->forceInsert( *it );
    }

    if ( !history()->empty() ) {
        m_lastSelection = -1;
        m_lastClipboard = -1;
        setClipboard( *history()->first(), Clipboard | Selection );
    }

    if( history_stream != &file_stream )
        delete history_stream;

    return true;
}

void Klipper::saveHistory() {
    static const char* const failed_save_warning =
        "Failed to save history. Clipboard history cannot be saved.";
    // don't use "appdata", klipper is also a kicker applet
    QString history_file_name( KStandardDirs::locateLocal( "data", "klipper/history2.lst" ) );
    if ( history_file_name.isNull() || history_file_name.isEmpty() ) {
        kWarning() << failed_save_warning ;
        return;
    }
    KSaveFile history_file( history_file_name );
    if ( !history_file.open() ) {
        kWarning() << failed_save_warning ;
        return;
    }
    QByteArray data;
    QDataStream history_stream( &data, QIODevice::WriteOnly );
    history_stream << klipper_version; // const char*
    for (  const HistoryItem* item = history()->first(); item; item = history()->next() ) {
        history_stream << item;
    }
    quint32 crc = crc32( 0, reinterpret_cast<unsigned char *>( data.data() ), data.size() );
    QDataStream ds ( &history_file );
    ds << crc << data;
}

void Klipper::readProperties(KConfig *kc)
{
    QStringList dataList;

    history()->slotClear();

    if (bKeepContents) { // load old clipboard if configured
        if ( !loadHistory() ) {
            // Try to load from the old config file.
            // Remove this at some point.
            KConfigGroup configGroup(kc, "General");
            dataList = configGroup.readEntry("ClipboardData",QStringList());

            for (QStringList::ConstIterator it = dataList.end();
                 it != dataList.begin();
             )
            {
                history()->forceInsert( new HistoryStringItem( *( --it ) ) );
            }

            if ( !dataList.isEmpty() )
            {
                m_lastSelection = -1;
                m_lastClipboard = -1;
                setClipboard( *history()->first(), Clipboard | Selection );
            }
        }
    }

}


void Klipper::readConfiguration( KConfig *_kc )
{
    KConfigGroup kc( _kc, "General");
    bPopupAtMouse = kc.readEntry("PopupAtMousePosition", false);
    bKeepContents = kc.readEntry("KeepClipboardContents", true);
    bURLGrabber = kc.readEntry("URLGrabberEnabled", false);
    bReplayActionInHistory = kc.readEntry("ReplayActionInHistory", false);
    bNoNullClipboard = kc.readEntry("NoEmptyClipboard", true);
    bUseGUIRegExpEditor = kc.readEntry("UseGUIRegExpEditor", true);
    history()->max_size( kc.readEntry("MaxClipItems", 7) );
    bIgnoreSelection = kc.readEntry("IgnoreSelection", false);
    bSynchronize = kc.readEntry("Synchronize", false);
    bSelectionTextOnly = kc.readEntry("SelectionTextOnly",true);
    bIgnoreImages = kc.readEntry("IgnoreImages",true);
}

void Klipper::writeConfiguration( KConfig *_kc )
{
    KConfigGroup kc( _kc, "General");
    kc.writeEntry("PopupAtMousePosition", bPopupAtMouse);
    kc.writeEntry("KeepClipboardContents", bKeepContents);
    kc.writeEntry("ReplayActionInHistory", bReplayActionInHistory);
    kc.writeEntry("NoEmptyClipboard", bNoNullClipboard);
    kc.writeEntry("UseGUIRegExpEditor", bUseGUIRegExpEditor);
    kc.writeEntry("MaxClipItems", history()->max_size() );
    kc.writeEntry("IgnoreSelection", bIgnoreSelection);
    kc.writeEntry("Synchronize", bSynchronize );
    kc.writeEntry("SelectionTextOnly", bSelectionTextOnly);
    kc.writeEntry("TrackImages", bIgnoreImages);
    kc.writeEntry("Version", klipper_version );

    if ( myURLGrabber )
        myURLGrabber->writeConfiguration( _kc );

    kc.sync();
}

// save session on shutdown. Don't simply use the c'tor, as that may not be called.
void Klipper::saveSession()
{
    if ( bKeepContents ) { // save the clipboard eventually
        saveHistory();
    }
}

void Klipper::slotSettingsChanged( int category )
{
    if ( category == (int) KGlobalSettings::SETTINGS_SHORTCUTS ) {
#ifdef __GNUC__
#warning TODO PORT ME (KGlobalAccel related)
#endif
        //globalKeys->readSettings();
        //globalKeys->updateConnections();
        //toggleURLGrabAction->setShortcut(globalKeys->shortcut("Enable/Disable Clipboard Actions"));
    }
}

void Klipper::disableURLGrabber()
{
    KMessageBox::information( 0L,
                              i18n( "You can enable URL actions later by right-clicking on the "
                                    "Klipper icon and selecting 'Enable Actions'" ) );

    setURLGrabberEnabled( false );
}

void Klipper::slotConfigure()
{
    bool haveURLGrabber = bURLGrabber;
    if ( !myURLGrabber ) { // temporary, for the config-dialog
        setURLGrabberEnabled( true );
        readConfiguration( m_config.data() );
    }

    ConfigDialog *dlg = new ConfigDialog( myURLGrabber->actionList(),
                                          globalKeys, isApplet() );
    dlg->setKeepContents( bKeepContents );
    dlg->setPopupAtMousePos( bPopupAtMouse );
    dlg->setStripWhiteSpace( myURLGrabber->trimmed() );
    dlg->setReplayActionInHistory( bReplayActionInHistory );
    dlg->setNoNullClipboard( bNoNullClipboard );
    dlg->setUseGUIRegExpEditor( bUseGUIRegExpEditor );
    dlg->setPopupTimeout( myURLGrabber->popupTimeout() );
    dlg->setMaxItems( history()->max_size() );
    dlg->setIgnoreSelection( bIgnoreSelection );
    dlg->setSynchronize( bSynchronize );
    dlg->setNoActionsFor( myURLGrabber->avoidWindows() );

    if ( dlg->exec() == QDialog::Accepted ) {
        bKeepContents = dlg->keepContents();
        bPopupAtMouse = dlg->popupAtMousePos();
        bReplayActionInHistory = dlg->replayActionInHistory();
        bNoNullClipboard = dlg->noNullClipboard();
        bIgnoreSelection = dlg->ignoreSelection();
        bSynchronize = dlg->synchronize();
        bUseGUIRegExpEditor = dlg->useGUIRegExpEditor();
        dlg->commitShortcuts();
#ifdef __GNUC__
#warning TODO PORT ME (KGlobalAccel related)
#endif
        //globalKeys->writeSettings();
        //globalKeys->updateConnections();
        //toggleURLGrabAction->setShortcut(globalKeys->shortcut("Enable/Disable Clipboard Actions"));

        myURLGrabber->setActionList( dlg->actionList() );
        myURLGrabber->setPopupTimeout( dlg->popupTimeout() );
        myURLGrabber->setStripWhiteSpace( dlg->trimmed() );
        myURLGrabber->setAvoidWindows( dlg->noActionsFor() );

        history()->max_size( dlg->maxItems() );

        writeConfiguration( m_config.data() );

    }
    setURLGrabberEnabled( haveURLGrabber );

    delete dlg;
}

void Klipper::slotQuit()
{
    // If the menu was just opened, likely the user
    // selected quit by accident while attempting to
    // click the Klipper icon.
    if ( showTimer->elapsed() < 300 ) {
        return;
    }

    saveSession();
    int autoStart = KMessageBox::questionYesNoCancel( 0L, i18n("Should Klipper start automatically\nwhen you login?"), i18n("Automatically Start Klipper?"),KGuiItem(i18n("Start")), KGuiItem(i18n("Do Not Start")) );

    KConfigGroup config( KGlobal::config(), "General");
    if ( autoStart == KMessageBox::Yes ) {
        config.writeEntry("AutoStart", true);
    } else if ( autoStart == KMessageBox::No) {
        config.writeEntry("AutoStart", false);
    } else  // cancel chosen don't quit
        return;
    config.sync();

    kapp->quit();

}

void Klipper::slotPopupMenu() {
    KlipperPopup* popup = history()->popup();
    popup->ensureClean();
    showPopupMenu( popup );
}


void Klipper::slotRepeatAction()
{
    if ( !myURLGrabber ) {
        myURLGrabber = new URLGrabber( m_config );
        connect( myURLGrabber, SIGNAL( sigPopup( QMenu * )),
                 SLOT( showPopupMenu( QMenu * )) );
        connect( myURLGrabber, SIGNAL( sigDisablePopup() ),
                 this, SLOT( disableURLGrabber() ) );
    }

    const HistoryStringItem* top = dynamic_cast<const HistoryStringItem*>( history()->first() );
    if ( top ) {
        myURLGrabber->invokeAction( top->text() );
    }
}

void Klipper::setURLGrabberEnabled( bool enable )
{
    if (enable != bURLGrabber) {
      bURLGrabber = enable;
      KConfigGroup kc(m_config.data(), "General");
      kc.writeEntry("URLGrabberEnabled", bURLGrabber);
      m_lastURLGrabberTextSelection = QString();
      m_lastURLGrabberTextClipboard = QString();
    }

    toggleURLGrabAction->setChecked( enable );

    if ( !bURLGrabber ) {
        delete myURLGrabber;
        myURLGrabber = 0L;
        toggleURLGrabAction->setText(i18n("Enable &Actions"));
    }

    else {
        toggleURLGrabAction->setText(i18n("&Actions Enabled"));
        if ( !myURLGrabber ) {
            myURLGrabber = new URLGrabber( m_config );
            connect( myURLGrabber, SIGNAL( sigPopup( QMenu * )),
                     SLOT( showPopupMenu( QMenu * )) );
            connect( myURLGrabber, SIGNAL( sigDisablePopup() ),
                     this, SLOT( disableURLGrabber() ) );
        }
    }
}

void Klipper::toggleURLGrabber()
{
    setURLGrabberEnabled( !bURLGrabber );
}

void Klipper::slotHistoryTopChanged() {
    if ( locklevel ) {
        return;
    }

    const HistoryItem* topitem = history()->first();
    if ( topitem ) {
        setClipboard( *topitem, Clipboard | Selection );
    }
    if ( bReplayActionInHistory && bURLGrabber ) {
        slotRepeatAction();
    }

}

void Klipper::slotClearClipboard()
{
    Ignore lock( locklevel );

    clip->clear(QClipboard::Selection);
    clip->clear(QClipboard::Clipboard);


}


//XXX: Should die, and the DCOP signal handled sensible.
QString Klipper::clipboardContents( bool * /*isSelection*/ )
{
    kWarning() << "Obsolete function called. Please fix" ;

#if 0
    bool selection = true;
    QMimeSource* data = clip->data(QClipboard::Selection);

    if ( data->serialNumber() == m_lastSelection )
    {
        QString clipContents = clip->text(QClipboard::Clipboard);
        if ( clipContents != m_lastClipboard )
        {
            contents = clipContents;
            selection = false;
        }
        else
            selection = true;
    }

    if ( isSelection )
        *isSelection = selection;

#endif

    return 0;
}

void Klipper::applyClipChanges( const QMimeData* clipData )
{
    if ( locklevel )
        return;
    Ignore lock( locklevel );
    history()->insert( HistoryItem::create( clipData ) );

}

void Klipper::newClipData( bool selectionMode )
{
    if ( locklevel ) {
        return;
    }

    if( blockFetchingNewData())
        return;

    checkClipData( selectionMode );

}

void Klipper::clipboardSignalArrived( bool selectionMode )
{
    if ( locklevel ) {
        return;
    }
    if( blockFetchingNewData())
        return;

    updateTimestamp();
    checkClipData( selectionMode );
}

// Protection against too many clipboard data changes. Lyx responds to clipboard data
// requests with setting new clipboard data, so if Lyx takes over clipboard,
// Klipper notices, requests this data, this triggers "new" clipboard contents
// from Lyx, so Klipper notices again, requests this data, ... you get the idea.
const int MAX_CLIPBOARD_CHANGES = 10; // max changes per second

bool Klipper::blockFetchingNewData()
{
// Hacks for #85198 and #80302.
// #85198 - block fetching new clipboard contents if Shift is pressed and mouse is not,
//   this may mean the user is doing selection using the keyboard, in which case
//   it's possible the app sets new clipboard contents after every change - Klipper's
//   history would list them all.
// #80302 - OOo (v1.1.3 at least) has a bug that if Klipper requests its clipboard contents
//   while the user is doing a selection using the mouse, OOo stops updating the clipboard
//   contents, so in practice it's like the user has selected only the part which was
//   selected when Klipper asked first.
// Use XQueryPointer rather than QApplication::mouseButtons()/keyboardModifiers(), because
//   Klipper needs the very current state.
    Window root, child;
    int root_x, root_y, win_x, win_y;
    uint state;
    XQueryPointer( QX11Info::display(), QX11Info::appRootWindow(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &state );
    if( ( state & ( ShiftMask | Button1Mask )) == ShiftMask // #85198
        || ( state & Button1Mask ) == Button1Mask ) { // #80302
        m_pendingContentsCheck = true;
        m_pendingCheckTimer.start( 100 );
        return true;
    }
    m_pendingContentsCheck = false;
    if( ++m_overflowCounter > MAX_CLIPBOARD_CHANGES )
        return true;
    return false;
}

void Klipper::slotCheckPending()
{
    if( !m_pendingContentsCheck )
        return;
    m_pendingContentsCheck = false; // blockFetchingNewData() will be called again
    updateTimestamp();
    newClipData( true ); // always selection
}

void Klipper::checkClipData( bool selectionMode )
{
    if ( ignoreClipboardChanges() ) // internal to klipper, ignoring QSpinBox selections
    {
        // keep our old clipboard, thanks
        // This won't quite work, but it's close enough for now.
        // The trouble is that the top selection =! top clipboard
        // but we don't track that yet. We will....
        const HistoryItem* top = history()->first();
        if ( top ) {
            setClipboard( *top, selectionMode ? Selection : Clipboard);
        }
        return;
    }

// debug code
#ifdef NOISY_KLIPPER
    kDebug() << "Checking clip data";
#endif
#if 0
    kDebug() << "====== c h e c k C l i p D a t a ============================"
              << kBacktrace()
              << "====== c h e c k C l i p D a t a ============================"
              << endl;;
#endif
#if 0
    if ( sender() ) {
        kDebug() << "sender=" << sender()->name();
    } else {
        kDebug() << "no sender";
    }
#endif
#if 0
    kDebug() << "\nselectionMode=" << selectionMode
              << "\nserialNo=" << clip->data()->serialNumber() << " (sel,cli)=(" << m_lastSelection << "," << m_lastClipboard << ")"
              << "\nowning (sel,cli)=(" << clip->ownsSelection() << "," << clip->ownsClipboard() << ")"
              << "\ntext=" << clip->text( selectionMode ? QClipboard::Selection : QClipboard::Clipboard) << endl;

#endif
#if 0
    const char *format;
    int i = 0;
    while ( (format = clip->data()->format( i++ )) )
    {
        qDebug( "    format: %s", format);
    }
#endif
    const QMimeData* data = clip->mimeData( selectionMode ? QClipboard::Selection : QClipboard::Clipboard );
    if ( !data ) {
        kWarning("No data in clipboard. This not not supposed to happen." );
        return;
    }
    // TODO: Rewrite to Qt4 !!!
    //int lastSerialNo = selectionMode ? m_lastSelection : m_lastClipboard;
    //bool changed = data->serialNumber() != lastSerialNo;
    bool changed = true; // ### FIXME
    bool clipEmpty = data->formats().isEmpty();

    if ( changed && clipEmpty && bNoNullClipboard ) {
        const HistoryItem* top = history()->first();
        if ( top ) {
            // keep old clipboard after someone set it to null
#ifdef NOISY_KLIPPER
            kDebug() << "Resetting clipboard (Prevent empty clipboard)";
#endif
            setClipboard( *top, selectionMode ? Selection : Clipboard );
        }
        return;
    }
 
    // this must be below the "bNoNullClipboard" handling code!
    // XXX: I want a better handling of selection/clipboard in general.
    // XXX: Order sensitive code. Must die.
    if ( selectionMode && bIgnoreSelection )
        return;

    if( selectionMode && bSelectionTextOnly && !data->hasText())
        return;

#ifdef __GNUC__
#warning This should be maybe extended for KDE4 or at least get a checkbox somewhere in UI
#endif
    if( KUrl::List::canDecode( data ) )
        ; // ok
    else if( data->hasText() )
        ; // ok
    else if( data->hasImage() )
    {
// Limit mimetypes that are tracked by Klipper (this is basically a workaround
// for #109032). Can't add UI in 3.5 because of string freeze, and I'm not sure
// if this actually needs to be more configurable than only text vs all klipper knows.
        if( bIgnoreImages )
            return;
    }
    else // unknown, ignore
        return;

    // store old contents:
#if 0
    if ( selectionMode )
        m_lastSelection = data->serialNumber();
    else
        m_lastClipboard = data->serialNumber();
#endif

    QString& lastURLGrabberText = selectionMode
        ? m_lastURLGrabberTextSelection : m_lastURLGrabberTextClipboard;
    if( data->hasText() )
    {
        if ( bURLGrabber && myURLGrabber )
        {
            QString text = data->text();

            // Make sure URLGrabber doesn't repeat all the time if klipper reads the same
            // text all the time (e.g. because XFixes is not available and the application
            // has broken TIMESTAMP target). Using most recent history item may not always
            // work.
            if ( text != lastURLGrabberText )
            {
                lastURLGrabberText = text;
                if ( myURLGrabber->checkNewData( text ) )
                {
                    return; // don't add into the history
                }
            }
        }
        else
            lastURLGrabberText = QString();
    }
    else
        lastURLGrabberText = QString();

    if (changed) {
        applyClipChanges( data );
#ifdef NOISY_KLIPPER
        kDebug() << "Synchronize?" << ( bSynchronize ? "yes" : "no" );
#endif
        if ( bSynchronize ) {
            const HistoryItem* topItem = history()->first();
            if ( topItem ) {
                setClipboard( *topItem, selectionMode ? Clipboard : Selection );
            }
        }
    }
}

void Klipper::setClipboard( const HistoryItem& item, int mode )
{
    Ignore lock( locklevel );

    Q_ASSERT( ( mode & 1 ) == 0 ); // Warn if trying to pass a boolean as a mode.

    if ( mode & Selection ) {
#ifdef NOSIY_KLIPPER
        kDebug() << "Setting selection to <" << item.text() << ">";
#endif
        clip->setMimeData( item.mimeData(), QClipboard::Selection );
#if 0
        m_lastSelection = clip->data()->serialNumber();<
#endif
    }
    if ( mode & Clipboard ) {
#ifdef NOSIY_KLIPPER
        kDebug() << "Setting clipboard to <" << item.text() << ">";
#endif
        clip->setMimeData( item.mimeData(), QClipboard::Clipboard );
#if 0
        m_lastClipboard = clip->data()->serialNumber();
#endif
    }

}

void Klipper::slotClearOverflow()
{
    if( m_overflowCounter > MAX_CLIPBOARD_CHANGES ) {
        kDebug() << "App owning the clipboard/selection is lame";
        // update to the latest data - this unfortunately may trigger the problem again
        newClipData( true ); // Always the selection.
    }
    m_overflowCounter = 0;
}

QStringList Klipper::getClipboardHistoryMenu()
{
    QStringList menu;
    for ( const HistoryItem* item = history()->first(); item; item = history()->next() ) {
        menu << item->text();
    }
    return menu;
}

QString Klipper::getClipboardHistoryItem(int i)
{
    for ( const HistoryItem* item = history()->first(); item; item = history()->next() , i-- ) {
        if ( i == 0 ) {
            return item->text();
        }
    }
    return QString();

}

//
// changing a spinbox in klipper's config-dialog causes the lineedit-contents
// of the spinbox to be selected and hence the clipboard changes. But we don't
// want all those items in klipper's history. See #41917
//
bool Klipper::ignoreClipboardChanges() const
{
    QWidget *focusWidget = qApp->focusWidget();
    if ( focusWidget )
    {
        if ( focusWidget->inherits( "QSpinBox" ) ||
             (focusWidget->parentWidget() &&
              focusWidget->inherits("QLineEdit") &&
              focusWidget->parentWidget()->inherits("QSpinWidget")) )
        {
            return true;
        }
    }

    return false;
}

// QClipboard uses qt_x_time as the timestamp for selection operations.
// It is updated mainly from user actions, but Klipper polls the clipboard
// without any user action triggering it, so qt_x_time may be old,
// which could possibly lead to QClipboard reporting empty clipboard.
// Therefore, qt_x_time needs to be updated to current X server timestamp.

// Call KApplication::updateUserTime() only from functions that are
// called from outside (DCOP), or from QTimer timeout !

static Time next_x_time;
static Bool update_x_time_predicate( Display*, XEvent* event, XPointer )
{
    if( next_x_time != CurrentTime )
        return False;
    // from qapplication_x11.cpp
    switch ( event->type ) {
    case ButtonPress:
      // fallthrough intended
    case ButtonRelease:
      next_x_time = event->xbutton.time;
      break;
    case MotionNotify:
      next_x_time = event->xmotion.time;
      break;
    case KeyPress:
      // fallthrough intended
    case KeyRelease:
      next_x_time = event->xkey.time;
      break;
    case PropertyNotify:
      next_x_time = event->xproperty.time;
      break;
    case EnterNotify:
    case LeaveNotify:
      next_x_time = event->xcrossing.time;
      break;
    case SelectionClear:
      next_x_time = event->xselectionclear.time;
      break;
    default:
      break;
    }
    return False;
}

void Klipper::updateTimestamp()
{
    static QWidget* w = 0;
    if ( !w )
        w = new QWidget;
    unsigned char data[ 1 ];
    XChangeProperty( QX11Info::display(), w->winId(), XA_ATOM, XA_ATOM, 8, PropModeAppend, data, 1 );
    next_x_time = CurrentTime;
    XEvent dummy;
    XCheckIfEvent( QX11Info::display(), &dummy, update_x_time_predicate, NULL );
    if( next_x_time == CurrentTime )
        {
        XSync( QX11Info::display(), False );
        XCheckIfEvent( QX11Info::display(), &dummy, update_x_time_predicate, NULL );
        }
    Q_ASSERT( next_x_time != CurrentTime );
    QX11Info::setAppTime( next_x_time );
    XEvent ev; // remove the PropertyNotify event from the events queue
    XWindowEvent( QX11Info::display(), w->winId(), PropertyChangeMask, &ev );
}

static const char * const description =
      I18N_NOOP("KDE cut & paste history utility");

void Klipper::createAboutData()
{
  about_data = new KAboutData("klipper", 0, ki18n("Klipper"),
    klipper_version, ki18n(description), KAboutData::License_GPL,
		       ki18n("(c) 1998, Andrew Stanley-Jones\n"
		       "1998-2002, Carsten Pfeiffer\n"
		       "2001, Patrick Dubroy"));

  about_data->addAuthor(ki18n("Carsten Pfeiffer"),
                      ki18n("Author"),
                      "pfeiffer@kde.org");

  about_data->addAuthor(ki18n("Andrew Stanley-Jones"),
                      ki18n( "Original Author" ),
                      "asj@cban.com");

  about_data->addAuthor(ki18n("Patrick Dubroy"),
                      ki18n("Contributor"),
                      "patrickdu@corel.com");

  about_data->addAuthor( ki18n("Luboš Luňák"),
                      ki18n("Bugfixes and optimizations"),
                      "l.lunak@kde.org");

  about_data->addAuthor( ki18n("Esben Mose Hansen"),
                      ki18n("Maintainer"),
                      "kde@mosehansen.dk");
}

void Klipper::destroyAboutData()
{
  delete about_data;
  about_data = NULL;
}

KAboutData* Klipper::about_data;

KAboutData* Klipper::aboutData()
{
  return about_data;
}

static void ensureGlobalSyncOff(KSharedConfigPtr config) {
    KConfigGroup cg(config, "General");
    if ( cg.readEntry( "SynchronizeClipboardAndSelection" , false) ) {
        kDebug() << "Shutting off global synchronization";
        cg.writeEntry("SynchronizeClipboardAndSelection", false, KConfig::Normal | KConfig::Global );
        cg.sync();
        kapp->setSynchronizeClipboard(false);
        KGlobalSettings::self()->emitChange( KGlobalSettings::ClipboardConfigChanged, 0 );
    }

}


#include "klipper.moc"
