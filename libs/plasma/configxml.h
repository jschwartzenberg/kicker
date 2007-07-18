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

#ifndef CONFIGXML_H
#define CONFIGXML_H

#include <KConfigSkeleton>

#include <plasma/plasma_export.h>

/**
 * @brief A KConfigSkeleton that populates itself based on KConfigXT XML
 *
 * This class allows one to ship an XML file and reconstitute it into a
 * KConfigSkeleton object at runtime. Common usage might look like this:
 * 
 * \code
 * QFile file(xmlFilePath);
 * Plasma::ConfigXml appletConfig(configFilePath, &file);
 * \endcode
 *
 * Alternatively, any QIODevice may be used in place of QFile in the
 * example above.
 *
 * Currently the following data types are supported:
 *
 * @li bools
 * @li colors
 * @li datetimes
 * @li enumerations
 * @li fonts
 * @li ints
 * @li passwords
 * @li paths
 * @li strings
 * @li stringlists
 * @li uints
 * @li urls
 *
 * The following data types which are supported by KConfigSkeleton
 * are not yet supported by ConfigXml's XML parsing:
 *
 * @li doubles
 * @li int lists
 * @li longlongs
 * @li path lists
 * @li points
 * @li rects
 * @li sizes
 * @li ulonglongs
 * @li url lists
 **/

namespace Plasma
{

class PLASMA_EXPORT ConfigXml : public KConfigSkeleton
{
public:
    /**
     * Creates a KConfigSkeleton populated using the definition found in
     * the XML data passed in.
     *
     * @param configFile path to the configuration file to use
     * @param xml the xml data; must be valid KConfigXT data
     * @param parent optional QObject parent
     **/
    ConfigXml(const QString &configFile, QIODevice *xml, QObject *parent = 0);
    ~ConfigXml();

    class Private;
private:
    Private * const d;
};

} // Plasma namespace

#endif //multiple inclusion guard