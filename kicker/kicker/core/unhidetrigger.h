/*****************************************************************

Copyright (c) 2002 John Firebaugh <jfirebaugh@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __UnhideTrigger_h__
#define __UnhideTrigger_h__

// Fix compilation with --enable-final
#ifdef None
#undef None
#endif

#include <QObject>

#include "utils.h"

class UnhideTrigger : public QObject
{
    Q_OBJECT
    public:
        static UnhideTrigger* self();

        void setEnabled( bool enable );
        bool isEnabled() const;

        // this is called whenever an item accepts a trigger, thereby preventing further calling of it again
        void triggerAccepted(Plasma::ScreenEdge t, int XineramaScreen);
        void resetTriggerThrottle();

    Q_SIGNALS:
        void triggerUnhide(Plasma::ScreenEdge t, int XineramaScreen);

    private Q_SLOTS:
        void pollMouse();

    private:
        UnhideTrigger();

        void emitTrigger(Plasma::ScreenEdge t , int XineramaScreen);
        Plasma::ScreenEdge m_lastTrigger;
        int m_lastXineramaScreen;
        QTimer *m_timer;
        int m_enabledCount;
};

#endif
