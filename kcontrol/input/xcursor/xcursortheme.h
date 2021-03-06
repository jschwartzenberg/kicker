/*
 * Copyright © 2006-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef XCURSORTHEME_H
#define XCURSORTHEME_H

#include <QList>
#include <QHash>
#include <QPixmap>

#include "legacytheme.h"

class QDir;

struct _XcursorImage;
struct _XcursorImages;

typedef _XcursorImage XcursorImage;
typedef _XcursorImages XcursorImages;

/**
 * The XCursorTheme class is a CursorTheme implementation for Xcursor themes.
 */
class XCursorTheme : public LegacyTheme
{
    public:
       /**
        * Initializes itself from the @p dir information, and parses the
        * index.theme file if the dir has one.
        */
        XCursorTheme(const QDir &dir);
        virtual ~XCursorTheme() {}

        const QStringList inherits() const { return m_inherits; }	
        QImage loadImage(const QString &name, int size = -1) const;
        QCursor loadCursor(const QString &name, int size = -1) const;

    protected:
        XCursorTheme(const QString &title, const QString &desc)
            : LegacyTheme(title, desc) {}
        void setInherits(const QStringList &val) { m_inherits = val; }

    private:
        XcursorImage *xcLoadImage(const QString &name, int size) const;
        XcursorImages *xcLoadImages(const QString &name, int size) const;
        void parseIndexFile();
        QString findAlternative(const QString &name) const;

        QStringList m_inherits;
        static QHash<QString, QString> alternatives;
};

#endif // XCURSORTHEME_H

