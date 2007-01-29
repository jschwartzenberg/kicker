/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_MINIMIZEANIMATION_H
#define KWIN_MINIMIZEANIMATION_H

// Include with base class for effects.
#include <effects.h>


namespace KWinInternal
{

/**
 * Animates minimize/unminimize
 **/
class MinimizeAnimationEffect
    : public Effect
    {
    public:
        MinimizeAnimationEffect( Workspace* ws );

        virtual void prePaintScreen( int* mask, QRegion* region, int time );
        virtual void prePaintWindow( EffectWindow* w, int* mask, QRegion* region, int time );
        virtual void paintWindow( EffectWindow* w, int mask, QRegion region, WindowPaintData& data );
        virtual void postPaintScreen();

        virtual void windowMinimized( EffectWindow* c );
        virtual void windowUnminimized( EffectWindow* c );

    protected:
        Client* findParentWithIconGeometry( Client* c );

    private:
        Workspace* mWorkspace;
        QMap< EffectWindow*, float > mAnimationProgress;
        int mActiveAnimations;
    };

} // namespace

#endif