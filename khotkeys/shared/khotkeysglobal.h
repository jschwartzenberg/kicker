/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KHOTKEYSGLOBAL_H_
#define _KHOTKEYSGLOBAL_H_

#define KHOTKEYS_VERSION "2.0pre2"
#define KHOTKEYS_CONFIG_FILE "khotkeysrc"

//#ifndef NDEBUG
//#define KHOTKEYS_DEBUG
//#endif

#include <qstring.h>

class KConfig;
class QObject;

namespace KHotKeys
{

class Kbd;
class Windows;
class Action_data_group;

extern Kbd* keyboard_handler;
extern Windows* windows_handler;

#define KHOTKEYS_DISABLE_COPY( cls ) private: cls( const cls& ); cls& operator=( const cls& )

// CHECKME hmms :(
bool khotkeys_active();
void khotkeys_set_active( bool active_P );

QString get_menu_entry_from_path( const QString& path_P );

void init_global_data( bool active_P, QObject* owner_P );

const char* const MENU_EDITOR_ENTRIES_GROUP_NAME = "Menu Editor entries";

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
