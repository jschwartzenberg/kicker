/*
 *  Copyright (C) 2006 Andriy Rysin (rysin@kde.org)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QtGui/QWidgetList>
#include <QDir>


#include <QHash>
#include <QRegExp>
#include <QTextStream>

#include <kdebug.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#define explicit int_explicit        // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#undef explicit
#include <X11/extensions/XKBrules.h>

#include "x11helper.h"
#include <config-workspace.h>

#ifndef HAVE_XKLAVIER

// Compiler will size array automatically.
static const char* X11DirList[] =
    {
        XLIBDIR,
        "/etc/X11/",
        "/usr/share/X11/",
        "/usr/local/share/X11/",
        "/usr/X11R6/lib/X11/",
        "/usr/X11R6/lib64/X11/",
        "/usr/local/X11R6/lib/X11/",
        "/usr/local/X11R6/lib64/X11/",
        "/usr/lib/X11/",
        "/usr/lib64/X11/",
        "/usr/local/lib/X11/",
        "/usr/local/lib64/X11/",
        "/usr/pkg/share/X11/",
        "/usr/pkg/xorg/lib/X11/"
    };

// Compiler will size array automatically.
static const char* rulesFileList[] =
    {
	"xkb/rules/xorg",
	"xkb/rules/xfree86"
    };

// Macro will return number of elements in any static array as long as the
// array has at least one element.
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static const int X11_DIR_COUNT = ARRAY_SIZE(X11DirList);
static const int X11_RULES_COUNT = ARRAY_SIZE(rulesFileList);

const QString
X11Helper::findX11Dir()
{
	for(int ii=0; ii<X11_DIR_COUNT; ii++) {
		const char* xDir = X11DirList[ii];
		if( xDir != NULL && QDir(QString(xDir) + "xkb").exists() ) {
//			for(int jj=0; jj<X11_RULES_COUNT; jj++) {
//
//			}
			return QString(xDir);
	    }

//	  if( X11_DIR.isEmpty() ) {
//	    return;
//	  }
	}
	return NULL;
}

const QString
X11Helper::findXkbRulesFile(const QString &x11Dir, Display *dpy)
{
	QString rulesFile;
	XkbRF_VarDefsRec vd;
	char *tmp = NULL;

	if (XkbRF_GetNamesProp(dpy, &tmp, &vd) && tmp != NULL ) {
// 			kDebug() << "namesprop " << tmp ;
	  	rulesFile = x11Dir + QString("xkb/rules/%1").arg(tmp);
// 			kDebug() << "rulesF " << rulesFile ;
	}
	else {
    // old way
    	for(int ii=0; ii<X11_RULES_COUNT; ii++) {
    		const char* ruleFile = rulesFileList[ii];
     		QString xruleFilePath = x11Dir + ruleFile;
  			kDebug() << "trying xrules path " << xruleFilePath;
		    if( QFile(xruleFilePath).exists() ) {
				rulesFile = xruleFilePath;
			break;
		    }
    	}
    }

	return rulesFile;
}

RulesInfo*
X11Helper::loadRules(const QString& file, bool layoutsOnly)
{
	XkbRF_RulesPtr xkbRules = XkbRF_Load(QFile::encodeName(file).data(), (char*)"", true, true);

	if (xkbRules == NULL) {
// throw Exception
		return NULL;
	}

	RulesInfo* rulesInfo = new RulesInfo();

	for (int i = 0; i < xkbRules->layouts.num_desc; ++i) {
		QString layoutName(xkbRules->layouts.desc[i].name);
		rulesInfo->layouts.insert( layoutName, xkbRules->layouts.desc[i].desc );
	}

	if( layoutsOnly == true ) {
		XkbRF_Free(xkbRules, true);
		return rulesInfo;
	}

  for (int i = 0; i < xkbRules->models.num_desc; ++i)
      rulesInfo->models.insert(xkbRules->models.desc[i].name, QString( xkbRules->models.desc[i].desc ) );

  for (int i = 0; i < xkbRules->options.num_desc; ++i) {
	  QString optionName = xkbRules->options.desc[i].name;

	  int colonPos = optionName.indexOf(':');
	  QString groupName = optionName.mid(0, colonPos);


	  if( colonPos != -1 ) {
kDebug() << " option: " << optionName;

		if( ! rulesInfo->optionGroups.contains( groupName ) ) {
		  rulesInfo->optionGroups.insert(groupName, createMissingGroup(groupName));
kDebug() << " added missing option group: " << groupName;
		}

		XkbOption option;
		option.name = optionName;
		option.description = xkbRules->options.desc[i].desc;
		option.group = &rulesInfo->optionGroups[ groupName ];

		rulesInfo->options.insert(optionName, option);
	  }
	  else {
	    XkbOptionGroup optionGroup;
		optionGroup.name = groupName;
		optionGroup.description = xkbRules->options.desc[i].desc;
		optionGroup.exclusive = isGroupExclusive( groupName );

kDebug() << " option group: " << groupName;
		rulesInfo->optionGroups.insert(groupName, optionGroup);
	  }
  }

  XkbRF_Free(xkbRules, true);

/*
   for(QHashIterator<QString, QString> it(rulesInfo->options) ; it.hasNext();  ) {
	  it.next();

	  QString option(it.key());
	  int columnPos = option.indexOf(":");

	  if( columnPos != -1 ) {
		  QString group = option.mid(0, columnPos);
		  if( !rulesInfo->options.contains(group) ) {
			  rulesInfo->options.insert(group, group);
			  kDebug() << "Added missing option group: " << group;
		  }
	  }
  }
  */
