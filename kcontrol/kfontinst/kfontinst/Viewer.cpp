/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Viewer.h"
#include "KfiConstants.h"
#include <klibloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kstandardaction.h>
#include <kactioncollection.h>

#define CFG_GROUP    "FontViewer Settings"
#define CFG_SIZE_KEY "Window Size"

namespace KFI
{

CViewer::CViewer(const KUrl &url)
{
    KLibFactory *factory=KLibLoader::self()->factory("libkfontviewpart");

    if(factory)
    {
        actionCollection()->addAction(KStandardAction::Open, this, SLOT(fileOpen()));
        actionCollection()->addAction(KStandardAction::Quit, kapp, SLOT(quit()));
        //actionCollection()->addAction(KStandardAction::Print, itsPreview,
        //                              SLOT(print()))->setEnabled(false);

        itsPreview=(KParts::ReadOnlyPart *)factory->create(this, "KParts::ReadOnlyPart");

        setCentralWidget(itsPreview->widget());
        createGUI(itsPreview);

        if(url.isValid())
            itsPreview->openUrl(url);

        QSize   defSize(440, 530);
        QString oldGroup(KGlobal::instance()->config()->group());

        KGlobal::instance()->config()->setGroup(CFG_GROUP);
        resize(KGlobal::instance()->config()->readEntry(CFG_SIZE_KEY, defSize));
        KGlobal::instance()->config()->setGroup(oldGroup);
    }
    else
        exit(0);
}

CViewer::~CViewer()
{
    QString oldGroup(KGlobal::instance()->config()->group());

    KGlobal::instance()->config()->setGroup(CFG_GROUP);
    KGlobal::instance()->config()->writeEntry(CFG_SIZE_KEY, size());
    KGlobal::instance()->config()->setGroup(oldGroup);
    KGlobal::instance()->config()->sync();
}

void CViewer::fileOpen()
{
    KUrl url(KFileDialog::getOpenUrl(KUrl(), "application/x-font-ttf application/x-font-otf "
                                             "application/x-font-ttc application/x-font-type1 "
                                             "application/x-font-bdf application/x-font-pcf ",
                                     this, i18n("Select Font to View")));
    if(url.isValid())
        itsPreview->openUrl(url);
}

}

#include "Viewer.moc"