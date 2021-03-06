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

#include <Qt3Support/Q3ColorDrag>

#include <ksycocaentry.h>
#include <kservice.h>
#include <kservicegroup.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "addbutton_mnu.h"
#include "addbutton_mnu.moc"
#include "containerarea.h"

PanelAddButtonMenu::PanelAddButtonMenu(ContainerArea* cArea, const QString & label,
				       const QString & relPath, QWidget * parent, const QString& _inlineHeader)
    : PanelServiceMenu(label, relPath, parent, true, _inlineHeader), containerArea(cArea)
{
}

PanelAddButtonMenu::PanelAddButtonMenu(ContainerArea* cArea, QWidget * parent, const QString& _inlineHeader)
    : PanelServiceMenu(QString(), QString(), parent, true, _inlineHeader), containerArea(cArea)
{
}

void PanelAddButtonMenu::slotExec(int id)
{
    if (!entryMap_.contains(id))
	return;

    const KSycocaEntry::Ptr e = entryMap_[id];

    if (e->isType(KST_KServiceGroup)) {
	const KServiceGroup::Ptr g = KServiceGroup::Ptr::staticCast(e);
	containerArea->addServiceMenuButton(g->relPath());
    } else if (e->isType(KST_KService)) {
	const KService::Ptr service = KService::Ptr::staticCast(e);
	containerArea->addServiceButton( service->entryPath() );
    }
}

PanelServiceMenu * PanelAddButtonMenu::newSubMenu(const QString & label, const QString & relPath,
                                                  QWidget * parent, const QString& _inlineHeader)
{
    return new PanelAddButtonMenu(containerArea, label, relPath, parent, _inlineHeader);
}

void PanelAddButtonMenu::addNonKDEApp()
{
    containerArea->addNonKDEAppButton();
}

