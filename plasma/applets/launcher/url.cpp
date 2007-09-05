/***************************************************************************
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "url.h"

#include <QGraphicsSceneDragDropEvent>

#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KMimeType>
#include <KPropertiesDialog>
#include <KRun>
#include <KSharedConfig>
#include <KUrl>

#include <plasma/widgets/icon.h>

Url::Url(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0)
{
    setAcceptDrops(true);

    m_icon = new Plasma::Icon(this);
    connect(m_icon, SIGNAL(clicked()), this, SLOT(openUrl()));

    KConfigGroup cg = config();

    int size = globalConfig().readEntry("IconSize", IconSize(K3Icon::Desktop));
    size = cg.readEntry("IconSize", size);

    m_icon->setIconSize(size, size);
    if (args.count() > 2) {
        setUrl(args.at(2).toString());
    } else {
        setUrl(cg.readEntry("Url"));
    }
}

Url::~Url()
{
    KConfigGroup cg = config();
    cg.writeEntry("IconSize", m_icon->iconSize().toSize());
    cg.writeEntry("Url", m_url);
}

QSizeF Url::contentSize() const
{
//    kDebug() << "contentSize is " << m_icon->boundingRect().size();
    return m_icon->boundingRect().size();
}

void Url::constraintsUpdated()
{
}

void Url::setUrl(const KUrl& url)
{
    m_url = url;
    KMimeType::Ptr mime = KMimeType::findByUrl(url);
    m_mimetype = mime->name();
    m_icon->setIcon(KMimeType::iconNameForUrl(url));
}

void Url::openUrl()
{
    if (m_url.isValid()) {
        KRun::runUrl(m_url, m_mimetype, 0);
    }
}

void Url::propertiesDialog()
{
    if (m_dialog == 0) {
        m_dialog = new KPropertiesDialog(m_url);
        connect(m_dialog, SIGNAL(applied()), this, SLOT(acceptedPropertiesDialog()));
    }

    m_dialog->show();
}

void Url::acceptedPropertiesDialog()
{
    KConfigGroup cg = config();
    m_url = m_dialog->kurl();
    cg.writeEntry("Url", m_url);
    update();
    delete m_dialog;
    m_dialog = 0;
}

void Url::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (KUrl::List::canDecode(event->mimeData())) {
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());

        if (urls.count() > 0) {
            event->accept();
            setUrl(urls.first());
        }
    }
}

#include "url.moc"
