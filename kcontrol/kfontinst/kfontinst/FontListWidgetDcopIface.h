#ifndef __FONT_LIST_WIDGET_DCOP_IFACE_H__
#define __FONT_LIST_WIDGET_DCOP_IFACE_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontListWidgetDcopIface
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 03/08/2002
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002
////////////////////////////////////////////////////////////////////////////////

#include <dcopobject.h>
#include <qstring.h>

class CFontListWidgetDcopIface : virtual public DCOPObject
{
    K_DCOP
    k_dcop:

    virtual void installFonts(QString list)=0;
    virtual bool ready()=0;
};

#endif
