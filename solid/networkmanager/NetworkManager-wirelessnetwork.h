/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef NETWORKMANAGER_WIRELESSNETWORK_H
#define NETWORKMANAGER_WIRELESSNETWORK_H

#include <kdelibs_export.h>

#include <QStringList>
#include <qdbusextratypes.h>

#include <solid/ifaces/wirelessnetwork.h>

#include "NetworkManager-network.h"

struct NMDBusWirelessNetworkProperties
{
    QDBusObjectPath path;
    QString essid;
    QString hwAddr;
    int strength;
    double frequency;
    int rate;
    int mode;
    int capabilities;
    bool broadcast;
};

//typedef QString MacAddress;
//typedef QStringList MacAddressList;

class Authentication;
class NMWirelessNetworkPrivate;

class KDE_EXPORT NMWirelessNetwork : public NMNetwork
{
Q_OBJECT
public:
    NMWirelessNetwork( const QString & networkPath );
    virtual ~NMWirelessNetwork();
    int signalStrength() const;
    int bitRate() const;
    double frequency() const;
    Solid::WirelessNetwork::Capabilities capabilities() const;
    QString essid() const;
    Solid::WirelessNetwork::OperationMode mode() const;
    bool isAssociated() const; // move to Device, is this a property on device?
    bool isEncrypted() const;
    bool isHidden() const;
    MacAddressList bssList() const;
    Authentication *authentication() const;
    void setAuthentication( Authentication *authentication );
Q_SIGNALS:
    void signalStrengthChanged( int strength );
    void bitrateChanged( int bitrate );
    void associationChanged( bool associated ); // move to Device?
    void authenticationNeeded();
private:
    NMWirelessNetworkPrivate * d;
};

#endif