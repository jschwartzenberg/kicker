/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#ifndef RESTARTINGAPPLICATION_H
#define RESTARTINGAPPLICATION_H

#include <Qt>
#include <KUniqueApplication>

class RestartingApplication : public KUniqueApplication
{
    Q_OBJECT

    public:
        RestartingApplication(Display *display,
                              Qt::HANDLE visual = 0,
                              Qt::HANDLE colormap = 0);
        RestartingApplication() : KUniqueApplication() {}
    private slots:
        void setCrashHandler();

    private:
        static void crashHandler(int signal);
};

#endif
