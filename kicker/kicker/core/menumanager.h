/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#ifndef KICKER_MENU_MANAGER_H
#define KICKER_MENU_MANAGER_H

#include "panelbutton.h"
#include <QList>
#include <kdemacros.h>
class PanelKMenu;
class QMenu;

typedef QList<PanelPopupButton*> KButtonList;

/**
 * The factory for menus created by other applications. Also the owner of these menus.
 */
class KDE_EXPORT MenuManager : public QObject
{
    Q_OBJECT
public:
    static MenuManager* self();
    ~MenuManager();

    // KMenu controls
    PanelKMenu* kmenu() { return m_kmenu; }
    void showKMenu();
    void popupKMenu(const QPoint &p);

    void registerKButton(PanelPopupButton *button);
    void unregisterKButton(PanelPopupButton *button);
    PanelPopupButton* findKButtonFor(QMenu* menu);

public Q_SLOTS:
    void slotSetKMenuItemActive();
    void kmenuAccelActivated();

protected:
    PanelKMenu* m_kmenu;

private:
    MenuManager(QObject *parent = 0);

    static MenuManager* m_self;
    KButtonList m_kbuttons;
};

#endif