//   // workaround for empty misc options group description in XFree86 4.4.0
//   if( rulesInfo->options.find("numpad:microsoft") && !rulesInfo->options.find("misc") ) {
//     rulesInfo->options.replace("misc", "Miscellaneous compatibility options" );
//   }

  return rulesInfo;
}


XkbOptionGroup
X11Helper::createMissingGroup(const QString& groupName)
{
// workaround for empty 'compose' options group description
   XkbOptionGroup optionGroup;
   optionGroup.name = groupName;
   optionGroup.exclusive = isGroupExclusive( groupName );

   if( groupName == "compose" ) {
		optionGroup.description = "Compose Key Position";
   }
   return optionGroup;
}

bool
X11Helper::isGroupExclusive(const QString& groupName)
{
    if( groupName == "ctrl" || groupName == "caps" || groupName == "altwin" )
		return true;

	return false;
}


/* pretty simple algorithm - reads the layout file and
    tries to find "xkb_symbols"
    also checks whether previous line contains "hidden" to skip it
*/
QStringList*
X11Helper::getVariants(const QString& layout, const QString& x11Dir)
{
  QStringList* result = new QStringList();

  QString file = x11Dir + "xkb/symbols/";
  // workaround for XFree 4.3 new directory for one-group layouts
  if( QDir(file+"pc").exists() )
    file += "pc/";

  file += layout;

//  kDebug() << "reading variants from " << file;

  QFile f(file);
  if (f.open(QIODevice::ReadOnly))
    {
      QTextStream ts(&f);

      QString line;
      QString prev_line;

	  while ( ts.status() == QTextStream::Ok ) {
    	prev_line = line;

		QString str = ts.readLine();
		if( str.isNull() )
		  break;

		line = str.simplified();

	    if (line[0] == '#' || line.left(2) == "//" || line.isEmpty())
		continue;

	    int pos = line.indexOf("xkb_symbols");
	    if (pos < 0)
		continue;

	    if( prev_line.indexOf("hidden") >=0 )
		continue;

	    pos = line.indexOf('"', pos) + 1;
	    int pos2 = line.indexOf('"', pos);
	    if( pos < 0 || pos2 < 0 )
		continue;

	    result->append(line.mid(pos, pos2-pos));
//  kDebug() << "adding variant " << line.mid(pos, pos2-pos);
      }

      f.close();
    }

	return result;
}

#endif

const QString X11Helper::X11_WIN_CLASS_ROOT = "<root>";
const QString X11Helper::X11_WIN_CLASS_UNKNOWN = "<unknown>";

QString
X11Helper::getWindowClass(WId winId, Display* dpy)
{
  unsigned long nitems_ret, bytes_after_ret;
  unsigned char* prop_ret;
  Atom     type_ret;
  int      format_ret;
  Window w = (Window)winId;	// suppose WId == Window
  QString  property;

  if( winId == X11Helper::UNKNOWN_WINDOW_ID ) {
	  kDebug() << "Got window class for " << winId << ": '" << X11_WIN_CLASS_ROOT << "'";
	  return X11_WIN_CLASS_ROOT;
  }

//  kDebug() << "Getting window class for " << winId;
  if((XGetWindowProperty(dpy, w, XA_WM_CLASS, 0L, 256L, 0, XA_STRING,
			&type_ret, &format_ret, &nitems_ret,
			&bytes_after_ret, &prop_ret) == Success) && (type_ret != None)) {
    property = QString::fromLocal8Bit(reinterpret_cast<char*>(prop_ret));
    XFree(prop_ret);
  }
  else {
	  property = X11_WIN_CLASS_UNKNOWN;
  }
  kDebug() << "Got window class for " << winId << ": '" << property << "'";

  return property;
}
