/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
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

#ifndef TIMEENGINE_H
#define TIMEENGINE_H

#include <KGenericFactory>

#include "dataengine.h"

class QTimer;

/**
 * This class evaluates the basic expressions given in the interface.
 */
class TimeEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        TimeEngine( QObject* parent, const QStringList& args );
        ~TimeEngine();

    protected:
        void init();

    protected slots:
        void updateTime();

    private:
        QTimer* m_timer;
};

K_EXPORT_PLASMA_DATAENGINE( time, TimeEngine )

#endif