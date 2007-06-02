/*
 *   Copyright (C) 2007 by Siraj Razick siraj@kde.org
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

#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsTextItem>

#include <plasma/dataengine.h>
#include <plasma/plasma_export.h>
#include <plasma/widgets/layoutitem.h>

//TODO
//Please Document this class

namespace Plasma
{

/**
 * Icon class, for URIs and menu popups in panels
 */
class PLASMA_EXPORT Icon : public QObject,
                           public QGraphicsItem,
                           public LayoutItem
{
    Q_OBJECT
    public:
        Icon(QGraphicsItem *parent = 0);
        virtual ~Icon();

        QString text() const;
        void setText(const QString &name);

        QSizeF size() const;
        void setSize(const QSizeF& size);
        void setSize(int height, int width);

        void setIcon(const QString& icon);
        void setIcon(const QIcon& icon);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
         QRectF boundingRect() const;

        //layout stufff
        Qt::Orientations expandingDirections() const;

        QSizeF minimumSize() const;
        QSizeF maximumSize() const;

        bool hasHeightForWidth() const;
        qreal heightForWidth(qreal w) const;

        bool hasWidthForHeight() const;
        qreal widthForHeight(qreal h) const;

        QRectF geometry() const;
        void setGeometry(const QRectF& r);

        QSizeF sizeHint() const;

     Q_SIGNALS:
         void pressed(bool down);
         void clicked();

    protected:
        bool isDown();
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void hoverEnterEvent (QGraphicsSceneHoverEvent * event);
        void hoverLeaveEvent (QGraphicsSceneHoverEvent * event);

    private:
        class Private;
        Private * const d;
};

} // namespace Plasma

#endif