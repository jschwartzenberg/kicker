/***************************************************************************
 *   Copyright (C) 2001 by Matthias Hoelzer-Kluepfel <mhk@caldera.de>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef __USB_DEVICES_H__
#define __USB_DEVICES_H__



#include <Qt3Support/Q3PtrList>

#if defined(__DragonFly__)
#include <bus/usb/usb.h>
#elif defined(Q_OS_FREEBSD)
#include <dev/usb/usb.h>
#endif

class USBDB;


class USBDevice
{
public:
  
  USBDevice();

  void parseLine(const QString &line);
  void parseSysDir(int bus, int parent, int level, const QString &line);

  int level() const { return _level; }
  int device() const { return _device; }
  int parent() const { return _parent; }
  int bus() const { return _bus; }
  QString product();

  QString dump();

  static Q3PtrList<USBDevice> &devices() { return _devices; }
  static USBDevice *find(int bus, int device);
  static bool parse(const QString& fname);
  static bool parseSys(const QString& fname);


private:

  static Q3PtrList<USBDevice> _devices;

  static USBDB *_db;

  int _bus, _level, _parent, _port, _count, _device, _channels, _power;
  float _speed;

  QString _manufacturer, _product, _serial;

  int _bwTotal, _bwUsed, _bwPercent, _bwIntr, _bwIso;
  bool _hasBW;

  unsigned int _verMajor, _verMinor, _class, _sub, _prot, _maxPacketSize, _configs;
  QString _className;

  unsigned int _vendorID, _prodID, _revMajor, _revMinor;

#ifdef Q_OS_FREEBSD
  void collectData( int fd, int level, usb_device_info &di, int parent );
  QStringList _devnodes;
#endif
};


#endif
