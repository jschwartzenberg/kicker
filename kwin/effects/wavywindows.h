/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_WAVYWINDOWS_H
#define KWIN_WAVYWINDOWS_H

// Include with base class for effects.
#include <effects.h>


namespace KWinInternal
{

/**
 * Demo effect which applies waves to all windows
 **/
class WavyWindowsEffect
    : public Effect
    {
    public:
        WavyWindowsEffect( Workspace* ws );

        virtual void prePaintScreen( int* mask, QRegion* region, int time );
        virtual void prePaintWindow( EffectWindow* w, int* mask, QRegion* region, int time );
        virtual void paintWindow( EffectWindow* w, int mask, QRegion region, WindowPaintData& data );
        virtual void postPaintScreen();

    private:
        Workspace* mWorkspace;
        float mTimeElapsed;
    };

} // namespace

#endif