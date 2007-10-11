/*
*   Copyright 2007 by Matt Broadstone <mbroadst@kde.org>
*   Copyright 2007 by Robert Knight <robertknight@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2, 
*   or (at your option) any later version.
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

#ifndef PLASMA_PANELVIEW_H
#define PLASMA_PANELVIEW_H

#include <QGraphicsView>

#include <plasma/plasma.h>

class QWidget;

namespace Plasma
{
    class Containment;
    class Corona;
    class Svg;
}

class PanelView : public QGraphicsView
{
    Q_OBJECT
public:

   /**
    * Constructs a new panelview.
    * @arg parent the QWidget this panel is parented to
    */
    PanelView(Plasma::Containment *panel, QWidget *parent = 0);

    /**
     * Sets the location (screen edge) where this panel is positioned.
     * @param location the location to place the panel at
     */
    void setLocation(Plasma::Location location);

    /**
     * @return the location (screen edge) where this panel is positioned.
     */
    Plasma::Location location() const;

    /**
     * @return the Containment associated with this panel.
     */
    Plasma::Containment *containment() const;

    /**
     * @return the Corona (scene) associated with this panel.
     */
    Plasma::Corona *corona() const;

protected:
    void updateStruts();

    virtual void moveEvent(QMoveEvent *event);

    virtual void resizeEvent(QResizeEvent *event);

private:
    void updatePanelGeometry();

    Plasma::Containment *m_containment;
    Plasma::Svg *m_background;
};

#endif
