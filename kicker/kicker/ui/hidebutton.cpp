/* This file is part of the KDE project
   Copyright (C) 2003-2004 Nadeem Hasan <nhasan@kde.org>
   Copyright (C) 2004 Aaron J. Seigo <aseigo@kde.org>

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

#include "hidebutton.h"

#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QEvent>

#include <kcursor.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <kstandarddirs.h>

HideButton::HideButton(QWidget *parent)
    : QAbstractButton(parent),
      m_highlight(false),
      m_arrow(Qt::LeftArrow)
{
    setBackgroundOrigin(AncestorOrigin);

    connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)));
    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)), SLOT(slotIconChanged(int)));

    slotSettingsChanged(KGlobalSettings::SETTINGS_MOUSE);
}

void HideButton::paintEvent(QPaintEvent*)
{
    QPainter painter( this );
    drawButton( &painter );
}

void HideButton::drawButton(QPainter *p)
{
    if (m_arrow == Qt::LeftArrow)
    {
        p->setPen(palette().color(QPalette::Mid));
        p->drawLine(width()-1, 0, width()-1, height());
    }
    else if (m_arrow == Qt::RightArrow)
    {
        p->setPen(palette().color(QPalette::Mid));
        p->drawLine(0, 0, 0, height());
    }
    else if (m_arrow == Qt::UpArrow)
    {
        p->setPen(palette().color(QPalette::Mid));
        p->drawLine(0, height()-1, width(), height()-1);
    }
    else if (m_arrow == Qt::DownArrow)
    {
        p->setPen(palette().color(QPalette::Mid));
        p->drawLine(0, 0, width(), 0);
    }

    drawButtonLabel(p);
}

void HideButton::drawButtonLabel(QPainter *p)
{
    QPixmap pix = m_highlight ? m_activeIcon : m_normalIcon;

    if (isChecked() || isDown())
    {
        p->translate(2, 2);
    }

    QPoint origin(2, 2);
    int w = width();
    int h = height();
    int pw = pix.width();
    int ph = pix.height();

    if (ph < (h - 4))
    {
        origin.setY(origin.y() + ((h - ph) / 2));
    }

    if (pw < (w - 4))
    {
        origin.setX(origin.x() + ((w - pw) / 2));
    }

    p->drawPixmap(origin, pix);
}

void HideButton::setPixmap(const QPixmap &pix)
{
    setIcon(pix);
    generateIcons();
}

void HideButton::setArrowType(Qt::ArrowType arrow)
{
    m_arrow = arrow;
    switch (arrow)
    {
        case Qt::LeftArrow:
            setPixmap(SmallIcon("arrow-left"));
        break;

        case Qt::RightArrow:
            setPixmap(SmallIcon("arrow-right"));
        break;

        case Qt::UpArrow:
            setPixmap(SmallIcon("arrow-up"));
        break;

        case Qt::DownArrow:
        default:
            setPixmap(SmallIcon("arrow-down"));
        break;
    }
}

void HideButton::generateIcons()
{
    if (icon().isNull())
    {
        m_normalIcon = QPixmap();
        m_activeIcon = QPixmap();
        return;
    }

    QPixmap pix = icon().pixmap();
    pix = pix.scaled(size() - QSize(4, 4), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    KIconEffect effect;
    m_normalIcon = effect.apply(pix, KIconLoader::Panel, KIconLoader::DefaultState);
    m_activeIcon = effect.apply(pix, KIconLoader::Panel, KIconLoader::ActiveState);
}

void HideButton::slotSettingsChanged(int category)
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

void HideButton::slotIconChanged(int group)
{
    if (group != KIconLoader::Panel)
    {
        return;
    }

    generateIcons();
    repaint();
}

void HideButton::enterEvent(QEvent *e)
{
    m_highlight = true;

    repaint();
    QAbstractButton::enterEvent(e);
}

void HideButton::leaveEvent(QEvent *e)
{
    m_highlight = false;

    repaint();
    QAbstractButton::enterEvent(e);
}

void HideButton::resizeEvent(QResizeEvent *)
{
    generateIcons();
}

#include "hidebutton.moc"

// vim:ts=4:sw=4:et
