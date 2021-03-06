/* This file is part of the KDE project
   Copyright (C) 2003-2004 Nadeem Hasan <nhasan@kde.org>
   Copyright (C) 2004-2005 Aaron J. Seigo <aseigo@kde.org>

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

#include "simplebutton.h"

#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QPaintEvent>

#include <kapplication.h>
#include <kcursor.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kicontheme.h>
#include <kstandarddirs.h>

class SimpleButton::Private
{
public:
    Private()
     : highlight(false), orientation(Qt::Horizontal)
    {}
    bool highlight;
    QPixmap normalIcon;
    QPixmap activeIcon;
    Qt::Orientation orientation;
};

SimpleButton::SimpleButton(QWidget *parent, const char *name)
    : QAbstractButton(parent),
      d(new Private())
{
    setObjectName( name );

    connect( KGlobalSettings::self(), SIGNAL( settingsChanged( int ) ),
       SLOT( slotSettingsChanged( int ) ) );
    connect( KGlobalSettings::self(), SIGNAL( iconChanged( int ) ),
       SLOT( slotIconChanged( int ) ) );

    slotSettingsChanged( KGlobalSettings::SETTINGS_MOUSE );
}

SimpleButton::SimpleButton(QWidget *parent)
    : QAbstractButton(parent),
      d(new Private())
{
    connect( KGlobalSettings::self(), SIGNAL( settingsChanged( int ) ),
       SLOT( slotSettingsChanged( int ) ) );
    connect( KGlobalSettings::self(), SIGNAL( iconChanged( int ) ),
       SLOT( slotIconChanged( int ) ) );

    slotSettingsChanged( KGlobalSettings::SETTINGS_MOUSE );
}

SimpleButton::~SimpleButton()
{
    delete d;
}

void SimpleButton::setIcon(const QIcon &icon)
{
    QAbstractButton::setIcon(icon);
    generateIcons();
    update();
}

void SimpleButton::setOrientation(Qt::Orientation orientation)
{
    d->orientation = orientation;
    update();
}

QSize SimpleButton::sizeHint() const
{
    const QPixmap* pm = pixmap();

    if (!pm)
    {
        return QAbstractButton::sizeHint();
    }

    if (d->orientation == Qt::Horizontal)
    {
        return QSize(pm->width() + KDialog::spacingHint(), pm->height());
    }
    else
    {
        return QSize(pm->width(), pm->height() + KDialog::spacingHint());
    }
}

void SimpleButton::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    drawButton( &p );
    p.end();
}

void SimpleButton::drawButton( QPainter *p )
{
    p->setPen(palette().mid().color());

    if (d->orientation == Qt::Vertical)
    {
        p->drawLine(0, height() - 1, width(), height() - 1);
    }
    else if (kapp->layoutDirection() == Qt::RightToLeft)
    {
        p->drawLine(0, 0, 0, height());
    }
    else
    {
        p->drawLine(width() - 1, 0, width() - 1, height());
    }

    drawButtonLabel(p);
}

void SimpleButton::drawButtonLabel( QPainter *p )
{
    if (!pixmap())
    {
        return;
    }

    QPixmap pix = d->highlight ? d->activeIcon : d->normalIcon;

    if (isChecked() || isDown())
    {
        pix = pix.scaled(pix.width() - 2, pix.height() - 2,
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    int h = height();
    int w = width();
    int ph = pix.height();
    int pw = pix.width();
    int margin = KDialog::spacingHint();
    QPoint origin(margin / 2, margin / 2);

    if (ph < (h - margin))
    {
        origin.setY((h - ph) / 2);
    }

    if (pw < (w - margin))
    {
        origin.setX((w - pw) / 2);
    }

    p->drawPixmap(origin, pix);
}

void SimpleButton::generateIcons()
{
    if (!pixmap())
    {
        return;
    }

    KIconEffect effect;
    QPixmap pix = *pixmap();

    d->normalIcon = effect.apply(pix, KIconLoader::Panel, KIconLoader::DefaultState);
    d->activeIcon = effect.apply(pix, KIconLoader::Panel, KIconLoader::ActiveState);
}

void SimpleButton::slotSettingsChanged(int category)
{
    if (category != KGlobalSettings::SETTINGS_MOUSE)
    {
        return;
    }

    bool changeCursor = KGlobalSettings::changeCursorOverIcon();

    if (changeCursor)
    {
        setCursor(Qt::PointingHandCursor);
    }
    else
    {
        unsetCursor();
    }
}

void SimpleButton::slotIconChanged( int group )
{
    if (group != KIconLoader::Panel)
    {
        return;
    }

    generateIcons();
    repaint();
}

void SimpleButton::enterEvent( QEvent *e )
{
    d->highlight = true;

    repaint();
    QAbstractButton::enterEvent( e );
}

void SimpleButton::leaveEvent( QEvent *e )
{
    d->highlight = false;

    repaint();
    QAbstractButton::enterEvent( e );
}

void SimpleButton::resizeEvent( QResizeEvent * )
{
    generateIcons();
}

#include "simplebutton.moc"

// vim:ts=4:sw=4:et
