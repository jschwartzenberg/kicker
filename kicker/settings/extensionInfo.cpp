/*
 *  Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>
 *  Copyright (c) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#include <QApplication>
#include <QDesktopWidget>

#include <KConfigGroup>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <klocale.h>

#include "extensionInfo.h"


ExtensionInfo::ExtensionInfo(const QString& desktopFile,
                             const QString& configFile,
                             const QString& configPath)
    : _configFile(configFile),
      _configPath(configPath),
      _desktopFile(desktopFile)
{
    load();
}

void ExtensionInfo::load()
{
    setDefaults();

    if (_desktopFile.isNull())
    {
        _name = i18n("Main Panel");
        _resizeable = true;
        _useStdSizes = true;
        _customSizeMin = 24;
        _customSizeMax = 256;
        _customSize = 56;
        _showLeftHB     = false;
        _showRightHB    = true;
	for (int i=0;i<4;i++) _allowedPosition[i]=true;
    }
    else
    {
        QString trueString ("true");
        KDesktopFile df(_desktopFile);
        _name = df.readName();
        _resizeable = trueString.compare(df.entryMap().value("X-KDE-PanelExt-Resizeable", "true")) == 0;

        if (_resizeable)
        {
            _useStdSizes = trueString.compare(df.entryMap().value("X-KDE-PanelExt-StdSizes", "true")) == 0;
            _size = df.entryMap().value("X-KDE-PanelExt-StdSizeDefault", QString::number(_size)).toInt();
            _customSizeMin = df.entryMap().value("X-KDE-PanelExt-CustomSizeMin", QString::number(_customSizeMin)).toInt();
            _customSizeMax = df.entryMap().value("X-KDE-PanelExt-CustomSizeMax", QString::number(_customSizeMax)).toInt();
            _customSize = df.entryMap().value("X-KDE-PanelExt-CustomSizeDefault", QString::number(_customSize)).toInt();
        }
	QStringList allowedPos=
            df.entryMap().value("X-KDE-PanelExt-Positions","Left,Top,Right,Bottom").toUpper().split(",", QString::SkipEmptyParts );
	for (int i=0;i<4;i++) _allowedPosition[i]=false;
	kDebug()<<"BEFORE X-KDE-PanelExt-Positions parsing";
	for (unsigned int i=0;i<allowedPos.count();i++) {
		kDebug()<<allowedPos[i];
		if (allowedPos[i]=="LEFT") _allowedPosition[Plasma::Left]=true;
		if (allowedPos[i]=="RIGHT") _allowedPosition[Plasma::Right]=true;
		if (allowedPos[i]=="TOP") _allowedPosition[Plasma::Top]=true;
		if (allowedPos[i]=="BOTTOM") _allowedPosition[Plasma::Bottom]=true;
	}	

    }

    // sanitize
    if (_customSizeMin < 0) _customSizeMin = 0;
    if (_customSizeMax < _customSizeMin) _customSizeMax = _customSizeMin;
    if (_customSize < _customSizeMin) _customSize = _customSizeMin;

    KConfig c(_configFile);
    KConfigGroup cg(&c, "General");

    _position       = cg.readEntry ("Position",           _position);
    _alignment      = cg.readEntry ("Alignment",          _alignment);
    _xineramaScreen = cg.readEntry ("XineramaScreen",     _xineramaScreen);
    _showLeftHB     = cg.readEntry ("ShowLeftHideButton", _showLeftHB);
    _showRightHB    = cg.readEntry ("ShowRightHideButton", _showRightHB);
    _hideButtonSize = cg.readEntry ("HideButtonSize",     _hideButtonSize);
    _autohidePanel  = cg.readEntry ("AutoHidePanel",      _autohidePanel);
    _backgroundHide = cg.readEntry ("BackgroundHide",     _backgroundHide);
    _autoHideSwitch = cg.readEntry ("AutoHideSwitch",     _autoHideSwitch);
    _autoHideDelay  = cg.readEntry ("AutoHideDelay",      _autoHideDelay);
    _hideAnim       = cg.readEntry ("HideAnimation",      _hideAnim);
    _hideAnimSpeed  = cg.readEntry ("HideAnimationSpeed", _hideAnimSpeed);
    _unhideLocation = cg.readEntry ("UnhideLocation",     _unhideLocation);
    _sizePercentage = cg.readEntry ("SizePercentage",     _sizePercentage);
    _expandSize     = cg.readEntry ("ExpandSize",         _expandSize);

    if (_resizeable)
    {
        _size           = cg.readEntry ("Size",      _size);
        _customSize     = cg.readEntry ("CustomSize", _customSize);
    }

    _orig_position = _position;
    _orig_alignment = _alignment;
    _orig_size = _size;
    _orig_customSize = _customSize;

    // sanitize
    if (_sizePercentage < 1) _sizePercentage = 1;
    if (_sizePercentage > 100) _sizePercentage = 100;
}

void ExtensionInfo::configChanged()
{
    KConfig c(_configFile);
    KConfigGroup cg(&c, "General");

    // check to see if the new value is different from both
    // the original value and the currently set value, then it
    // must be a newly set value, external to the panel!
    int position  = cg.readEntry ("Position", 3);
    if (position != _position && position != _orig_position)
    {
        _orig_position = _position = position;
    }

    int alignment = cg.readEntry ("Alignment", QApplication::isRightToLeft() ? 2 : 0);
    if (alignment != _alignment && alignment != _orig_alignment)
    {
        _orig_alignment = _alignment = alignment;
    }

    if (_resizeable)
    {
        int size = cg.readEntry ("Size", 2);
        if (size != _size && size != _orig_size)
        {
            _orig_size = _size = size;
        }

        int customSize = cg.readEntry ("CustomSize", 0);
        if (customSize != _customSize && customSize != _orig_customSize)
        {
            _orig_customSize = _customSize = customSize;
        }

    }
}

void ExtensionInfo::setDefaults()
{
    // defaults
    _position       = 3;
    _alignment      = QApplication::isRightToLeft() ? 2 : 0;
    _xineramaScreen = QApplication::desktop()->primaryScreen();
    _size           = 2;
    _showLeftHB     = false;
    _showRightHB    = true;
    _hideButtonSize = 14;
    _autohidePanel  = false;
    _backgroundHide = false;
    _autoHideSwitch = false;
    _autoHideDelay  = 3;
    _hideAnim       = true;
    _hideAnimSpeed  = 40;
    _unhideLocation = 0;
    _sizePercentage = 100;
    _expandSize     = true;
    _customSize     = 0;
    _resizeable     = false;
    _useStdSizes    = false;
    _customSizeMin  = 0;
    _customSizeMax  = 0;
}

void ExtensionInfo::save()
{
    KConfig c(_configFile);
    KConfigGroup cg(&c, "General");

    cg.writeEntry("Position",           _position);
    cg.writeEntry("Alignment",          _alignment);
    cg.writeEntry("XineramaScreen",     _xineramaScreen);
    cg.writeEntry("ShowLeftHideButton", _showLeftHB);
    cg.writeEntry("ShowRightHideButton", _showRightHB);
    cg.writeEntry("AutoHidePanel",      _autohidePanel);
    cg.writeEntry("BackgroundHide",     _backgroundHide);
    cg.writeEntry("AutoHideSwitch",     _autoHideSwitch);
    cg.writeEntry("AutoHideDelay",      _autoHideDelay);
    cg.writeEntry("HideAnimation",      _hideAnim);
    cg.writeEntry("HideAnimationSpeed", _hideAnimSpeed);
    cg.writeEntry("UnhideLocation",     _unhideLocation);
    cg.writeEntry("SizePercentage",     _sizePercentage );
    cg.writeEntry("ExpandSize",         _expandSize );

    // FIXME: this is set only for the main panel and only in the
    // look 'n feel (aka appearance) tab. so we can't save it here
    // this should be implemented properly. - AJS
    //c.writeEntry("HideButtonSize",     _hideButtonSize);

    if (_resizeable)
    {
        cg.writeEntry("Size",      _size);
        cg.writeEntry("CustomSize", _customSize);
    }

    _orig_position = _position;
    _orig_alignment = _alignment;
    _orig_size = _size;
    _orig_customSize = _customSize;

    c.sync();
}
