/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2006 Zack Rusin <zack@kde.org>
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

#include <KAboutData>
#include <KCmdLineArgs>
#include <kdebug.h>
#include <KLocale>

#include "krunnerapp.h"
#include "saverengine.h"
#include "startupid.h"
#include "kscreensaversettings.h" // contains screen saver config
#include "klaunchsettings.h" // contains startup config

#include <X11/extensions/Xrender.h>

static const char description[] = I18N_NOOP( "KDE run command interface" );
static const char version[] = "0.1";

bool KRunnerApp::s_haveCompositeManager = false;

int main(int argc, char* argv[])
{
    KAboutData aboutData( "krunner", 0, ki18n( "Run Command Interface" ),
                          version, ki18n(description), KAboutData::License_GPL,
                          ki18n("(c) 2006, Aaron Seigo") );
    aboutData.addAuthor( ki18n("Aaron J. Seigo"),
                         ki18n( "Author and maintainer" ),
                         "aseigo@kde.org" );

    KCmdLineArgs::init(argc, argv, &aboutData);
    if (!KUniqueApplication::start()) {
        return 0;
    }

    // thanks to zack rusin and frederik for pointing me in the right direction
    // for the following bits of X11 code
//     Display *dpy = XOpenDisplay(0); // open default display
//     if (!dpy)
//     {
//         kError() << "Cannot connect to the X server" << endl;
//         return 1;
//     }
// 
//     bool argbVisual = false ;
//     KRunnerApp::s_haveCompositeManager = KWindowSystem::compositingActive();
// 
//     Colormap colormap = 0;
//     Visual *visual = 0;
// 
//     if (KRunnerApp::s_haveCompositeManager)
//     {
//         int screen = DefaultScreen(dpy);
//         int eventBase, errorBase;
// 
//         if (XRenderQueryExtension(dpy, &eventBase, &errorBase))
//         {
//             int nvi;
//             XVisualInfo templ;
//             templ.screen  = screen;
//             templ.depth   = 32;
//             templ.c_class = TrueColor;
//             XVisualInfo *xvi = XGetVisualInfo(dpy, VisualScreenMask |
//                                                    VisualDepthMask |
//                                                    VisualClassMask,
//                                               &templ, &nvi);
//             for (int i = 0; i < nvi; ++i)
//             {
//                 XRenderPictFormat *format = XRenderFindVisualFormat(dpy,
//                                                                     xvi[i].visual);
//                 if (format->type == PictTypeDirect && format->direct.alphaMask)
//                 {
//                     visual = xvi[i].visual;
//                     colormap = XCreateColormap(dpy, RootWindow(dpy, screen),
//                                                visual, AllocNone);
//                     argbVisual = true;
//                     break;
//                 }
//             }
//         }
// 
//         KRunnerApp::s_haveCompositeManager = argbVisual;
//     }
// 
//     kDebug() << "KRunnerApp::s_haveCompositeManager: " << KRunnerApp::s_haveCompositeManager;

    KRunnerApp app;

//     if (KRunnerApp::s_haveCompositeManager) {
//         app = new KRunnerApp(dpy, Qt::HANDLE(visual), Qt::HANDLE(colormap));
//     } else {
//         app = new KRunnerApp;//(dpy, 0, 0);
//     }

    int rc = app.exec();
    //delete app;
    return rc;
}

