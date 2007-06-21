/*
 *   Copyright (C) 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "layout.h"

#include <QtCore/QList>

namespace Plasma
{

class Layout::Private
{
	public:
		Private() : margin(10.0), spacing(10.0), parent(0) {}
		~Private() {}

		qreal margin;
		qreal spacing;

		LayoutItem *parent;
};


Layout::Layout(LayoutItem *parent)
	: LayoutItem(),
	  d(new Private)
{
	d->parent = parent;
}

Layout::~Layout()
{
}

qreal Layout::margin() const
{
	return d->margin;
}

void Layout::setMargin(qreal m)
{
	d->margin = m;
}

qreal Layout::spacing() const
{
	return d->spacing;
}

void Layout::setSpacing(qreal s)
{
	d->spacing = s;
}

LayoutItem *Layout::parent() const
{
	return d->parent;
}

}