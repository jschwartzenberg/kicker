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

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QObject>
#include <QRegion>
#include <QStringList>

#include <plasma_export.h>

class QGraphicsItem;

namespace Plasma
{

class PLASMA_EXPORT Animator : public QObject
{
    Q_OBJECT

public:
    explicit Animator(QObject *parent = 0, const QStringList& list = QStringList());
    ~Animator();

    virtual int appearFrames();
    virtual void appear(int frame, QGraphicsItem* item);

    virtual int disappearFrames();
    virtual void disappear(int frame, QGraphicsItem* item);

    virtual int frameAppearFrames();
    virtual void frameAppear(int frame, QGraphicsItem* item, const QRegion& drawable);

    virtual int activateFrames();
    virtual void activate(int frame, QGraphicsItem* item);

    virtual void renderBackground(QImage& background);
};

}; // Plasma namespace

#endif // multiple inclusion guard