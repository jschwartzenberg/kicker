/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Sebastian Kuegler <sebas@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef BATTERY_H
#define BATTERY_H

#include <QLabel>
#include <QGraphicsSceneHoverEvent>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include "ui_batteryConfig.h"



class KDialog;

namespace Plasma
{
    class Svg;
}

class Battery : public Plasma::Applet
{
    Q_OBJECT
    public:
        Battery(QObject *parent, const QVariantList &args);
        ~Battery();

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            const QRect &contents);
        void setPath(const QString&);
        QSizeF contentSizeHint() const;
        //QSizeF contentSize() const;
        void constraintsUpdated(Plasma::Constraints constraints);

    public slots:
        void updated(const QString &name, const Plasma::DataEngine::Data &data);
        void showConfigurationInterface();
 
    protected Q_SLOTS:
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    protected slots:
        void configAccepted();

    private:
        /* Paint a label on top of the battery
         *
         */
        void paintLabel(QPainter *p, const QString& labelText);
        /* Should the battery charge information be shown on top?
         */
        bool m_showBatteryString;
        QSizeF m_size;
        int m_pixelSize;
        int m_smallPixelSize;
        Plasma::Svg* m_theme;
        QString m_battery_percent_label;
        int m_battery_percent;
        bool m_acadapter_plugged;

        // Configuration dialog
        KDialog *m_dialog;
        Ui::batteryConfig ui;
        
        // Internal data
        QList<QVariant> batterylist, acadapterlist;
        QFont m_font;
        bool m_isHovered;
        bool m_hasBattery;
        bool m_drawBackground;
        QColor m_boxColor;
        QColor m_textColor;
        int m_boxAlpha;
        int m_boxHoverAlpha;
};

K_EXPORT_PLASMA_APPLET(battery, Battery)

#endif